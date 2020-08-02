from ryu.base import app_manager
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import packet
from ryu.controller import ofp_event, dpset
from ryu.lib.ip import ipv4_to_bin
from ryu.base.app_manager import RyuApp
from ryu.controller.ofp_event import EventOFPSwitchFeatures
from ryu.controller.ofp_event import EventOFPPacketIn
from ryu.ofproto.ofproto_v1_2 import OFPG_ANY
from ryu.ofproto.ofproto_v1_3 import OFP_VERSION
from ryu.ofproto import ether
from ryu.lib.packet import vlan
import thread
import socket
import time

class SimpleSwitch13(app_manager.RyuApp):
	OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

	def __init__(self, *args, **kwargs):
		super(SimpleSwitch13, self).__init__(*args, **kwargs)
		# self.mac_to_port = {}
		var=""
		thread.start_new_thread(self.SendToHosts ,(var))
		thread.start_new_thread(self.ReceiveFromHosts ,(var))
	# @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
	def SendToHosts(var):
		# print "Thread invoked for send."
		try:
			sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		except sock.error:
			print "Socket creation error"
		# print "Socket created successfully."

		message = "Get elephant flows in the network"
		while(1):
			time.sleep(1)
			for x in xrange(1,17):
				str1 = "20.0.0."
				str1 += str(x)
				sock.sendto(message, (str1, 5000))
				print "Message sent - ", message, "to", str1
				time.sleep(0.05)

	def ReceiveFromHosts(var):
		# print "Thread invoked for receive."
		try:
			sock_recv = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		except sock_recv.error:
			print "Socket creation error"
		# print "Socket created successfully."
		while (1):
			try:
				# print "Trying to bind."
				sock_recv.bind(("20.0.0.100", 5010))
				break;
			except:
				pass
		while(1):
			reply = sock_recv.recv(100)
			print "Message received - ", reply, "on 20.0.0.100"

	def prepareSwitch(self, sw):
		ofproto = sw.ofproto
		ofp_parser = sw.ofproto_parser
		ofp = ofproto

		def edgeSwitchUpward(output_port1, output_port2, hostIp):
			action1 = sw.ofproto_parser.OFPActionOutput(output_port1)
			action2 = sw.ofproto_parser.OFPActionOutput(output_port2)
			bucket1 = sw.ofproto_parser.OFPBucket(weight=1, actions=[action1])
			bucket2 = sw.ofproto_parser.OFPBucket(weight=1, actions=[action2])
			group_mod = sw.ofproto_parser.OFPGroupMod(
					datapath=sw, command=ofproto.OFPGC_ADD, 
					type_=ofproto.OFPGT_SELECT, group_id=1,
					buckets=[bucket1, bucket2])
			sw.send_msg(group_mod)

			match = sw.ofproto_parser.OFPMatch(eth_type=ether.ETH_TYPE_IP, ipv4_dst=((10 << 24) + hostIp))
			group_action = sw.ofproto_parser.OFPActionGroup(1)
			inst = [ofp_parser.OFPInstructionActions(ofp.OFPIT_APPLY_ACTIONS, [group_action])]
			mod = sw.ofproto_parser.OFPFlowMod(
					datapath=sw, match=match, cookie=0,
					command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
					priority=1100,
					flags=ofproto.OFPFF_SEND_FLOW_REM, instructions=inst)
			sw.send_msg(mod)


		def edgeSwitchBottom(outport_port, hostIp):
			match = sw.ofproto_parser.OFPMatch(eth_type=ether.ETH_TYPE_IP, ipv4_dst=((10 << 24) + hostIp))
			action = sw.ofproto_parser.OFPActionOutput(outport_port)
			inst = [ofp_parser.OFPInstructionActions(ofp.OFPIT_APPLY_ACTIONS, [action])]
			mod = sw.ofproto_parser.OFPFlowMod(
					datapath=sw, match=match, cookie=0,
					command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
					priority=1100,
					flags=ofproto.OFPFF_SEND_FLOW_REM, instructions=inst)
			sw.send_msg(mod)

		if (1 <= sw.id <= 8):
			for hostIpY in range(1, 17):
				if (hostIpY == (sw.id*2)):
					edgeSwitchBottom(2, hostIpY)
				elif (hostIpY == ((sw.id*2)-1)):
					edgeSwitchBottom(1, hostIpY)
				else:
					edgeSwitchUpward(3, 4, hostIpY)

		if sw.id in [9, 11, 13, 15]:
			tempSwitchID = sw.id - 8
			for hostIpY in range(1, 17):
				if hostIpY in [((tempSwitchID * 2) - 1), (tempSwitchID * 2)]:
					edgeSwitchBottom(4, hostIpY)
				elif hostIpY in [((tempSwitchID * 2) + 1), ((tempSwitchID * 2) + 2)]:
					edgeSwitchBottom(3, hostIpY)
				else:
					edgeSwitchUpward(1, 2, hostIpY)

		if sw.id in [10, 12, 14, 16]:
			tempSwitchID = sw.id - 8
			for hostIpY in range(1, 17):
				if hostIpY in [((tempSwitchID * 2) - 1), (tempSwitchID * 2)]:
					edgeSwitchBottom(4, hostIpY)
				elif hostIpY in [((tempSwitchID * 2) - 3), ((tempSwitchID * 2) - 2)]:
					edgeSwitchBottom(3, hostIpY)
				else:
					edgeSwitchUpward(1, 2, hostIpY)

		if (17 <= sw.id <= 20):
			tempSwitchID = sw.id - 16
			for hostIpY in range(1, 17):
				if (1 <= hostIpY <= 4):
					edgeSwitchBottom(1, hostIpY)
				elif (5 <= hostIpY <= 8):
					edgeSwitchBottom(2, hostIpY)
				elif (9 <= hostIpY <= 12):
					edgeSwitchBottom(3, hostIpY)
				elif (13 <= hostIpY <= 16):
					edgeSwitchBottom(4, hostIpY)

	@set_ev_cls(dpset.EventDP)
	def switchStatus(self, ev):
		print("Switch %s: %s!" %
				(ev.dp.id, "connected" if ev.enter else "disconnected"))
		self.prepareSwitch(ev.dp)
