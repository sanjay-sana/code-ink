#include "custom_globals.h"

#define MY_DEST_MAC0 0x00
#define MY_DEST_MAC1 0x04
#define MY_DEST_MAC2 0x23
#define MY_DEST_MAC3 0xA6
#define MY_DEST_MAC4 0x56
#define MY_DEST_MAC5 0x70

#define INTERFACE_TO_RTR1 "eth3"

int file_des;
unsigned int file_len;

struct stat buffer;
unsigned char *data_array;

int seq_no_max=0;
int curr_packet_no=0;

pthread_t sender_thread, ack_thread;

void *send_data_func()
{
	printf("Function running\n");
	printf("Buffer\n%s\n", data_array);

	if(file_len % DATA_LENGTH == 0)
	{
		seq_no_max=(file_len/DATA_LENGTH);
	}
	else
	{
		seq_no_max=(file_len/DATA_LENGTH) + 1;
	}
	printf("The value of maximum sequnce number obtained is %d\n",seq_no_max);

	//Making the socket for sending to the router1
	int socket_send;
	socket_send = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	// Prepare packet to send
	unsigned char* buffer = (unsigned char*)malloc(FRAME_LEN);
	memset(buffer, 0, FRAME_LEN);

	// header_format *hdr = (header_format*)malloc(sizeof(header_format));
	packet_format *packet_send = (packet_format*)malloc(sizeof(packet_format));
	memset(packet_send, 0, sizeof(packet_format));

	(packet_send->hdr).packet_type=9999;
	(packet_send->hdr).src_addr=0;
	(packet_send->hdr).dest_addr=2;
	(packet_send->hdr).seq_no=curr_packet_no;

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (INTERFACE_TO_RTR1)) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit (EXIT_FAILURE);
	}
	device.sll_family  =   AF_PACKET;
	device.sll_addr[0] = MY_DEST_MAC0;
	device.sll_addr[1] = MY_DEST_MAC1;		
	device.sll_addr[2] = MY_DEST_MAC2;
	device.sll_addr[3] = MY_DEST_MAC3;
	device.sll_addr[4] = MY_DEST_MAC4;
	device.sll_addr[5] = MY_DEST_MAC5;
	device.sll_halen   = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	while (curr_packet_no < seq_no_max)
	{
		(packet_send->hdr).seq_no = curr_packet_no;
		if (file_len - (curr_packet_no * DATA_LENGTH) > DATA_LENGTH)
		{
			memcpy( packet_send->data_inside , &data_array[curr_packet_no * DATA_LENGTH] , DATA_LENGTH);
		}
		else
		{
			memcpy( packet_send->data_inside , &data_array[curr_packet_no * DATA_LENGTH] , file_len - (curr_packet_no * DATA_LENGTH));
		}

		memcpy((void*)buffer, (void*)packet_send, sizeof(packet_format));
		packet_format *pckt=(packet_format*)buffer;
		printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);
		printf("pckt value of src address is %d\n",pckt->hdr.src_addr);
		printf("pckt value of dst address is %d\n",pckt->hdr.dest_addr);
		printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);
		printf("The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt->data_inside);
		printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");

		//Sending the packet one by one through sendto()
		int send_result;
		send_result = sendto(socket_send, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
		if (send_result == -1) 
		{ 
			printf("Sending of packet %d failed\n",curr_packet_no);
			exit(0);
		}

		curr_packet_no++;
		memset(packet_send->data_inside, 0, DATA_LENGTH);
		memset(buffer, 0, FRAME_LEN);
	}
	close(socket_send);
}

void *recv_data_func()
{

	printf("The sniffing loop starts here ... \n");
	int saddr_size , data_size;
	struct sockaddr saddr;
	int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if (sock_raw == -1) { 
      printf("socket not created properly\n");
    }
    setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth3" , strlen("eth3")+ 1 );
    char *buf = (char *) malloc(FRAME_LEN);
    while(1)
	{
		saddr_size = sizeof saddr;
		data_size= recvfrom(sock_raw, buf, ACK_FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
		packet_format *pckt2=(packet_format*)(buf);
		if(pckt2->hdr.packet_type==9999)
		{
			printf("ack pckt value of packet_type is %d\n",pckt2->hdr.packet_type);
		    printf("ack pckt value of src address is %d\n",pckt2->hdr.src_addr);
		    printf("ack pckt value of dst address is %d\n",pckt2->hdr.dest_addr);
		    printf("ack pckt value of sequence number is %d\n",pckt2->hdr.seq_no);
		    // struct ethhdr *eth_ptr=(struct ethhdr *)(buf);
		    // printf("Packet src mac address in HEX inside our packet before change is : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n", eth_ptr->h_source[0] , eth_ptr->h_source[1] , eth_ptr->h_source[2] , eth_ptr->h_source[3] , eth_ptr->h_source[4] , eth_ptr->h_source[5]);
		    // printf("Packet destination mac address in HEX inside our packet before change is : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n", eth_ptr->h_dest[0] , eth_ptr->h_dest[1] , eth_ptr->h_dest[2] , eth_ptr->h_dest[3] , eth_ptr->h_dest[4] , eth_ptr->h_dest[5]);
		    printf("ack The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt2->data_inside);
		    printf("||||||||||||||||||||||ACK PACKET||||||||||||||||||||||\n");
		    printf("||||||||||||||||||||||ACK PACKET||||||||||||||||||||||\n");
	        printf("||||||||||||||||||||||ACK PACKET||||||||||||||||||||||\n");
		}
	}	

    close(sock_raw);
}   

int main(int argc, char const *argv[])
{
	file_des=open(FILE_NAME,O_RDONLY);
	fstat(file_des,&buffer);

	file_len = (unsigned int)buffer.st_size;
	data_array = (unsigned char*)mmap(0, file_len, PROT_READ, MAP_FILE|MAP_PRIVATE, file_des, 0);

	// send_data_func();
	pthread_create (&sender_thread, NULL, send_data_func, NULL);
	pthread_create (&ack_thread, NULL, recv_data_func, NULL);

	pthread_join (sender_thread, NULL);
	pthread_join (ack_thread, NULL);
	return 0;
}
