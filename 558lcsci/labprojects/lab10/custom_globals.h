#include <pcap.h>
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
#include <sys/time.h>

#define DATA_INSIDE_LENGTH 1368
#define NEW_ETH_FRAME_LEN 1398
#define ETH_P_CUSTOM	0x0822

// #define INP_FILE       "testfile_lab5.txt"
#define INP_FILE       "data10.bin"
// #define INP_FILE_LEN   15124
#define INP_FILE_LEN   10485760
// #define TOT_PKT        12
#define TOT_PKT        7666

extern int seq_no_max;

 
typedef struct header_format_structure
{
	int packet_type;
	int src_addr;
	int dest_addr;
	int seq_no;
}header_format;

typedef struct packet_format_structure
{
    header_format hdr;
	char data_inside[DATA_INSIDE_LENGTH];
}packet_format;

typedef struct ack_format_structure
{
	int packet_type;
	int ack_no;
}ack_format;


