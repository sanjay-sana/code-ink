To run the custom topology on mininet, issue:

sudo mn --custom topology.py --topo poly,4 --controller remote --arp

To run the controller in another terminal, issue:

ryu-manager controller.py
