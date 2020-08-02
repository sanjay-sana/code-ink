#!/usr/bin/env python

import collections
from collections import defaultdict
from dateutil.parser import parser
import os
import re
import shutil
import time
import sys

NUM_HOSTS=16
DEFAULT_WAIT_TIME=3
TRACE='./traffic'

MN_PATH='~/mininet'
MN_UTIL=os.path.join(MN_PATH, 'util', 'm')

CmdTcpDump = {
    'start': 'sudo tcpdump -i {host}-eth0 -n -s 64 -B 8192 -w dump/{host}.pcap >/dev/null 2>&1 &',
    'kill' : 'sudo killall tcpdump 2>/dev/null'
}

CmdSender = {
    'start': './sender {traffic}/{host}.trace > logs/sender-{host}.log 2>&1 &',
    'kill': 'sudo killall sender 2>/dev/null'
}

CmdReceiver = {
    'start': './receiver {traffic}/port.txt > logs/receiver-{host}.log 2>&1 &',
    'kill': 'sudo killall receiver 2>/dev/null'
}

def MnExec(hostName, command):
    cmd = '%s %s %s' % (MN_UTIL, hostName, command)
    os.system(cmd)


class Host(object):
    def __init__(self, id_):
        self.id_ = id_

    @property
    def name(self):
        return 'h' + str(self.id_)

    def ip(self):
        return '10.0.0.%s' % str(self.id_)

    def startTcpDump(self, trace):
        name = self.name
        MnExec(name, CmdTcpDump['start'].format(host=name))

    def startSender(self, trace):
        name = self.name
        MnExec(name, CmdSender['start'].format(host=name, traffic=trace))

    def startReceiver(self, trace):
        name = self.name
        MnExec(name, CmdReceiver['start'].format(host=name, traffic=trace))

    def kill(self):
        name = self.name
        MnExec(name, CmdTcpDump['kill'].format(host=name))
        MnExec(name, CmdReceiver['kill'].format(host=name))
        MnExec(name, CmdSender['kill'].format(host=name))

def WaitMinutes(t):
    time.sleep(t * 60)

def ForAll(func, arr):
    for el in arr: func(el)

def VolumeToBytes(vol):
    volNum = int(re.findall("\d+", vol)[0])
    if 'K' in vol:
        volNum *= 1000
    elif 'G' in vol:
        volNum *= 1e9
    elif 'M' in vol:
        volNum *= 1e6
    return volNum

def ParseSenderFile(src, fname):
    traffic = defaultdict(lambda: defaultdict(dict))
    with open(fname, 'r') as f:
        regexp = re.compile('-d 10.0.0.(\S*) -p (\S*) -n (\S*)')
        for line in f.readlines():
            m = regexp.match(line)
            if not m: continue

            dstHost = m.group(1)
            dstPort = m.group(2)
            dstVol  = VolumeToBytes(m.group(3))

            dst = Host(int(dstHost))
            traffic[dst.name][src.name][dstPort] = dstVol

    return traffic

def NestedMerge(d, u):
    for k, v in u.iteritems():
        if isinstance(v, collections.Mapping):
            r = NestedMerge(d.get(k, {}), v)
            d[k] = r
        else:
            d[k] = u[k]
    return d

def ParseTraceFiles(hosts, trace):
    receiverResults = {}
    senderSpecs = map(lambda h: 
            (h, os.path.join(trace, h.name + '.trace')), hosts)

    # Parse sender file
    traffic = {}
    for sf in senderSpecs:
        tr = ParseSenderFile(sf[0], sf[1])
        traffic = NestedMerge(traffic, tr)
    return traffic

def ConnectionBytes(src, dst, port, pcap):
    cmd = 'tcpdump tcp and dst {dst} and src {src} and dst port {dstport} -r dump/{pcap} -w dump/tmp.pcap 2>/dev/null'
    cmd = cmd.format(src=src, dst=dst, dstport=port, pcap=pcap)
    os.system(cmd)

    byteCount = 0
    firstPacketTime = 0
    lastPacketTime = 0

    try:
        cmd = 'tcptrace -bln dump/tmp.pcap | grep "unique bytes sent" | awk \'{print $4}\''
        byteCount = os.popen(cmd).read()
        dateParser = parser()

        cmd = 'tcptrace -bln dump/tmp.pcap | grep "first packet" | cut -d: -f2-'
        firstPacketTime = dateParser.parse(os.popen(cmd).read())

        cmd = 'tcptrace -bln dump/tmp.pcap | grep "last packet" | cut -d: -f2-'
        lastPacketTime = dateParser.parse(os.popen(cmd).read())
    except Exception as e:
        return (0, None, None)
    
    if byteCount == '': byteCount = 0
    return (int(byteCount), firstPacketTime, lastPacketTime)

def AnalyzeDump(hosts, trace):
    trafficStats = ParseTraceFiles(hosts, trace)
    incorrectFlows = 0
    correctFlows = 0
    totalByteCount = 0

    def hostToIp(name):
        return '10.0.0.%d' % (int(name[1:]))

    def hostPcap(name):
        return '%s.pcap' % name


    tMin = None
    tMax = None

    for dst in trafficStats:
        dstInfo = trafficStats[dst]
        for src in dstInfo:
            srcInfo = dstInfo[src]
            for dstPort in srcInfo:
                expectedByteCount = int(srcInfo[dstPort])
                byteCount, t1, t2 = ConnectionBytes(hostToIp(src), hostToIp(dst), dstPort, hostPcap(dst))
                totalByteCount += byteCount

                if (not tMin) or (t1 and (tMin > t1)):
                    tMin = t1

                if (not tMax) or (t2 and (tMax < t2)):
                    tMax = t2

                if expectedByteCount != byteCount:
                    incorrectFlows += 1
                    print >> sys.stderr, \
                            ('Error: flow %s -> %s:%s -- expected: %d, got: %d' % \
                            (src, dst, dstPort, expectedByteCount, byteCount))
                else:
                    correctFlows += 1

    return {
        'correct': correctFlows,
        'incorrect': incorrectFlows, 
        'start': tMin,
        'end': tMax,
        'bytes': totalByteCount
        }

def RunExperiment(hosts, trace, wait=DEFAULT_WAIT_TIME):
    def MakeDumpDirectories():
        if os.path.exists('dump'): shutil.rmtree('dump')
        if os.path.exists('logs'): shutil.rmtree('logs')

        os.makedirs('dump')
        os.makedirs('logs')

    MakeDumpDirectories()

    print "> Starting the hosts."
    ForAll(lambda h: h.startTcpDump(trace), hosts)
    time.sleep(2)
    ForAll(lambda h: h.startReceiver(trace), hosts)
    ForAll(lambda h: h.startSender(trace), hosts)

    print "Waiting for the experiment to finish."
    WaitMinutes(wait)

    print "> Killing pending host processes."
    ForAll(lambda h: h.kill(), hosts)

    print "> Analyzing dump folder"
    return AnalyzeDump(hosts, trace)


if __name__ == '__main__':
    hosts = [Host(x) for x in range(1,NUM_HOSTS+1)]
    output = RunExperiment(hosts, TRACE)
    seconds = (output['end'] - output['start']).total_seconds()
    byteCount = output['bytes']
    incorrect = output['incorrect']
    correct = output['correct']
    totalFlows = correct + incorrect

    print 'Throughput: %.2f Mbps\n%% of correct flows: %.2f' % \
            ((byteCount*8.0/(seconds*1024.0*1024)), (correct*100.0/totalFlows))
