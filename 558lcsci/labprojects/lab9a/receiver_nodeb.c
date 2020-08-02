#include "custom_globals.h"
// #define FILENAME "testfile_lab5_dup.txt"

int recv_data_func()
{

	printf("The sniffing loop starts here ... \n");
	int saddr_size , data_size;
	struct sockaddr saddr;
	int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if (sock_raw == -1) { 
      printf("socket not created properly\n");
    }
    setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth3" , strlen("eth3")+ 1 );
    unsigned char *buf = (unsigned char *) malloc(FRAME_LEN);
    while(1)
	{
		saddr_size = sizeof saddr;
		data_size= recvfrom(sock_raw, buf, FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
		
		packet_format *pckt2=(packet_format*)(buf);
		if(pckt2->hdr.packet_type==9999){
			printf("pckt value of packet_type is %d\n",pckt2->hdr.packet_type);
		    printf("pckt value of src address is %d\n",pckt2->hdr.src_addr);
		    printf("pckt value of dst address is %d\n",pckt2->hdr.dest_addr);
		    printf("pckt value of sequence number is %d\n",pckt2->hdr.seq_no);
		    // struct ethhdr *eth_ptr=(struct ethhdr *)(buf);
		    // printf("Packet src mac address in HEX inside our packet before change is : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n", eth_ptr->h_source[0] , eth_ptr->h_source[1] , eth_ptr->h_source[2] , eth_ptr->h_source[3] , eth_ptr->h_source[4] , eth_ptr->h_source[5]);
		    // printf("Packet destination mac address in HEX inside our packet before change is : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n", eth_ptr->h_dest[0] , eth_ptr->h_dest[1] , eth_ptr->h_dest[2] , eth_ptr->h_dest[3] , eth_ptr->h_dest[4] , eth_ptr->h_dest[5]);
		    printf("The data value inside the packet value is --------------------->>>>\n\n%s\n",pckt2->data_inside);
		    printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		    printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
	        printf("||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
		}
			

	}	

    close(sock_raw);
}    

int main()
{
	
	recv_data_func();
	return 0;
}