#include "custom_globals.h"

#define MY0_SRC_MAC0 0x00
#define MY0_SRC_MAC1 0x04
#define MY0_SRC_MAC2 0x23
#define MY0_SRC_MAC3 0xA6
#define MY0_SRC_MAC4 0xCE
#define MY0_SRC_MAC5 0xE3 

int sock_sniff0, sock_sniff4;

unsigned char *buffer0, *buffer4;

void sniff_intf0_func()
{
	printf("sniff_intf0_func\n");
	sock_sniff0 = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	if (sock_sniff0 == -1)
	{ 
		printf("socket not created properly\n");
	}
	setsockopt(sock_sniff0 , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+1);

	int saddr_size , data_size;
	struct sockaddr saddr;
	saddr_size = sizeof saddr;
	buffer0 = (unsigned char *)malloc(FRAME_LEN);

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY0_SRC_MAC0;
	device.sll_addr[1]  = MY0_SRC_MAC1;
	device.sll_addr[2]  = MY0_SRC_MAC2;
	device.sll_addr[3]  = MY0_SRC_MAC3;
	device.sll_addr[4]  = MY0_SRC_MAC4;
	device.sll_addr[5]  = MY0_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	int send_result = 0;

	printf("Sniffing on eth0...\n");
	int i=0;
	while(i < 12 )
	// while(1)
	{
		data_size= recvfrom(sock_sniff0, buffer0, FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
		packet_format *pckt=(packet_format*)(buffer0);
		if(pckt->hdr.packet_type == 9999 && pckt->hdr.dest_addr == 2)
		{
			printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);
		    printf("pckt value of src address is %d\n",pckt->hdr.src_addr);
		    printf("pckt value of dst address is %d\n",pckt->hdr.dest_addr);
		    printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);
		    printf("eth0 The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt->data_inside);
		    printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		    
	    	if ((device.sll_ifindex = if_nametoindex ("eth4")) == 0)
			{
				perror ("if_nametoindex() failed to obtain interface index ");
				exit (EXIT_FAILURE);
			}
		    send_result = sendto(sock_sniff0, buffer0, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
			if (send_result == -1) 
			{ 
				printf("Sending of packet failed");
			}

			if ((device.sll_ifindex = if_nametoindex ("eth0")) == 0)
			{
				perror ("if_nametoindex() failed to obtain interface index ");
				exit (EXIT_FAILURE);
			}
			pckt->hdr.src_addr = 2;
			pckt->hdr.dest_addr = 0;
		    send_result = sendto(sock_sniff0, buffer0, ACK_FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
			if (send_result == -1) 
			{ 
				printf("Sending of packet failed");
			}
		}
		packet_format *pckt1=(packet_format*)(buffer0);
		if(pckt1->hdr.packet_type == 9999)
		{
			i++;
		}
	}
	close(sock_sniff0);
}

int main(int argc, char const *argv[])
{
	sniff_intf0_func();
	return 0;
}