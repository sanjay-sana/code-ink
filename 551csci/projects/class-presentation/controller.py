from ryu.base import app_manager
from ryu.controller import ofp_event, dpset
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.lib.ip import ipv4_to_bin

class Controller(app_manager.RyuApp):
    def prepareSwitch(self, sw):
        hostIp = int(sw.id) - 1
        ofproto = sw.ofproto

        # Set the outport, etc.
        match = sw.ofproto_parser.OFPMatch(in_port=1)
        action = sw.ofproto_parser.OFPActionOutput(2)
        mod = sw.ofproto_parser.OFPFlowMod(
                datapath=sw, match=match, cookie=0,
                command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
                priority=1100,
                flags=ofproto.OFPFF_SEND_FLOW_REM, actions=[action])
        sw.send_msg(mod)

        # Send the ARP/IP packets to the proper host
        match = sw.ofproto_parser.OFPMatch(in_port=3, dl_type=0x806, nw_dst=((10 << 24) + hostIp + 1))
        action = sw.ofproto_parser.OFPActionOutput(1)
        mod = sw.ofproto_parser.OFPFlowMod(
                datapath=sw, match=match, cookie=0,
                command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
                priority=1200,
                flags=ofproto.OFPFF_SEND_FLOW_REM, actions=[action])
        sw.send_msg(mod)
        match = sw.ofproto_parser.OFPMatch(in_port=3, dl_type=0x800, nw_dst=((10 << 24) + hostIp + 1))
        action = sw.ofproto_parser.OFPActionOutput(1)
        mod = sw.ofproto_parser.OFPFlowMod(
                datapath=sw, match=match, cookie=0,
                command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
                priority=1000,
                flags=ofproto.OFPFF_SEND_FLOW_REM, actions=[action])
        sw.send_msg(mod)

        # Ignore all the other types of packets in the network
        match = sw.ofproto_parser.OFPMatch(in_port=3, dl_type=0x806)
        action = sw.ofproto_parser.OFPActionOutput(2)
        mod = sw.ofproto_parser.OFPFlowMod(
                datapath=sw, match=match, cookie=0,
                command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
                priority=10,
                flags=ofproto.OFPFF_SEND_FLOW_REM, actions=[action])
        sw.send_msg(mod)
        match = sw.ofproto_parser.OFPMatch(in_port=3, dl_type=0x800)
        action = sw.ofproto_parser.OFPActionOutput(2)
        mod = sw.ofproto_parser.OFPFlowMod(
                datapath=sw, match=match, cookie=0,
                command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
                priority=10,
                flags=ofproto.OFPFF_SEND_FLOW_REM, actions=[action])
        sw.send_msg(mod)

    # the rest of the code
    @set_ev_cls(dpset.EventDP)
    def switchStatus(self, ev):
        print("Switch %s: %s!" %
                (ev.dp.id, "connected" if ev.enter else "disconnected"))

        self.prepareSwitch(ev.dp)
