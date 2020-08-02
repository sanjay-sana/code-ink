from mininet.topo import Topo

class PolygonTopo(Topo):
    def __init__(self, count):
        Topo.__init__(self)
        vertexCount = count

        self.hosts_ = [
                self.addHost('h%d' % hostId)
                for hostId in range(vertexCount)]

        self.switches_ = [
                self.addSwitch('s%d' % switchId, dpid=("%0.2X" % (switchId+1)))
                for switchId in range(vertexCount)]

        self.hostLinks_ = [
                self.addLink('h%d' % eId, 's%d' % eId)
                for eId in range(vertexCount)]

        self.switchLinks_ = [
                self.addLink('s%d' % eId, 's%d' % ((eId+1) % vertexCount), 2, 3)
                for eId in range(vertexCount)]

    @classmethod
    def create(cls, count):
        return cls(count)

topos = {'poly': PolygonTopo.create}
