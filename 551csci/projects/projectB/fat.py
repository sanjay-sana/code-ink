from mininet.topo import Topo

class FatTopo(Topo):
	def __init__(self):
		Topo.__init__(self)
		# vertexCount = count
		count = 16
		switch_count = 0

# Defining 16 hosts
		self.hosts_ = [
				self.addHost('h%d' % (hostId + 1))
				for hostId in range(0, count)]
# Defining 8 edge switches
		self.edge_switches_= []
		for switchId in range(0, 8):
			switch_count += 1
			self.edge_switches_.append(self.addSwitch('edge_s%d' % switchId, dpid=("%0.2X" % switch_count), protocols='OpenFlow13'))

# Defining 8 aggregate switches
		self.aggre_switches_ = []
		for switchId in range(0, 8):
			switch_count += 1
			self.aggre_switches_.append(self.addSwitch('aggre_s%d' % switchId, dpid=("%0.2X" % switch_count), protocols='OpenFlow13'))

# Defining 4 core switches
		self.core_switches_ = []
		for switchId in range(0, 4):
			switch_count += 1
			self.core_switches_.append(self.addSwitch('core_s%d' % switchId, dpid=("%0.2X" % switch_count), protocols='OpenFlow13'))

# Defining links between hosts and edge switches
		for eId in range(0, len(self.edge_switches_)):
			self.addLink(self.hosts_[(eId * 2)], self.edge_switches_[eId], 0, 1)
			self.addLink(self.hosts_[((eId * 2) + 1)], self.edge_switches_[eId], 0, 2)

# Defining links between edge switches and aggregate switches
		for eId in range(0, len(self.edge_switches_), 2):
			self.addLink(self.edge_switches_[eId], self.aggre_switches_[eId], 3, 4)
			self.addLink(self.edge_switches_[eId], self.aggre_switches_[eId + 1], 4, 3)
			self.addLink(self.edge_switches_[eId + 1], self.aggre_switches_[eId], 4, 3)
			self.addLink(self.edge_switches_[eId + 1], self.aggre_switches_[eId + 1], 3, 4)

# Defining links between aggregate switches and core switches
		for cId in range(0, len(self.core_switches_)):
			eth_counter = 1
			if cId < 2:
				for aId in range(0, len(self.aggre_switches_), 2):
					self.addLink(self.core_switches_[cId], self.aggre_switches_[aId], eth_counter, ((cId % 2) + 1))
					eth_counter+=1
			else:
				for aId in range(1, len(self.aggre_switches_), 2):
					self.addLink(self.core_switches_[cId], self.aggre_switches_[aId], eth_counter, ((cId % 2) + 1))
					eth_counter+=1

topos = {'fattopo': (lambda:FatTopo())}
