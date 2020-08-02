from mininet.cli import CLI
from mininet.net import Mininet
from mininet.node import *
from mininet.nodelib import LinuxBridge
from mininet.topo import Topo

import mininet

from optparse import OptionParser
import os, sys
import subprocess

CONTROLLER_IP="20.0.0.100/32"

# Connects all the host in the network to the root
# Namespace.  This enables the nodes to communicate with
# Arbiter, by using the 20.0.0.100 IP address.
def connect_hosts_to_root_ns(net):
    sw   = LinuxBridge('rtsw', dpid="001020203")
    root = Node('root', inNamespace=False)
    root0 = net.addLink(root, sw)

    for host in net.hosts:
        ip = ""

        # Get the IP of the other interface
        try:
            ip = host.intfList()[0].ip
            ip = "20." + ".".join(ip.split(".")[1:])
        except Exception:
            print ("Failed to get the IP address of your interface."
                + "  The offending host is: " + str(host))
            return False

        link = net.addLink(sw, host)
        host.cmd('ifconfig ' + str(link.intf2) + ' ' + str(ip) + '/24')
        
    root.setIP(CONTROLLER_IP, intf=root0.intf1)
    root.cmd('route add -net 20.0.0.0/24 dev ' + str(root0.intf1));
    sw.start([])
    return True


def parse_options():
    parser = OptionParser()
    parser.add_option("-c", "--custom", dest="custom",
            help="The custom topology file.")
    parser.add_option("-t", "--topo", dest="topo", 
            help="Name of the topology in the topos dictionary")
    return parser.parse_args()

def module_name_from_file(filename):
    return os.path.splitext(filename)[0]

def is_bridge_utils_installed():
    try: 
        subprocess.call(['which', 'brctl'])
        return True
    except: 
        return False

class TenMbpsIntf(mininet.link.TCIntf):
    def __init__(self, *args, **kwargs):
        super(TenMbpsIntf, self).__init__(*args, **kwargs)

    def bwCmds(self, bw=None, **params):
        is_root = 'root' in self.name
        is_aux  = 'rtsw' in self.name
        is_sec  = 'eth1' in self.name

        if bw is None or is_root or is_aux or is_sec:
          return [], ' root '

        cmds   = [ '%s qdisc add dev %s root handle 5:0 htb default 1',
            '%s class add dev %s parent 5:0 classid 5:1 htb ' +
            'rate 10Mbit burst 15k ceil 12Mbit']
        parent = ' parent 5:1 '
        return cmds, parent

class TenMbpsLink(mininet.link.Link):
    def __init__( self, node1, node2, port1=None, port2=None,
          intfName1=None, intfName2=None,
          addr1=None, addr2=None, **params ):
        params = dict(bw=10, max_queue_size=200)
        super(TenMbpsLink, self).__init__(
            node1, node2, port1=port1, port2=port2,
            intfName1=intfName1, intfName2=intfName2,
            cls1=TenMbpsIntf,
            cls2=TenMbpsIntf,
            addr1=addr1, addr2=addr2,
            params1=params,
            params2=params )

if __name__ == '__main__':
    if not is_bridge_utils_installed():
        print ("Please install bridge-utils: sudo aptitude install bridge-utils")
        exit(-1)

    (options, args) = parse_options()

    print ("Importing the topology file.")
    topo = __import__(module_name_from_file(options.custom))

    topo_args = options.topo.split(",")
    topo_name = topo_args[0]
    topo_params = []
    if len(topo_args) > 1:
        topo_params = map(lambda x: int(x), topo_args[1:])

    print ("Initiating Mininet.")
    net = Mininet(topo.topos[topo_name](*topo_params), controller=RemoteController,
        autoStaticArp=True, link=TenMbpsLink)

    print ("Initiating the host/controller network.")
    if (not connect_hosts_to_root_ns(net)):
        print("Failed to create the host/controller network.")
        sys.exit()


    print ("Hosts can access the Controller at: " + str(CONTROLLER_IP))
    net.start()

    print ("Initiating Mininet CLI.")
    CLI(net)

    net.stop()
