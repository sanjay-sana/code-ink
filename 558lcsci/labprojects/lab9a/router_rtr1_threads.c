#include "custom_globals.h"

#define MY4_SRC_MAC0 0x00
#define MY4_SRC_MAC1 0x04
#define MY4_SRC_MAC2 0x23
#define MY4_SRC_MAC3 0xA6
#define MY4_SRC_MAC4 0xC1
#define MY4_SRC_MAC5 0xE9

#define MY0_SRC_MAC0 0x00
#define MY0_SRC_MAC1 0x04
#define MY0_SRC_MAC2 0x23
#define MY0_SRC_MAC3 0xA6
#define MY0_SRC_MAC4 0xCE
#define MY0_SRC_MAC5 0xE3 

pthread_t intf4, intf0;

// void send_packet_to_dest(unsigned char *buffer, struct sockaddr_ll device, int sock_send, char* ether_port)
// {
// 	if ((device.sll_ifindex = if_nametoindex (ether_port)) == 0)
// 	{
// 		perror ("if_nametoindex() failed to obtain interface index ");
// 		exit (EXIT_FAILURE);
// 	}
// 	printf("Custom packet received in intf4_flow\n");
// 	int send_result = 0;
// 	packet_format *pckt1=(packet_format*)(buffer);
// 	printf("pckt value of packet_type is %d\n",pckt1->hdr.packet_type);
// 	printf("pckt value of src address is %d\n",pckt1->hdr.src_addr);
// 	printf("pckt value of dst address is %d\n",pckt1->hdr.dest_addr);
// 	printf("pckt value of sequence number is %d\n",pckt1->hdr.seq_no);
// 	printf("The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt1->data_inside);
// 	printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
// 	usleep(200);
// 	send_result = sendto(sock_send, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
// 	if (send_result == -1) 
// 	{ 
// 		printf("Sending of packet failed");
// 	}
// }

// void* intf4_flow(void *arg)
// {
// 	printf("Packet sniff for interface 0 of router1 is starting from here-->\n");
// 	int saddr_size , data_size;
// 	struct sockaddr saddr;
	
// 	//Process packet loop function here
// 	printf("Starting...\n");

// 	int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
// 	if (sock_raw == -1)
// 	{ 
// 		printf("socket not created properly\n");
// 		// return(0);
// 	}
	
// 	memset (&device, 0, sizeof (device));
// 	// if ((device.sll_ifindex = if_nametoindex ("eth4")) == 0)
// 	// {
// 	// 	perror ("if_nametoindex() failed to obtain interface index ");
// 	// 	exit (EXIT_FAILURE);
// 	// }
// 	device.sll_family =   AF_PACKET;
// 	device.sll_addr[0]  = MY4_SRC_MAC0;
// 	device.sll_addr[1]  = MY4_SRC_MAC1;
// 	device.sll_addr[2]  = MY4_SRC_MAC2;
// 	device.sll_addr[3]  = MY4_SRC_MAC3;
// 	device.sll_addr[4]  = MY4_SRC_MAC4;
// 	device.sll_addr[5]  = MY4_SRC_MAC5;
// 	device.sll_halen = ETH_ALEN;
// 	device.sll_protocol=htons(0x1234);

// 	unsigned char *buffer = (unsigned char *) malloc(FRAME_LEN);
// 	int sock_send = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
// 	if (sock_send == -1) 
// 	{
// 		printf("The socket was not created successfully\n");
// 	}
// 	int i=0;
// 	while(i < 12 )
// 	// while(1)
// 	{
// 		saddr_size = sizeof saddr;
// 		//Receive a packet
// 		data_size = recvfrom(sock_raw , buffer , FRAME_LEN, 0 , &saddr , (socklen_t*)&saddr_size);
// 		if(data_size < 0 )
// 		{
// 			printf("Recvfrom error , failed to get packets\n");
// 			// return (0);
// 		}
// 		// printf("Packet received data_size = %d\n", data_size);
// 		if (data_size > 0)
// 		{
// 			// void *buffer=(void*)Buffer;
// 			packet_format *pckt=(packet_format*)(buffer);
// 			if(pckt->hdr.packet_type==9999)
// 			{
// 				if (pckt->hdr.dest_addr == 2)
// 				{
// 					send_packet_to_dest(buffer, device, sock_send, "eth4");
// 				}
// 				else 
// 					if (pckt->hdr.dest_addr == 0)
// 					{
// 						send_packet_to_dest(buffer, device, sock_send, "eth0");
// 					}
// 					else
// 					{
// 						printf("Invalid destination address\n");
// 					}
// 			}
// 		}

// 		packet_format *pckt=(packet_format*)(buffer);
// 		if(pckt->hdr.packet_type==9999)
// 		{
// 			i++;
// 		}
// 	}
// 	close(sock_send);
// 	close(sock_raw);
// }

void* intf4_flow(void *arg)
{
	printf("Packet sniff for interface 0 of router1 is starting from here-->\n");
	int saddr_size , data_size;
	struct sockaddr saddr;
	struct sockaddr_ll device={0};
	//Process packet loop function here
	printf("Starting...\n");

	int sock_raw4 = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	if (sock_raw4 == -1)
	{ 
		printf("socket not created properly\n");
		// return(0);
	}
	setsockopt(sock_raw4 , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+ 1 );
	
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex ("eth4")) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit (EXIT_FAILURE);
	}
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY4_SRC_MAC0;
	device.sll_addr[1]  = MY4_SRC_MAC1;
	device.sll_addr[2]  = MY4_SRC_MAC2;
	device.sll_addr[3]  = MY4_SRC_MAC3;
	device.sll_addr[4]  = MY4_SRC_MAC4;
	device.sll_addr[5]  = MY4_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1235);

	unsigned char *buffer = (unsigned char *) malloc(FRAME_LEN);
	int sock_send4 = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock_send4 == -1) 
	{
		printf("The socket was not created successfully\n");
	}
	setsockopt(sock_send4 , SOL_SOCKET , SO_BINDTODEVICE , "eth4" , strlen("eth4")+ 1 );
	int i=0;
	while(i < 12 )
	// while(1)
	{
		saddr_size = sizeof saddr;
		//Receive a packet
		data_size = recvfrom(sock_raw4 , buffer , FRAME_LEN, 0 , &saddr , (socklen_t*)&saddr_size);
		if(data_size < 0 )
		{
			printf("Recvfrom error , failed to get packets\n");
			// return (0);
		}
		// printf("Packet received data_size = %d\n", data_size);
		if (data_size > 0)
		{
			// void *buffer=(void*)Buffer;
			packet_format *pckt=(packet_format*)(buffer);
			if(pckt->hdr.packet_type == 9999)
			{
				printf("Custom packet received in intf0_flow\n");
				if (pckt->hdr.dest_addr == 2)
				{
					int send_result = 0;
					packet_format *pckt1=(packet_format*)(buffer);
					printf("pckt value of packet_type is %d\n",pckt1->hdr.packet_type);
					printf("pckt value of src address is %d\n",pckt1->hdr.src_addr);
					printf("pckt value of dst address is %d\n",pckt1->hdr.dest_addr);
					printf("pckt value of sequence number is %d\n",pckt1->hdr.seq_no);
					printf("The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt1->data_inside);
					printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
					usleep(200);
					send_result = sendto(sock_send4, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
					if (send_result == -1) 
					{ 
						printf("Sending of packet failed");
					}
				}
			}
		}
		packet_format *pckt=(packet_format*)(buffer);
		if(pckt->hdr.packet_type==9999)
		{
			i++;
		}
	}
	close(sock_send4);
	close(sock_raw4);
}

void* intf0_flow(void *arg)
{
	printf("Packet sniff for interface 4 of router1 is starting from here-->\n");
	int saddr_size , data_size;
	struct sockaddr saddr;
	struct sockaddr_ll device={0};
	//Process packet loop function here
	printf("Starting...\n");

	int sock_raw0 = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	if (sock_raw0 == -1)
	{ 
		printf("socket not created properly\n");
		// return(0);
	}
	setsockopt(sock_raw0 , SOL_SOCKET , SO_BINDTODEVICE , "eth4" , strlen("eth4")+ 1 );
	
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex ("eth0")) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit (EXIT_FAILURE);
	}
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY0_SRC_MAC0;
	device.sll_addr[1]  = MY0_SRC_MAC1;
	device.sll_addr[2]  = MY0_SRC_MAC2;
	device.sll_addr[3]  = MY0_SRC_MAC3;
	device.sll_addr[4]  = MY0_SRC_MAC4;
	device.sll_addr[5]  = MY0_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	unsigned char *buffer = (unsigned char *) malloc(FRAME_LEN);
	int sock_send0 = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock_send0 == -1) 
	{
		printf("The socket was not created successfully\n");
	}
	setsockopt(sock_send0 , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+ 1 );
	int i=0;
	while(i < 12 )
	// while(1)
	{
		saddr_size = sizeof saddr;
		//Receive a packet
		data_size = recvfrom(sock_raw0 , buffer , FRAME_LEN, 0 , &saddr , (socklen_t*)&saddr_size);
		if(data_size < 0 )
		{
			printf("Recvfrom error , failed to get packets\n");
			// return (0);
		}
		// printf("Packet received data_size = %d\n", data_size);
		if (data_size > 0)
		{
			// void *buffer=(void*)Buffer;
			packet_format *pckt=(packet_format*)(buffer);
			if(pckt->hdr.packet_type==9999)
			{
				printf("Custom packet received in intf0_flow\n");
				if (pckt->hdr.dest_addr == 0)
				{
					int send_result = 0;
					packet_format *pckt1=(packet_format*)(buffer);
					printf("pckt value of packet_type is %d\n",pckt1->hdr.packet_type);
					printf("pckt value of src address is %d\n",pckt1->hdr.src_addr);
					printf("pckt value of dst address is %d\n",pckt1->hdr.dest_addr);
					printf("pckt value of sequence number is %d\n",pckt1->hdr.seq_no);
					printf("The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt1->data_inside);
					printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
					usleep(200);
					send_result = sendto(sock_send0, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
					if (send_result == -1) 
					{ 
						printf("Sending of packet failed");
					}
				}
			}
		}
		packet_format *pckt=(packet_format*)(buffer);
		if(pckt->hdr.packet_type==9999)
		{
			i++;
		}
	}
	close(sock_send0);
	close(sock_raw0);
}

int main(int argc, char *argv[])
{
	 
	pthread_create(&intf4, NULL, intf4_flow, NULL);
	pthread_create(&intf0, NULL, intf0_flow, NULL);
	
	pthread_join(intf4,NULL);
	pthread_join(intf0,NULL);

	return 0;
}
