#include "custom_globals.h"

#define MY_DEST_MAC0 0x00
#define MY_DEST_MAC1 0x04
#define MY_DEST_MAC2 0x23
#define MY_DEST_MAC3 0xA6
#define MY_DEST_MAC4 0x56
#define MY_DEST_MAC5 0x70

#define INTERFACE_TO_RTR1 "eth3"

pthread_mutex_t datagram_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ack_mutex = PTHREAD_MUTEX_INITIALIZER;

packet_format datagram_recv, datagram_sent;

unsigned char *data_array;
struct stat buffer;
int packet_count=0;
int ack_count=0;
int *datagram_status;
pthread_t sender_thread, ack_thread;

int datagram_total=0;

struct timeval start, end;

int *retransmit_status;
int *retransmit_status1;

char set_flag = 'n';

FILE *fp;
unsigned int len;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void *SendData(void *inputargv)
{
	//SENDING DATA 
	printf("Sending data\n");
	int send_result;

    //Making the socket for sending to the router1
	int sockfd;
	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	int packet_num = 0;
	packet_count = buffer.st_size / DATA_LENGTH;
	if (buffer.st_size % DATA_LENGTH > 0)
	{
		packet_count++;
	}
	// printf("packet_count %d\n", packet_count);

	datagram_status=(int*)calloc(packet_count, sizeof(int));
	// printf("Size data ................................  %u\n", sizeof(datagram_status));
	memset(datagram_status , 0 , packet_count*sizeof(int));
	printf("Packet count: %d\n", packet_count);

	// Prepare packet to send
	unsigned char* buf = (unsigned char*)malloc(FRAME_LEN);
	memset(buf, 0, FRAME_LEN);

	packet_format *packet_send = (packet_format*)malloc(sizeof(packet_format));
	memset(packet_send, 0, sizeof(packet_format));

	// printf("Size of packet  %lu\n", sizeof(packet_format));
	// printf("Size of packet hdr  %lu\n", sizeof(packet_send->hdr));

	(packet_send->hdr).packet_type=9999;
	(packet_send->hdr).src_addr=0;
	(packet_send->hdr).dest_addr=2;
	// (packet_send->hdr).file_size = buffer.st_size;
	//(packet_send->hdr).seq_no=packet_num;

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (INTERFACE_TO_RTR1)) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit (EXIT_FAILURE);
	}
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY_DEST_MAC0;
	device.sll_addr[1]  = MY_DEST_MAC1;     
	device.sll_addr[2]  = MY_DEST_MAC2;
	device.sll_addr[3]  = MY_DEST_MAC3;
	device.sll_addr[4]  = MY_DEST_MAC4;
	device.sll_addr[5]  = MY_DEST_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	// Timer starts
	gettimeofday(&start, NULL);
	printf("Start: %f\n", start.tv_sec + (double)(start.tv_usec/1e6));
	do
	{ 
		// printf("yes0 %d\n", packet_num);
		(packet_send->hdr).seq_no = packet_num;
		if (buffer.st_size - (packet_num * DATA_LENGTH) > DATA_LENGTH)
		{
				memcpy(packet_send->data_inside , &data_array[packet_num * DATA_LENGTH] , DATA_LENGTH);
		}
		else
		{
				memcpy(packet_send->data_inside, &data_array[packet_num * DATA_LENGTH] , buffer.st_size - (packet_num * DATA_LENGTH));
		}
		// printf("yes2\n");
		memcpy((void*)buf, (void*)packet_send, FRAME_LEN);
		packet_format *pckt=(packet_format*)buf;
		printf("***************\n");
		printf("(packet_send->hdr).seq_no = %d \n", (pckt->hdr).packet_type);
		printf("(packet_send->hdr).seq_no = %d \n", (pckt->hdr).src_addr);
		printf("(packet_send->hdr).seq_no = %d \n", (pckt->hdr).dest_addr);
		printf("(packet_send->hdr).seq_no = %d \n", (pckt->hdr).seq_no);
		// printf("(packet_send->hdr).seq_no = %d \n", (pckt->hdr).file_size);
		printf("***************\n");
		usleep(1000);
		// printf("before send0\n");
		send_result = sendto(sockfd, buf, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
		// printf("after send0\n");
		if (send_result == -1) 
		{ 
			printf("Sending of packet %d failed\n",packet_num);
			exit(0);
		}
		// printf("yes3\n");
		packet_num++;
		// printf("yes3.3\n");
		memset(packet_send->data_inside, 0, DATA_LENGTH);
		// printf("yes3.5\n");
		memset(buf, 0, FRAME_LEN);
		(packet_send->hdr).seq_no = 0;
		// printf("yes4 %d\n", packet_num);
	}while(packet_num < packet_count);
	printf("first set done.....\n");
	//RETRANSMISSION OF DATA

		// printf("Ack count = %d\n", ack_count);
		// printf("pck count = %d\n", packet_count);

		// printf("Bit Array description\n");
		// printf("**********************\n");
		// printf("datagram_status[0]: %d\n", datagram_status[0]);
		// printf("datagram_status[1]: %d\n", datagram_status[1]);
		// printf("datagram_status[2]: %d\n", datagram_status[2]);
		// printf("datagram_status[3]: %d\n", datagram_status[3]);
		// printf("datagram_status[4]: %d\n", datagram_status[4]);
		// printf("datagram_status[5]: %d\n", datagram_status[5]);
		// printf("datagram_status[6]: %d\n", datagram_status[6]);
		// printf("datagram_status[7]: %d\n", datagram_status[7]);
		// printf("datagram_status[8]: %d\n", datagram_status[8]);
		// printf("datagram_status[9]: %d\n", datagram_status[9]);
		// printf("datagram_status[10]:%d\n", datagram_status[10]);
		// printf("datagram_status[11]:%d\n", datagram_status[11]);
		// printf("**********************\n");
	// printf("Retransmission starts 1.......\n");
/*	pthread_mutex_lock(&ack_mutex);
	if (ack_count < packet_count)
	{
		pthread_mutex_unlock(&ack_mutex);
		printf("Retransmission starts.......\n");
		usleep(400);
		// printf("Ack count = %d\n", ack_count);
		// printf("pck count = %d\n", packet_count);
		int retransmit_count=0;
		retransmit_status = (int*)calloc(packet_count, sizeof(int));
		int i;
		for (i = 0; i < packet_count; ++i)
		{
			pthread_mutex_lock(&datagram_mutex);
			if (datagram_status[i] == 0)
			{
				pthread_mutex_unlock(&datagram_mutex);
				retransmit_status[retransmit_count] = (packet_send->hdr).seq_no;
				retransmit_count++;
				(packet_send->hdr).seq_no = i;
				printf("seq num %d\n", (packet_send->hdr).seq_no);
				if (buffer.st_size - (i * DATA_LENGTH) > DATA_LENGTH)
				{
					memcpy( packet_send->data_inside , &data_array[i * DATA_LENGTH] , DATA_LENGTH);
				}
				else
				{
					memcpy( packet_send->data_inside , &data_array[i * DATA_LENGTH] , buffer.st_size - (i * DATA_LENGTH));
				}
				(packet_send->hdr).file_size=buffer.st_size;
				memcpy((void*)buf, (void*)packet_send, sizeof(packet_format));
				packet_format *pckt=(packet_format*)buf;
				printf("before send1\n");
				send_result = sendto(sockfd, buf, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
				printf("after send1\n");
			}
			else
			{
				pthread_mutex_unlock(&datagram_mutex);
			}
		}
		printf("second set done\n");
		retransmit_status1 = (int*)calloc(retransmit_count, sizeof(int));
		set_flag = 'y';
		int iterator = retransmit_count;
		while(1)
		{
			usleep(300);
			int k=0;
			// printf("iterator ====== %d\n", iterator);
			// printf("yes6\n");
			pthread_mutex_lock(&ack_mutex);
			if (ack_count < packet_count)
			{
				pthread_mutex_unlock(&ack_mutex);
				int i;
				for (i = 0; i < iterator; ++i)
				{
					pthread_mutex_lock(&datagram_mutex);
					if (datagram_status[retransmit_status[i]] == 0)
					{
						pthread_mutex_unlock(&datagram_mutex);
						retransmit_status1[k] = retransmit_status[i];
						// printf("yes6.5\n");
						(packet_send->hdr).seq_no = retransmit_status1[k];
						// printf("yes7\n");
						if (buffer.st_size - (retransmit_status[i] * DATA_LENGTH) > DATA_LENGTH)
						{
							memcpy( packet_send->data_inside , &data_array[retransmit_status[i] * DATA_LENGTH] , DATA_LENGTH);
						}
						else
						{
							memcpy( packet_send->data_inside , &data_array[retransmit_status[i] * DATA_LENGTH] , buffer.st_size - (retransmit_status[i] * DATA_LENGTH));
						}
						// printf("yes8\n");
						memcpy((void*)buf, (void*)packet_send, sizeof(packet_format));
						packet_format *pckt=(packet_format*)buf;
						// printf("yes8.5\n");
						(packet_send->hdr).file_size=buffer.st_size;
						// printf("yes8.7\n");
						// usleep(20);
						printf("before send2\n");
						send_result = sendto(sockfd, buf, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
						printf("after send2\n");
						// printf("yes8.8\n");
						k++;
					}
					else
					{
						pthread_mutex_unlock(&datagram_mutex);
					}
				}
				// printf("yes9\n");
				free(retransmit_status);
				retransmit_status = (int*)calloc(k, sizeof(int));
				memcpy( retransmit_status , retransmit_status1 , k * sizeof(int));
				free(retransmit_status1);
				retransmit_status1 = (int*)calloc(k, sizeof(int));
				// printf("Value of k ===========> %d\n", k);
				iterator = k;
			}
			else
			{
				pthread_mutex_unlock(&ack_mutex);
				break;
			}
		}
	}
	pthread_mutex_unlock(&ack_mutex);
	close(sockfd);
	pthread_exit(0); */
	return 0;

}

ack_format *ack_recv;
void *ReceiveAck(void *inputargv)
{
	char **argvar = (char**)inputargv;
	printf("Receiving Acks\n");
	int sockfd;
	// int dg_ack;
	int saddr_size , data_size;
	struct sockaddr saddr;
	sockfd = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	if (sockfd == -1) { 
		 printf("socket not created properly\n");
	}

	unsigned char *buf_ack = (unsigned char *) malloc(ACK_FRAME_LEN);
	while(1)
	{
		saddr_size = sizeof saddr;
		data_size= recvfrom(sockfd, buf_ack, ACK_FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
		if (data_size == -1)
		{
			perror("recvfrom");
			exit(1);
		}
		ack_recv=(ack_format*)(buf_ack);
		if (ack_recv->hdr_ack.packet_type == 9999 && ack_recv->hdr_ack.dest_addr == 0)
		{
			if ((ack_recv->hdr_ack).seq_no == 99999999)
			{
				break;
			}
			pthread_mutex_lock(&datagram_mutex);
			pthread_mutex_lock(&ack_mutex);
			if (datagram_status[(ack_recv->hdr_ack).seq_no] != 1)
			{
				datagram_status[(ack_recv->hdr_ack).seq_no]=1;
				// printf("Ack seque num = %d\n", (ack_recv->hdr_ack).seq_no);
				ack_count++;
			}
			pthread_mutex_unlock(&ack_mutex);
			pthread_mutex_unlock(&datagram_mutex);
		}
	}

	close(sockfd);
	pthread_cancel(sender_thread);
	pthread_exit(0);
	return 0;
}

int main(int argc, char *argv[])
{
	int file_des;
	int page_size;
	file_des=open(FILE_NAME,O_RDONLY);

	if (file_des < 0)
	{
		fprintf(stderr,"Error: Unable to read dictionary file\n");
		return 0;
	}
	if (fstat(file_des,&buffer) < 0)
	{
		fprintf(stderr,"Error: Unable to determine file size\n");
		return 0;
	}

	len = (unsigned int)buffer.st_size;
	data_array = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,file_des,0);
	if (!data_array)
	{
			fprintf(stderr,"Error: Unable to memory map dictionary!\n");
	}

	// page_size = getpagesize();

	pthread_create(&sender_thread, 0, SendData, (void *)argv);
	// pthread_create(&ack_thread, 0, ReceiveAck, (void *)argv);

	// pthread_join(ack_thread, 0);
	pthread_join(sender_thread, 0);

	free(datagram_status);
	if (set_flag == 'y')
	{
			free(retransmit_status);
			free(retransmit_status1);
	}
	munmap(data_array,buffer.st_size);
	return 0;
}
