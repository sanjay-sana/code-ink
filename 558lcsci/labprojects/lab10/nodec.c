#include "custom_globals.h"

#define FILENAME_fromA "testfile_lab5_copy_AC.bin"
#define FILENAME_fromB "testfile_lab5_copy_BC.bin"

int mycase=0;
int nodea_flag=0;
int nodeb_flag=0;

//Sender variables
int curr_packet_no=0;
float global_end_time;
float global_end_time_end;
pthread_t send_data;

//Receiver variables
int curr_packet_no_fromA=0;
int curr_packet_no_fromB=0;
char *file_array_fromA=NULL;
char *file_array_fromB=NULL;
int file_length_recv=INP_FILE_LEN;
int ending_flag=0;
FILE *fd1;
FILE *fd2;

int total_pckt_recv_fromA=0;
int total_pckt_recv_fromB=0;
unsigned char *array_write;
pthread_t recv_data;


void* send_data_func(void* arg)

{
	int fd1_send;
	unsigned char *data_array;
	struct stat st;
	int file_length;
	int packet_s;
	int seq_no_max;
	packet_format new_packet;
	int format_data_size=sizeof(new_packet.data_inside);

	packet_s=sizeof(new_packet);
	//printf("The size of the initialized packed is %d\n",packet_s);

	// Opening a file for reading 
	fd1_send=open(INP_FILE,O_RDONLY);
	if (fd1_send < 0)
	{
		fprintf(stderr,"Error: Unable to read dictionary file\n");
		return NULL;
	}
	char *file_name=(char*)malloc(20*sizeof(char));
	strcpy(file_name,INP_FILE);

	//Doing the mmap of the file
	if (stat(file_name, &st) == 0)
		file_length=(int)st.st_size;
	else	
		fprintf(stderr, "Cannot determine size of %s: %s\n",file_name, strerror(errno));
	//printf("The size of the file is determined to be %d\n",file_length);

	data_array = (unsigned char*)mmap(0,file_length,PROT_READ,MAP_FILE|MAP_PRIVATE,fd1_send,0);

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
	pcap_t* pcap_send=pcap_open_live("eth4",NEW_ETH_FRAME_LEN,0,0,pcap_send_errbuf);
	int dir_ret_send=pcap_setdirection(pcap_send,PCAP_D_OUT );
	printf("The value of dir_ret_send is %d\n",dir_ret_send);
	if(dir_ret_send==0)
		printf("The outbound direction (for packets going t orouter) for pcap_send handle is successfully set\n");
	if(dir_ret_send<0)
	{
		printf("Error on setting the outbound direction for pcap_send handle\n");
		exit(1);
	}
	if (pcap_send_errbuf[0]!='\0')
	{
		fprintf(stderr,"%s\n",pcap_send_errbuf);
	}
	if (!pcap_send)
	{
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

		unsigned char src_mac[6] = {0xA0, 0x36, 0x9F, 0x0A, 0x5B, 0x5E};
		unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
		if(nodea_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x34, 0x48};
		{
			dest_mac[0] = 0xA0;
			dest_mac[1] = 0x36;
			dest_mac[2] = 0x9F;
			dest_mac[3] = 0x0A;
			dest_mac[4] = 0x5B;
			dest_mac[5] = 0x76;
		}
		if(nodeb_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x29, 0xA4};
		{
			dest_mac[0] = 0xA0;
			dest_mac[1] = 0x36;
			dest_mac[2] = 0x9F;
			dest_mac[3] = 0x08;
			dest_mac[4] = 0x53;
			dest_mac[5] = 0xAE;
		}
		memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
		memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
		eh->h_proto = 0x00;

		//updating the custom header value
		memset(&new_packet,0,sizeof(new_packet));

		new_packet.hdr.packet_type=9999;
		new_packet.hdr.src_addr=2;
		if(nodea_flag==1)
			new_packet.hdr.dest_addr=0;
		if(nodeb_flag==1)
			new_packet.hdr.dest_addr=1;

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

void* send_data_to_both_func(void* arg)

{
	int fd1_send;
	unsigned char *data_array;
	struct stat st;
	int file_length;
	int packet_s;
	int seq_no_max;
	packet_format new_packet;
	int format_data_size=sizeof(new_packet.data_inside);

	packet_s=sizeof(new_packet);
	//printf("The size of the initialized packed is %d\n",packet_s);

	// Opening a file for reading 
	fd1_send=open(INP_FILE,O_RDONLY);
	if (fd1_send < 0)
	{
		fprintf(stderr,"Error: Unable to read dictionary file\n");
		return NULL;
	}
	char *file_name=(char*)malloc(20*sizeof(char));
	strcpy(file_name, INP_FILE);

	//Doing the mmap of the file
	if (stat(file_name, &st) == 0)
		file_length=(int)st.st_size;
	else	
		fprintf(stderr, "Cannot determine size of %s: %s\n",file_name, strerror(errno));
	//printf("The size of the file is determined to be %d\n",file_length);

	data_array = (unsigned char*)mmap(0,file_length,PROT_READ,MAP_FILE|MAP_PRIVATE,fd1_send,0);

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
	pcap_t* pcap_send=pcap_open_live("eth4",NEW_ETH_FRAME_LEN,0,0,pcap_send_errbuf);
	int dir_ret_send=pcap_setdirection(pcap_send,PCAP_D_OUT );
	printf("The value of dir_ret_send is %d\n",dir_ret_send);
	if(dir_ret_send==0)
		printf("The outbound direction (for packets going t orouter) for pcap_send handle is successfully set\n");
	if(dir_ret_send<0)
	{
		printf("Error on setting the outbound direction for pcap_send handle\n");
		exit(1);
	}
	if (pcap_send_errbuf[0]!='\0')
	{
		fprintf(stderr,"%s\n",pcap_send_errbuf);
	}
	if (!pcap_send)
	{
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

		unsigned char src_mac[6] = {0xA0, 0x36, 0x9F, 0x0A, 0x5B, 0x5E};
		unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
		// if(nodeb_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x34, 0x48};
		// {
			dest_mac[0] = 0xA0;
			dest_mac[1] = 0x36;
			dest_mac[2] = 0x9F;
			dest_mac[3] = 0x0A;
			dest_mac[4] = 0x5B;
			dest_mac[5] = 0x76;
		// }
		// if(nodec_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x29, 0xA4};
		// {
		// 	dest_mac[0] = 0x00;
		// 	dest_mac[1] = 0x15;
		// 	dest_mac[2] = 0x17;
		// 	dest_mac[3] = 0x5D;
		// 	dest_mac[4] = 0x29;
		// 	dest_mac[5] = 0xA4;
		// }
		memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
		memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
		eh->h_proto = 0x00;

		//updating the custom header value
		memset(&new_packet,0,sizeof(new_packet));

		new_packet.hdr.packet_type=9999;
		new_packet.hdr.src_addr=2;
		// if(nodeb_flag==1)
			new_packet.hdr.dest_addr=0;
		// if(nodec_flag==1)
		// 	new_packet.hdr.dest_addr=2;

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
	printf("Sent packet to nodeB\n");
	curr_packet_no = 0;
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

		unsigned char src_mac[6] = {0xA0, 0x36, 0x9F, 0x0A, 0x5B, 0x5E};
		unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
		// if(nodeb_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x34, 0x48};
		// {
		// 	dest_mac[0] = 0x00;
		// 	dest_mac[1] = 0x15;
		// 	dest_mac[2] = 0x17;
		// 	dest_mac[3] = 0x5D;
		// 	dest_mac[4] = 0x34;
		// 	dest_mac[5] = 0x48;
		// }
		// if(nodec_flag==1)
		//unsigned char dest_mac[6] = {0x00, 0x15, 0x17, 0x5D, 0x29, 0xA4};
		// {
			dest_mac[0] = 0xA0;
			dest_mac[1] = 0x36;
			dest_mac[2] = 0x9F;
			dest_mac[3] = 0x08;
			dest_mac[4] = 0x53;
			dest_mac[5] = 0xAE;
		// }
		memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
		memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
		eh->h_proto = 0x00;

		//updating the custom header value
		memset(&new_packet,0,sizeof(new_packet));

		new_packet.hdr.packet_type=9999;
		new_packet.hdr.src_addr = 2;
		// if(nodeb_flag==1)
		// 	new_packet.hdr.dest_addr=1;
		// if(nodec_flag==1)
			new_packet.hdr.dest_addr = 1;

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
	printf("Sent packet to nodeC\n");
}

void process_packet_intf0(u_char *args, const struct pcap_pkthdr *header,const u_char *old_buffer)
{
	void *buffer=(void*)old_buffer;
	int size = header->len;
	int captured_length=header->caplen;
	packet_format *pckt=(packet_format*)(buffer+14);

	if(pckt->hdr.packet_type==9999)
	{
		if(pckt->hdr.src_addr==0)
		{
			total_pckt_recv_fromA++;
			printf("The icoming packet is coming from node A\n");
		}
		if(pckt->hdr.src_addr==1)
		{
			total_pckt_recv_fromB++;
			printf("The incoming packet is coming from node B\n");
		}
  
		//printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);	
		printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);
		struct ethhdr *eth_ptr=(struct ethhdr *)(buffer);
		if(pckt->hdr.src_addr==0)
			curr_packet_no_fromA=pckt->hdr.seq_no;
		if(pckt->hdr.src_addr==1)
			curr_packet_no_fromB=pckt->hdr.seq_no;
		if(pckt->hdr.seq_no==0 && pckt->hdr.src_addr==0 )
			file_array_fromA=(char*)malloc(file_length_recv*sizeof(char));
		if(pckt->hdr.seq_no==0 && pckt->hdr.src_addr==1 )
			file_array_fromB=(char*)malloc(file_length_recv*sizeof(char));
	
		if(pckt->hdr.src_addr == 0)
		{
			if (file_length_recv - (curr_packet_no_fromA * DATA_INSIDE_LENGTH) > DATA_INSIDE_LENGTH)
			{
				memcpy( &file_array_fromA[curr_packet_no_fromA * DATA_INSIDE_LENGTH] ,pckt->data_inside, DATA_INSIDE_LENGTH);
			}
			else
			{
				memcpy( &file_array_fromA[curr_packet_no_fromA * DATA_INSIDE_LENGTH] ,pckt->data_inside, file_length_recv - (curr_packet_no_fromA * DATA_INSIDE_LENGTH));
			}
	
			printf("Value of total_pckt_recv from A is %d\n",total_pckt_recv_fromA);
			if(total_pckt_recv_fromA == TOT_PKT)
			{
				//printf("It did enter here\n");
				printf("All the packets received from A ... writing to file\n");
				fwrite(file_array_fromA, INP_FILE_LEN, 1,fd1);
				struct timeval end_fromA;
				gettimeofday(&end_fromA, NULL);
				printf("The file writing for packets coming from A is completed at the following timestamp : %f\n", end_fromA.tv_sec + (double)(end_fromA.tv_usec/1e6));
				fflush(fd1);
				fclose(fd1);
				printf("Writing of file done\n");
				//exit(0);
			}
		}
		if(pckt->hdr.src_addr == 1)
		{
			if (file_length_recv - (curr_packet_no_fromB * DATA_INSIDE_LENGTH) > DATA_INSIDE_LENGTH)
			{
				memcpy( &file_array_fromB[curr_packet_no_fromB * DATA_INSIDE_LENGTH] ,pckt->data_inside, DATA_INSIDE_LENGTH);
			}
			else
			{
				memcpy( &file_array_fromB[curr_packet_no_fromB * DATA_INSIDE_LENGTH] ,pckt->data_inside, file_length_recv - (curr_packet_no_fromB * DATA_INSIDE_LENGTH));
			}
			printf("Value of total_pckt_recv from B is %d\n",total_pckt_recv_fromB);
			if(total_pckt_recv_fromB==TOT_PKT)
			{
				//printf("It did enter here\n");
				printf("All the packets received from B ... writing to file\n");
				fwrite(file_array_fromB, INP_FILE_LEN, 1,fd2);
				struct timeval end_fromA;
				gettimeofday(&end_fromA, NULL);
				printf("The file writing for packets coming from B is completed at the following timestamp : %f\n", end_fromA.tv_sec + (double)(end_fromA.tv_usec/1e6));
				fflush(fd2);
				fclose(fd2);
				printf("Writing of file done\n");
				// exit(0);
			}
		}
	}
}

void* recv_data_func(void* arg)
{
	fd1=fopen(FILENAME_fromA,"wb");
	fd2=fopen(FILENAME_fromB,"wb");

	char *file_name_fromA=(char*)malloc(30*sizeof(char));
	strcpy(file_name_fromA,FILENAME_fromA);

	char *file_name_fromB=(char*)malloc(30*sizeof(char));
	strcpy(file_name_fromB,FILENAME_fromB);

	char pcap_recv_errbuf[PCAP_ERRBUF_SIZE];
	pcap_recv_errbuf[0]='\0';
	pcap_t* pcap_recv=pcap_open_live("eth4",NEW_ETH_FRAME_LEN,0,0,pcap_recv_errbuf);
	int dir_recv=pcap_setdirection(pcap_recv,PCAP_D_IN );
	printf("The value of dir_recv is %d\n",dir_recv);
	if(dir_recv==0)
		printf("The inbound direction for incoming packets at nodeA is successfully set\n");
	if(dir_recv<0)
	{
		printf("Error on setting the inbound direction for pcap_recv handle\n");
		exit(1);
	}
	if (pcap_recv_errbuf[0]!='\0')
	{
		fprintf(stderr,"%s\n",pcap_recv_errbuf);
	}
	if (!pcap_recv)
	{
		exit(1);
	}
	printf("The sniffing pcap loop starts here ... \n");
	while(ending_flag!=1)
	{
		if(ending_flag==1)
			break;
		pcap_loop(pcap_recv , -1 , process_packet_intf0 , NULL);
		if(ending_flag==1)
			break;
	}
}

int main(int argc, char *argv[])
{
	switch(argc)
	{
		// case 2 :
			// if(strcmp(argv[1],"do_recv")==0)
			// {
			// 	mycase=111;	
			// 	printf("The node C is now running for receiving the packets from other nodes\n");
			// }
			// else
			// {
			// 	printf("Error !!! Insufficient or wrong arguments given by the user\n");
			// }
			// break;
		case 3 :
			if(strcmp(argv[1],"do_send")==0)
			{
				if(strcmp(argv[2],"nodeA")==0)
				{
					printf("The node C will now be sending packets to nodeA\n");
					mycase=222;
					nodea_flag=1;
				}
				else
				{
					if(strcmp(argv[2],"nodeB")==0)
					{
						printf("The node C will now be sending packets to nodeB\n");
						mycase=333;
						nodeb_flag=1;
					}
					else
					{
						printf("Error !!! Wrong argument is given for do_send command\n");
					}
				}
			}
			break;
		case 4 :
			if(!strcmp(argv[1],"do_send"))
			{
				if (!strcmp(argv[2],"nodeA") || !strcmp(argv[2],"nodeB"))
				{
					if (!strcmp(argv[3],"nodeB") || !strcmp(argv[3],"nodeA"))
					{
						mycase =  444;
					}
				}
			}
			break;
		// default:
		// 	printf("Error !!! Incorrect number of arguments given\n");
		// 	break;
	}
	pthread_create(&recv_data, NULL, recv_data_func, NULL);
	switch(mycase)
	{
		// case 111 :
		// 	pthread_create(&recv_data, NULL, recv_data_func, NULL);
		// 	pthread_join(recv_data,NULL);
		// 	break;
		case 222 :
			pthread_create(&send_data, NULL, send_data_func, NULL);
			pthread_join(send_data,NULL);
			break;
		case 333 :
			pthread_create(&send_data, NULL, send_data_func, NULL);
			pthread_join(send_data,NULL);
			break;
		case 444 :
			pthread_create(&send_data,  NULL, send_data_to_both_func, NULL);
			pthread_join(send_data,NULL);
			break;
		// default:
		// 	printf("Error !!! Incorrect number of arguments given\n");
		// 	break;
	}
	pthread_join(recv_data,NULL);
	return(0);
}
