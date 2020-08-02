//#include "my402list.h"
//#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <pcap.h>
#include <pthread.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <endian.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>

short int node0 = 1;
short int node1 = 2;
short int node2 = 3;


#define data_len 1440
#define frame_len 1488
#define aes_block_size 16
#define HDR_SIZE 16

#define handle_error(msg) do { perror(msg); exit(0); } while (0)

// int FILESIZE=12;
// int FILESIZE = 1048576;
int file_size;

typedef struct header_format_structure1
{
	short int packet_type;
	short int src_addr;
	short int dest_addr;
	short int padding2;
	int seq_no;
	int padding1;

}header_format1;

typedef struct packet_format_structure
{
	
    header_format1 hdr;
	char data_inside[data_len];
	
}packet_format;
