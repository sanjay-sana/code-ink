1. Did you use code from anywhere for your project? If not, say so. If so, say what
functions and where they are from. (Also identify this with a comment in the
source code.)

From host_receiver.c

getListOfEthernetInterfaces() - Partially obtained from StackOverFlow. This was shared in Piazza too. Function obtains the list of Ethernet interfaces and their corresponding IP addresses.

// Partially obtained from StackOverflow
void getListOfEthernetInterfaces()
{
}

// Imported from Binary tides page which supports receiving data from multiple clients
// http://www.binarytides.com/socket-programming-c-linux-tutorial/
void *ReceiveData(void *input)
{
}

From host_sender.c

// Imported code with some modifications from the below link - Allows to receive and reply 
// back with a UDP packet to make a communication with the controller.
// https://www.abc.se/~m6695/udp.html
void *get_flow_state()
{
}

2. Describe how you implement the sender and receiver at hosts

Receiver - 

Following steps are involved - 
(a) Reads the file and extracts the port numbers present in the file.
(b) Copies the port numbers into an internal structure.
(c) Invokes as many number of ports as present in the file.
(d) Each thread creates a socket for each flow and listens for the connections.
(e) Upon accepting, the thread creates invokes another thread to further receive the packets. This allows the receiver to handle multiple connections concurrently. The same occurs for each flow. 

Sender.c

Following steps are involved - 

(a) Reads the file and extracts the flow details present in the file.
(b) Copies the flow details into an internal structure containing corresponding variables.
(c) Invokes one thread per flow.
(d) Each thread establishes a connection with the receiver. Based on the number of bytes to be sent, fragmentation is done such that 1460 bytes of payload is transmitted on each packet.
(e) Simultaneously, another thread getFlowState() runs which communicates with the controller on 20.0.0.100
(f) This function interacts with controller using UDP packets. It is an unreliable way of communication which responds whenever it receives a request packet from the controller.

3. Describe the algorithm you use at the controller.

Following steps are involved - 

(a) PrepareSwitch function handles all the rules needed to route the packets to each of the hosts in the network.
(b) Controller follows IP based routing for downward flow of packets in the fat tree.
(c) Upward flow of the packet is controlled using group tables.
(d) Two output ports are provided to the group table for any given switch which makes sure that each output port is chosen with a probability of half.
(e) Two threads are invoked in the init function on the controller.
(f) Of the 2 threads, the first one sends the request UDP packets continuously to each of the 16 hosts every 1 second. The second thread listens for the incoming response from the hosts.
