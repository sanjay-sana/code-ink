#include "custom_globals.h"
#define MY_DEST_MAC0 0x00
#define MY_DEST_MAC1 0x04
#define MY_DEST_MAC2 0x23
#define MY_DEST_MAC3 0xA6
#define MY_DEST_MAC4 0x58
#define MY_DEST_MAC5 0x42


#define INTERFACE_SECOND "eth3"

pthread_mutex_t datagram_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ack_mutex = PTHREAD_MUTEX_INITIALIZER;

// packet_format datagram_recv, datagram_sent;

unsigned char *data_array;
struct stat buffer;
int packet_count=0;
int ack_count=0;
int *datagram_status;
pthread_t sender_thread, ack_thread;

int datagram_total=0;

struct timeval end;

int *retransmit_status;
int *retransmit_status1;

char set_flag = 'n';

FILE *fp;
unsigned int len;

packet_format *packet_recv;

int recv_data_func()
{
	printf("The sniffing loop starts here ... \n");
	fp = fopen(FILENAME, "wb");
	int sockfd,n;
	int saddr_size , data_size;
	struct sockaddr saddr;

	sockfd = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if (sockfd == -1)
    { 
      printf("socket not created properly\n");
    }

	int buff_alloc_flag = 0;
	int tot_count = 0;

	int once = 0;

	char *file_array;
    char end_flag = 'n';

    unsigned char *buf = (unsigned char *) malloc(FRAME_LEN);
	while(1)
	{
        int custom_packet_received = 0;
		saddr_size = sizeof saddr;
		//memset(packet_recv, 0, sizeof(packet_format));
        // printf("yes1\n");
        if (end_flag == 'n')
        {
            memset(buf , 0 , sizeof(FRAME_LEN));
    		data_size= recvfrom(sockfd, buf, FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
            usleep(1000);
			packet_recv=(packet_format*)(buf);
            header_format *temp = (header_format*)malloc(sizeof(header_format));
            temp = (header_format*)(buf);
            if (packet_recv->hdr.packet_type == 9999 && packet_recv->hdr.dest_addr == 2)
            {
                // if (temp->seq_no == 10485760)
                // {
                //     temp->seq_no = 129;
                //     temp->file_size = 10485760;
                // }
                printf("yes2\n");
                printf("temp packet_type %d\n", temp->packet_type);
                printf("temp src_addr %d\n", temp->src_addr);
                printf("temp dest_addr %d\n", temp->dest_addr);
                printf("temp seq_no %d\n", temp->seq_no);
                printf("temp file_size %d\n", temp->file_size);
                custom_packet_received = 1;
        		if (buff_alloc_flag == 0)
        		{
                    printf("yes3 if cond");
        			float check = (float)((packet_recv->hdr).file_size)/(float)DATA_LENGTH;
        			tot_count = ceil(check);
        			datagram_status=(int*)malloc(tot_count * sizeof(int));
        			memset(datagram_status , 0 , tot_count * sizeof(int));

        			printf("Total file size = %d\n", (packet_recv->hdr).file_size);
        			file_array=(char*)malloc((packet_recv->hdr).file_size);
    				buff_alloc_flag = 1;
        		}
                printf("yes3 ");
                // printf("%d\n", (packet_recv->hdr).seq_no);
        		if (datagram_status[(packet_recv->hdr).seq_no] != 1)
        		{
                    printf("yes4\n");
        			datagram_status[(packet_recv->hdr).seq_no] = 1;
                    printf("yes5\n");
        			datagram_total++;
                    printf("yes6\n");
                    // printf("Data seque num = %d\n", (packet_recv->hdr).seq_no);
        			printf("datagram_total = %d\n", datagram_total);
        			if ((packet_recv->hdr).file_size - (((packet_recv->hdr).seq_no) * DATA_LENGTH) < DATA_LENGTH)
        			{
                        printf("yes11\n");
        				memcpy( &file_array[(packet_recv->hdr).seq_no * DATA_LENGTH] , packet_recv->data_inside , (packet_recv->hdr).file_size - ((packet_recv->hdr).seq_no * DATA_LENGTH));
        			     printf("yes12\n");
                    }
        			else
        			{
        				memcpy( &file_array[(packet_recv->hdr).seq_no * DATA_LENGTH] , packet_recv->data_inside , DATA_LENGTH);
        			}
                    printf("yes7\n");
        		}
                printf("yes10\n");
            }
        }
        if (custom_packet_received == 1)
        {
        /*
            unsigned char* buf_ack = (unsigned char*)malloc(ACK_FRAME_LEN);
       		memset(buf_ack, 0, ACK_FRAME_LEN);

        	ack_format *ack_send = (ack_format*)malloc(sizeof(ack_format));
        	memset(ack_send, 0, sizeof(ack_format));

        	(ack_send->hdr_ack).packet_type=9999;
    	    (ack_send->hdr_ack).src_addr=2;
    	    (ack_send->hdr_ack).dest_addr=0;
    	    (ack_send->hdr_ack).file_size = (packet_recv->hdr).file_size;

            struct sockaddr_ll device={0};
    	    memset (&device, 0, sizeof (device));
    	    if ((device.sll_ifindex = if_nametoindex (INTERFACE_SECOND)) == 0)
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

    		if (end_flag == 'y')
            {
                (ack_send->hdr_ack).seq_no=99999999;
        		memcpy(ack_send->data_in_ack , "AAAA" , ACK_DATA_LEN);
                int i;
                memcpy((void*)buf_ack, (void*)ack_send, sizeof(ack_format));
            	
                for (i = 0; i < 20; ++i)
                {
                    n = sendto(sockfd, buf_ack, ACK_FRAME_LEN,0, (struct sockaddr*)&device, sizeof(device));
                }
                break;
            }
            else
            {
            	(ack_send->hdr_ack).seq_no = (packet_recv->hdr).seq_no ;
                memcpy(ack_send->data_in_ack , "AAAA" , ACK_DATA_LEN);
                n = sendto(sockfd, buf_ack, ACK_FRAME_LEN,0, (struct sockaddr*)&device, sizeof(device));
            }*/
            // printf("yes5\n");
    		if (datagram_total == tot_count && once == 0)
    		{
    			once = 1;
    			printf("file_array length = %u\n", strlen(file_array));
    			printf("file_size = %d\n", (packet_recv->hdr).file_size);
    			fwrite(file_array, (packet_recv->hdr).file_size, 1,fp);
    			fflush(fp);
    			fclose(fp);
                gettimeofday(&end, NULL);
                printf("End time: %f\n", end.tv_sec + (double)(end.tv_usec/1e6));
                end_flag = 'y';
    		}
        }
	}
	close(sockfd);
	//return 0;
}

int main()
{
	recv_data_func();
	return 0;
}