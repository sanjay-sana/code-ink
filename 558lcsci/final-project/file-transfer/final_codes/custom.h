#include <pcap.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include <stdbool.h>

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

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#define SIMUL_FLOWS    	4
#define MAX_HOP         2

//G and N for diffie hellman
#define GENERATOR       7 
#define N               17

#define DH_REQUEST    0
#define DH_REPLY      1

#define LS_REQUEST    0
#define LS_REPLY      1

#define DH_PACKET_TYPE_N1	1
#define DH_PACKET_TYPE_N2	2
#define DH_PACKET_TYPE_N3	3
#define DH_PACKET_TYPE_N4	4
#define DH_PACKET_TYPE_N5	5
#define DH_PACKET_TYPE_N6	6

#define DH_PACKET_TYPE_R1A	7
#define DH_PACKET_TYPE_R1B	8
#define DH_PACKET_TYPE_R1C	9
#define DH_PACKET_TYPE_R1D	10
#define DH_PACKET_TYPE_R2A	12
#define DH_PACKET_TYPE_R2C	11
#define DH_PACKET_TYPE_R2B	13
#define DH_PACKET_TYPE_R2D	14
#define DH_PACKET_TYPE_R3A	15
#define DH_PACKET_TYPE_R3B	16
#define DH_PACKET_TYPE_R3C	17
#define DH_PACKET_TYPE_R3D	18

#define FT_PACKET_TYPE_N1	21
#define FT_PACKET_TYPE_N2	22
#define FT_PACKET_TYPE_N3	23
#define FT_PACKET_TYPE_N4	24
#define FT_PACKET_TYPE_N5	25
#define FT_PACKET_TYPE_N6	26

#define FT_PACKET_TYPE_R1A	27
#define FT_PACKET_TYPE_R1B	28
#define FT_PACKET_TYPE_R1C	29
#define FT_PACKET_TYPE_R1D	30
#define FT_PACKET_TYPE_R2A	31
#define FT_PACKET_TYPE_R2B	32
#define FT_PACKET_TYPE_R2C	33
#define FT_PACKET_TYPE_R2D	34
#define FT_PACKET_TYPE_R3A	35
#define FT_PACKET_TYPE_R3B	36
#define FT_PACKET_TYPE_R3C	37
#define FT_PACKET_TYPE_R3D	38

#define ACK_PACKET_TYPE_N1	41
#define ACK_PACKET_TYPE_N2	42
#define ACK_PACKET_TYPE_N3	43
#define ACK_PACKET_TYPE_N4	44
#define ACK_PACKET_TYPE_N5	45
#define ACK_PACKET_TYPE_N6	46

#define ACK_PACKET_TYPE_R1A	47
#define ACK_PACKET_TYPE_R1B	48
#define ACK_PACKET_TYPE_R1C	49
#define ACK_PACKET_TYPE_R1D	50
#define ACK_PACKET_TYPE_R2A	51
#define ACK_PACKET_TYPE_R2B	52
#define ACK_PACKET_TYPE_R2C	53
#define ACK_PACKET_TYPE_R2D	54
#define ACK_PACKET_TYPE_R3A	55
#define ACK_PACKET_TYPE_R3B	56
#define ACK_PACKET_TYPE_R3C	57
#define ACK_PACKET_TYPE_R3D	58

#define BROADCAST_PACKET_TYPE  999

#define LINKSTATE_PACKET_TYPE  998

#define N1 1
#define N2 2
#define N3 3
#define N4 4
#define N5 5
#define N6 6

#define R1 7
#define R2 8
#define R3 9

#define N1_FLAG 91
#define N2_FLAG 92
#define N3_FLAG 93
#define N4_FLAG 94
#define N5_FLAG 95
#define N6_FLAG 96

#define R1_FLAG 97
#define R2_FLAG 98
#define R3_FLAG 99

// DH Instructions
#define REPLY_KEY         1
#define KEY_ACK           2
#define SAVE_FLAG         3
#define DECRYPT_FORWARD   4
#define FORWARED_ACK      5
#define STORE_DATA        6
/*TODO 11/27 */
#define N2_R1_FT_PACKET_TYPE 90
#define R1_N2_FT_PACKET_TYPE 89
#define N3_R1_FT_PACKET_TYPE 88
#define R1_N3_FT_PACKET_TYPE 87

#define ACK_R1_IN 86
#define ACK_R1_OUT 85
#define ACK_PACKET 84

/*TODO 11/27 */

/*
#define ACK_PACKET_N2 90
#define ACK_PACKET_N3 91
*/


#define AES_BLOCK_SIZE    16
#define INPUT_FILE      "testfile.txt"
#define OUTPUT_FILE     "output.txt"



typedef struct header_format_structure
{
	short int packet_type;
	short int src_addr;
	short int dest_addr;
	short int seq_no;	
}header_format;

// typedef struct packet_format_structure
// {
//     header_format hdr;
// 	char data_inside[DATA_LENGTH];
// }packet_format;

typedef struct multiple_dh_packet_packet_format_structure
{
	short int data;
	short int dest;
}multiple_dh_packet;

typedef struct diffie_hellman_packet_format_structure
{
	header_format hdr;
	multiple_dh_packet body;
	int instruction;
	short int flag;
}dh_packet;

struct hops
{
	int first_hop;
	int second_hop;
	int num_hops;
};