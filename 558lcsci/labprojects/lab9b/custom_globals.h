#include <pcap.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>    
#include <net/if.h>
#include <netinet/in.h>
#include <errno.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>   
#include <netinet/udp.h>   
#include <netinet/tcp.h>   
#include <netinet/ip.h>  
#include <sys/stat.h> 
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <fcntl.h>
#include <pcap.h>
#include <pthread.h>

#define DATA_LENGTH 1492
#define FRAME_LEN 1512
#define ACK_DATA_LEN 5
#define ACK_FRAME_LEN 29

// #define FILE_NAME "testfile_lab5.txt"
// #define FILENAME "output.txt"

#define FILE_NAME "data2"
#define FILENAME "output2"
       
typedef struct header_format_structure
{
	int packet_type;
	int src_addr;
	int dest_addr;
	int seq_no;
	// int file_size;
}header_format;

typedef struct packet_format_structure
{
    header_format hdr;
	char data_inside[DATA_LENGTH];
}packet_format;


typedef struct ack_format_structure
{
	header_format hdr_ack;
	char data_in_ack[ACK_DATA_LEN];
}ack_format;

