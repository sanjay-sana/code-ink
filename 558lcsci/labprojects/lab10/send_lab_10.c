#include "custom_globals.h"

int curr_packet_no=0;
int file_flag=0;
float global_end_time;
float global_end_time_end;
pthread_t send_data;

void* send_data_func(void* arg)

{
	 int fd1;
	unsigned char *data_array;
	struct stat st;
	int file_length;
	int packet_s;
	int seq_no_max;
	//printf("The size of packet format data structure is %d\n",sizeof(packet_format));
	//printf("The size of packet header data structure is %d\n",sizeof(header_format));
	packet_format new_packet;
	int format_data_size=sizeof(new_packet.data_inside);
	//printf("The size of packet format data part is %d\n",format_data_size);
	packet_s=sizeof(new_packet);
	//printf("The size of the initialized packed is %d\n",packet_s);
	
	// Opening a file for reading 
 fd1=open("testfile_lab5.txt",O_RDONLY);
  if (fd1 < 0)
    {
    fprintf(stderr,"Error: Unable to read dictionary file\n");
    return NULL;;
	}
     char *file_name=(char*)malloc(20*sizeof(char));
	 strcpy(file_name,"testfile_lab5.txt");

	//Doing the mmap of the file
	if (stat(file_name, &st) == 0)
       file_length=(int)st.st_size;
    else	
    fprintf(stderr, "Cannot determine size of %s: %s\n",file_name, strerror(errno));
	//printf("The size of the file is determined to be %d\n",file_length);
	
	data_array = (unsigned char*)mmap(0,file_length,PROT_READ,MAP_FILE|MAP_PRIVATE,fd1,0);
	
	if (data_array == MAP_FAILED)
		printf("Error with mmap %d", errno);
	else
		printf("Mapping done successfully - Proceed\n");
	
    // Determining the maximum sequence number;
	if(file_length%format_data_size==0)
	 seq_no_max=(file_length/format_data_size);
   else
	   seq_no_max=(file_length/format_data_size)+1;
   
   int s; /*socketdescriptor*/

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s == -1) 
	{
		printf("The socket was not created successfully\n");
	}
	struct sockaddr_ll socket_address;
	
	     char pcap_send_errbuf[PCAP_ERRBUF_SIZE];
	     pcap_send_errbuf[0]='\0';
		pcap_t* pcap_send=pcap_open_live("eth0",NEW_ETH_FRAME_LEN,0,0,pcap_send_errbuf);
			int dir_ret_send=pcap_setdirection(pcap_send,PCAP_D_OUT );
			printf("The value of dir_ret_send is %d\n",dir_ret_send);
			if(dir_ret_send==0)
				printf("The outbound direction (for packets going t orouter) for pcap_send handle is successfully set\n");
			if(dir_ret_send<0)
			{
				printf("Error on setting the outbound direction for pcap_send handle\n");
				exit(1);
			}
			if (pcap_send_errbuf[0]!='\0') {
			    fprintf(stderr,"%s\n",pcap_send_errbuf);
			}
			if (!pcap_send) {
			    exit(1);
			}

	
	struct timeval start;
	gettimeofday(&start, NULL);
    printf("The sending of packets start the following timestamp : %f\n", start.tv_sec + (double)(start.tv_usec/1e6));
    global_end_time=start.tv_sec + (double)(start.tv_usec/1e6);
	global_end_time_end=global_end_time;
	
	while(curr_packet_no<seq_no_max)
   {
	   int head_value=0;
	   printf("The packet being made is packet number %d\n",curr_packet_no);
       void* buffer = (void*)malloc(NEW_ETH_FRAME_LEN);
	   memset(buffer,0,NEW_ETH_FRAME_LEN);
	   unsigned char* etherhead = buffer;
	   unsigned char* data_head = buffer + 14;
	   struct ethhdr *eh = (struct ethhdr *)etherhead;
	   int send_result = 0;
	   unsigned char src_mac[6] = {0x00, 0x15, 0x17, 0x5d, 0x34, 0x48};
	   unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5d, 0x2a, 0x0c};
	  memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
      memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
       eh->h_proto = 0x00;
       
	   //updating the custom header value
	   memset(&new_packet,0,sizeof(new_packet));
	   
	   new_packet.hdr.packet_type=9999;
	   new_packet.hdr.src_addr=2;
	   new_packet.hdr.dest_addr=0;
	   new_packet.hdr.seq_no=curr_packet_no;
	   
	   if (file_length - (curr_packet_no * DATA_INSIDE_LENGTH) > DATA_INSIDE_LENGTH)
        {
            memcpy( new_packet.data_inside , &data_array[curr_packet_no * DATA_INSIDE_LENGTH] , DATA_INSIDE_LENGTH);
        }
        else
		{
			memcpy( new_packet.data_inside , &data_array[curr_packet_no * DATA_INSIDE_LENGTH] , file_length - (curr_packet_no * DATA_INSIDE_LENGTH));
		}
		
		memcpy((void*)data_head,(void*)&new_packet,sizeof(new_packet));
		packet_format *pckt=(packet_format*)(buffer+14);
		printf("pckt value of sequence number  in the initial loop is %d\n",pckt->hdr.seq_no);
	   struct ethhdr *eth_ptr=(struct ethhdr *)(buffer);
	   
	   int bytes_written=pcap_inject(pcap_send,buffer,1398);
			if (bytes_written==-1)
			{
			    pcap_perror(pcap_send,0);
			    pcap_close(pcap_send);
			    return NULL;;
			}
			else
			{
				printf("The number of bytes successfully written is %d\n",bytes_written);
			}
		curr_packet_no++;
       
      }
}
     


int main()
{
	
	pthread_create(&send_data,NULL,send_data_func,NULL);
	
	//pthread_create(&get_ack,NULL,recv_ack_func,NULL);
	
	pthread_join(send_data,NULL);
	
	//pthread_join(get_ack,NULL);
	
	

   return(0);
}
