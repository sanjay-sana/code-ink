#include "custom_globals.h"
#define FILENAME "testfile_lab5_copy.txt"
int curr_packet_no=0;
char *file_array=NULL;
int file_flag=0;
int file_length=15124;
int seq_no_max;
int ending_flag=0;
int disp=0;
FILE *fd1;
//int fd1;
int total_pckt_recv=0;
unsigned char *array_write;

void process_packet_intf3(u_char *args, const struct pcap_pkthdr *header,const u_char *old_buffer)
{
	void *buffer=(void*)old_buffer;
	int size = header->len;
	int captured_length=header->caplen;
	packet_format *pckt=(packet_format*)(buffer+14);
				   
				  if(pckt->hdr.packet_type==9999 )
				 {
	                    total_pckt_recv++;
						printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);
						printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);
	                   struct ethhdr *eth_ptr=(struct ethhdr *)(buffer);
					   curr_packet_no=pckt->hdr.seq_no;
					   if(pckt->hdr.seq_no==0)
		               {
		                 file_array=(char*)malloc(file_length*sizeof(char));
		                }
						
					if (file_length - (curr_packet_no * DATA_INSIDE_LENGTH) > DATA_INSIDE_LENGTH)
                   {
                     memcpy( &file_array[curr_packet_no * DATA_INSIDE_LENGTH] ,pckt->data_inside, DATA_INSIDE_LENGTH);
                     }
                    else
		            {
			         memcpy( &file_array[curr_packet_no * DATA_INSIDE_LENGTH] ,pckt->data_inside, file_length - (curr_packet_no * DATA_INSIDE_LENGTH));
		             }
					 
					 printf("Value of total_pckt_recv is %d\n",total_pckt_recv);
				 if(total_pckt_recv==12)
				  {
					//printf("It did enter here\n");
					printf("All the packets received ... writing to file\n");
					fwrite(file_array, 15124, 1,fd1);
		            struct timeval end;
	               gettimeofday(&end, NULL);
                    printf("The file writing is completed at the following timestamp : %f\n", end.tv_sec + (double)(end.tv_usec/1e6));
		            fflush(fd1);
		             fclose(fd1);
					 printf("Writing of file done\n");
					 exit(0);
				  }
				 
      
		//printf("Doing the first loop \n");		
				}
}
    


int recv_data_func()
{
	
	
	fd1=fopen(FILENAME,"w");
	
	char *file_name=(char*)malloc(30*sizeof(char));
	 strcpy(file_name,FILENAME);
	 
	 char pcap_recv_errbuf[PCAP_ERRBUF_SIZE];
	           pcap_recv_errbuf[0]='\0';
		     pcap_t* pcap_recv=pcap_open_live("eth0",NEW_ETH_FRAME_LEN,0,0,pcap_recv_errbuf);
			if (pcap_recv_errbuf[0]!='\0') {
			    fprintf(stderr,"%s\n",pcap_recv_errbuf);
			}
			if (!pcap_recv) {
			    exit(1);
			}
			
	printf("The sniffing pcap loop starts here ... \n");
	while(ending_flag!=1)
	   {
	      if(ending_flag==1)
			  break;
		  pcap_loop(pcap_recv , -1 , process_packet_intf3 , NULL);
		   if(ending_flag==1)
			  break;
		}

 	 return(0);

}
			


int main()
{
	
	recv_data_func();
	

   return(0);
   }