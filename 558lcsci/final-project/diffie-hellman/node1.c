#include "custom.h"

#define NODE_ID    1
#define NODEID_MAC0 0x00
#define NODEID_MAC1 0x04
#define NODEID_MAC2 0x23
#define NODEID_MAC3 0xBB
#define NODEID_MAC4 0x25
#define NODEID_MAC5 0xAA

#define ETHERNET_PORT_NODE "eth4"

#define FILENAME "key_log_node1.txt"

int currentNode = N1;
int currentNode_packetType = DH_PACKET_TYPE_N1;

FILE *fp;
char *file_write;

struct threadArguments
{
	int dest_node;
};
struct threadArguments *arg;

struct sharedKey
{
	int key;
	int flag;
};
struct sharedKey *shared_key[10];
int shared_key_counter=0;

int skipArray[10] = {0};
int threadCount = -1;

struct hops sourceRoute[6][6];

pthread_t sender_thread[20], receiver_thread;
pthread_mutex_t sharedKey_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t skipDH_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void setupSourceRoute()
{
	sourceRoute[N1][N2].num_hops=2;
	sourceRoute[N1][N3].num_hops=3;
	sourceRoute[N1][N4].num_hops=3;
	sourceRoute[N1][N5].num_hops=3;
	sourceRoute[N1][N6].num_hops=3;

	sourceRoute[N2][N1].num_hops=2;
	sourceRoute[N2][N3].num_hops=3;
	sourceRoute[N2][N4].num_hops=3;
	sourceRoute[N2][N5].num_hops=3;
	sourceRoute[N2][N6].num_hops=3;

	sourceRoute[N3][N1].num_hops=3;
	sourceRoute[N3][N2].num_hops=3;
	sourceRoute[N3][N4].num_hops=2;
	sourceRoute[N3][N5].num_hops=3;
	sourceRoute[N3][N6].num_hops=3;

	sourceRoute[N4][N1].num_hops=3;
	sourceRoute[N4][N2].num_hops=3;
	sourceRoute[N4][N3].num_hops=2;
	sourceRoute[N4][N5].num_hops=3;
	sourceRoute[N4][N6].num_hops=3;

	sourceRoute[N5][N1].num_hops=3;
	sourceRoute[N5][N2].num_hops=3;
	sourceRoute[N5][N3].num_hops=3;
	sourceRoute[N5][N4].num_hops=3;
	sourceRoute[N5][N6].num_hops=2;

	sourceRoute[N6][N1].num_hops=3;
	sourceRoute[N6][N2].num_hops=3;
	sourceRoute[N6][N3].num_hops=3;
	sourceRoute[N6][N4].num_hops=3;
	sourceRoute[N6][N5].num_hops=2;

/***************************************************/
/***************************************************/

	sourceRoute[N1][N2].first_hop=R1;
	sourceRoute[N1][N3].first_hop=R1;
	sourceRoute[N1][N4].first_hop=R1;
	sourceRoute[N1][N5].first_hop=R1;
	sourceRoute[N1][N6].first_hop=R1;

	sourceRoute[N2][N1].first_hop=R1;
	sourceRoute[N2][N3].first_hop=R1;
	sourceRoute[N2][N4].first_hop=R1;
	sourceRoute[N2][N5].first_hop=R1;
	sourceRoute[N2][N6].first_hop=R1;

	sourceRoute[N3][N1].first_hop=R2;
	sourceRoute[N3][N2].first_hop=R2;
	sourceRoute[N3][N4].first_hop=R2;
	sourceRoute[N3][N5].first_hop=R2;
	sourceRoute[N3][N6].first_hop=R2;

	sourceRoute[N4][N1].first_hop=R2;
	sourceRoute[N4][N2].first_hop=R2;
	sourceRoute[N4][N3].first_hop=R2;
	sourceRoute[N4][N5].first_hop=R2;
	sourceRoute[N4][N6].first_hop=R2;

	sourceRoute[N5][N1].first_hop=R3;
	sourceRoute[N5][N2].first_hop=R3;
	sourceRoute[N5][N3].first_hop=R3;
	sourceRoute[N5][N4].first_hop=R3;
	sourceRoute[N5][N6].first_hop=R3;

	sourceRoute[N6][N1].first_hop=R3;
	sourceRoute[N6][N2].first_hop=R3;
	sourceRoute[N6][N3].first_hop=R3;
	sourceRoute[N6][N4].first_hop=R3;
	sourceRoute[N6][N5].first_hop=R3;

/***************************************************/
/***************************************************/

	//sourceRoute[N1][N2].second_hop=1;
	sourceRoute[N1][N3].second_hop=R2;
	sourceRoute[N1][N4].second_hop=R2;
	sourceRoute[N1][N5].second_hop=R3;
	sourceRoute[N1][N6].second_hop=R3;

	//sourceRoute[N2][N1].second_hop=1;
	sourceRoute[N2][N3].second_hop=R2;
	sourceRoute[N2][N4].second_hop=R2;
	sourceRoute[N2][N5].second_hop=R3;
	sourceRoute[N2][N6].second_hop=R3;

	sourceRoute[N3][N1].second_hop=R1;
	sourceRoute[N3][N2].second_hop=R1;
	//sourceRoute[N3][N4].second_hop=1;
	sourceRoute[N3][N5].second_hop=R3;
	sourceRoute[N3][N6].second_hop=R3;

	sourceRoute[N4][N1].second_hop=R1;
	sourceRoute[N4][N2].second_hop=R1;
	//sourceRoute[N4][N3].second_hop=1;
	sourceRoute[N4][N5].second_hop=R3;
	sourceRoute[N4][N6].second_hop=R3;

	sourceRoute[N5][N1].second_hop=R1;
	sourceRoute[N5][N2].second_hop=R1;
	sourceRoute[N5][N3].second_hop=R2;
	sourceRoute[N5][N4].second_hop=R2;
	//sourceRoute[N5][N6].second_hop=1;

	sourceRoute[N6][N1].second_hop=R1;
	sourceRoute[N6][N2].second_hop=R1;
	sourceRoute[N6][N3].second_hop=R2;
	sourceRoute[N6][N4].second_hop=R2;
	//sourceRoute[N6][N5].second_hop=1;
}

int secretInteger(int r_gen)
{
	int n;
	do
	{
		n = rand() % r_gen;
		if(n>r_gen)
		{
			n = n%r_gen;
		}
		// printf("rand loop %d\n", n);
	}while(n <= 1);
	return n;
}

int createSocket(char* ethIntr)
{
	int sock;
	sock = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL));
	if (sock == -1)
	{ 
		printf("socket not created properly\n");
		exit (1);
	}
	setsockopt(sock, SOL_SOCKET , SO_BINDTODEVICE , ethIntr, strlen(ethIntr));
	return sock;
}

void modifyPacket_part1(dh_packet *pckt, struct threadArguments *threadArg)
{
	(pckt->hdr).packet_type = currentNode_packetType;
	(pckt->hdr).src_addr = currentNode;
	(pckt->hdr).dest_addr = sourceRoute[currentNode][threadArg->dest_node].first_hop;
}

void modifyDH_Packet(dh_packet *pckt, short int type, short int src, short int destin, short int seqn, short int body_data, short int body_dest, short int instr, short int pkt_flag)
{
	(pckt->hdr).packet_type = type;
	(pckt->hdr).src_addr    = src;
	(pckt->hdr).dest_addr   = destin;
	(pckt->hdr).seq_no      = seqn;
	(pckt->body).data       = body_data;
	(pckt->body).dest       = body_dest;
	pckt->instruction       = instr;
	pckt->flag              = pkt_flag;
}

void print_dhPacket(dh_packet *pckt)
{
	printf("pckt value of packet_type is %d\n", pckt->hdr.packet_type);
	printf("pckt value of src address is %d\n", pckt->hdr.src_addr);
	printf("pckt value of dst address is %d\n", pckt->hdr.dest_addr);
	printf("pckt value of sequence number is %d\n", pckt->hdr.seq_no);
	printf("pckt value of key is %d\n", pckt->body.data);
	printf("pckt value of dest is %d\n", pckt->body.dest);
	printf("pckt value of instruction is %d\n", pckt->instruction);
	printf("pckt value of flag is %d\n", pckt->flag);
}

unsigned char *keyCalculate(int key)
{
    unsigned char *aes_key = (char*)malloc(16*sizeof(unsigned char));
    aes_key[0]=(unsigned char)key;
    aes_key[1]='0'; 
    aes_key[2]='0';
    aes_key[3]='0';
    aes_key[4]='0'; 
    aes_key[5]='0';
    aes_key[6]='0';
    aes_key[7]='0'; 
    aes_key[8]='0';
    aes_key[9]='0';
    aes_key[10]='0'; 
    aes_key[11]='0';
    aes_key[12]='0';
    aes_key[13]='0'; 
    aes_key[14]='0';
    aes_key[15]='0';
    return aes_key;
}

unsigned char *dec_out;
unsigned char* AESencrypt(unsigned char* aes_input, unsigned char aes_key[],int size)
{
    unsigned char iv_enc[AES_BLOCK_SIZE];
    int keylength=128;
    
    AES_KEY enc_key;
    memset(iv_enc,0x00,AES_BLOCK_SIZE);

    const size_t encslength = ((strlen(aes_input) + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char *enc_out = (unsigned char *)malloc(FRAME_LEN * sizeof(unsigned char));
    memset(enc_out, 0, sizeof(enc_out));


    AES_set_encrypt_key(aes_key, keylength, &enc_key);
    AES_cbc_encrypt(aes_input,enc_out, size, &enc_key, iv_enc, AES_ENCRYPT);
  
    return enc_out;
}

void attach_header(short int packet_type_arg, short int src_addr_arg, short int dest_addr_arg, int seq_no_arg, short int body_data_arg, short int body_dest_arg, short int instr_arg, unsigned char* packet_ptr)
{
	memcpy(packet_ptr, (unsigned char *)&packet_type_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+2, (unsigned char *)&src_addr_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+4, (unsigned char *)&dest_addr_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+6, (unsigned char *)&seq_no_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+8, (unsigned char *)&body_data_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+10, (unsigned char *)&body_dest_arg, 2 * sizeof(unsigned char));
	memcpy(packet_ptr+12, (unsigned char *)&instr_arg, 4 * sizeof(unsigned char));
}

unsigned char* encrypt_data_func(unsigned char *aes_key, int getKey, unsigned char *buf_tobe_encrypted)
{
	aes_key = keyCalculate(getKey);
	return AESencrypt(buf_tobe_encrypted, aes_key, 16);
}

// void *SendDH_Keys(void *arg)
void SendDH_Keys(void *arg)
{
	struct threadArguments *threadArg = (struct threadArguments*)arg;
	printf("Dest = %d\n", threadArg->dest_node);
	int sockfd;
	struct sockaddr_ll device = {0};
	int saddr_size, data_size;
	struct sockaddr saddr;
	sockfd = createSocket(ETHERNET_PORT_NODE);

	if ((device.sll_ifindex = if_nametoindex (ETHERNET_PORT_NODE)) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit (EXIT_FAILURE);
	}
	device.sll_family   = AF_PACKET;
	device.sll_addr[0]  = NODEID_MAC0;
	device.sll_addr[1]  = NODEID_MAC1;
	device.sll_addr[2]  = NODEID_MAC2;
	device.sll_addr[3]  = NODEID_MAC3;
	device.sll_addr[4]  = NODEID_MAC4;
	device.sll_addr[5]  = NODEID_MAC5;
	device.sll_halen    = ETH_ALEN;
	device.sll_protocol = htons(0x1234);

	unsigned char* buf = (unsigned char*)malloc(FRAME_LEN);
	unsigned char *buffer = (unsigned char *)malloc(FRAME_LEN);

	memset(buf, 0, FRAME_LEN);
	
	dh_packet *packet_send=(dh_packet*)(buf);
	dh_packet *packet_send_h2 = (dh_packet*)(buf + sizeof(dh_packet));
	dh_packet *packet_send_h3 = (dh_packet*)(buf + (2 * sizeof(dh_packet)));
	
	memset(buf + sizeof(dh_packet), 0, FRAME_LEN - sizeof(dh_packet));
	memset(buf + (2 * sizeof(dh_packet)), 0, FRAME_LEN - (2 * sizeof(dh_packet)));

	int skip_send = 0;
	//
	int a;
	int i=0;
	dh_packet *pckt=(dh_packet*)(buffer);
	dh_packet *pckt_h2 = (dh_packet*)(buffer + sizeof(dh_packet));
	dh_packet *pckt_h3 = (dh_packet*)(buffer + (2 * sizeof(dh_packet)));

	modifyPacket_part1(packet_send, threadArg);
	/*==Diffie hellman start ==*/
	a=secretInteger(N);
	(packet_send->body).data = (int)((GENERATOR)^a)%N;
	(packet_send->hdr).seq_no = (packet_send->body).data + threadArg->dest_node;
	/*==Diffie hellman end==*/

	// Setting up variables for encryption
	unsigned char *aes_key = (unsigned char *)malloc(AES_BLOCK_SIZE * sizeof(unsigned char));
	// unsigned char *output_hdr1 = (unsigned char *)malloc(16 * sizeof(unsigned char));
	unsigned char *buf_tobe_encrypted = (unsigned char *)malloc(FRAME_LEN);
	unsigned char *buf_encrypted = (unsigned char *)malloc(FRAME_LEN);
	// memset(output_hdr1, 0, 16);
	memset(buf_encrypted, 0, FRAME_LEN);
	short int hold_key;
	// for (i = 1; i <= 1; ++i)
	for (i = 1; i <= sourceRoute[currentNode][threadArg->dest_node].num_hops; ++i)
	{
		hold_key = 0;
		skip_send = 0;
		switch(i)
		{
			case 1:
				printf("First hop...\n");
				pthread_mutex_lock(&skipDH_mutex);
				if (skipArray[sourceRoute[currentNode][threadArg->dest_node].first_hop] == 0)
				{
					skipArray[sourceRoute[currentNode][threadArg->dest_node].first_hop] = 1;
					packet_send->instruction = REPLY_KEY;
				}
				else
				{
					skip_send = 1;
				}
				pthread_mutex_unlock(&skipDH_mutex);
				break;
			case 2:
				printf("Second hop...\n");
				if (i == sourceRoute[currentNode][threadArg->dest_node].num_hops)
				{
					pthread_mutex_lock(&skipDH_mutex);
					if (skipArray[threadArg->dest_node] == 0)
					{
						skipArray[threadArg->dest_node] = 1;
						packet_send->instruction = DECRYPT_FORWARD;
						packet_send->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;
						packet_send->body.dest = sourceRoute[currentNode][threadArg->dest_node].second_hop;
						modifyDH_Packet(packet_send_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, threadArg->dest_node, (packet_send->hdr).seq_no, (int)((GENERATOR)^a)%N, 0, REPLY_KEY, 0);
						hold_key = packet_send->body.data;

						printf("First part...\n");
						print_dhPacket(packet_send);
						printf("Before encryption, second part...\n");
						print_dhPacket(packet_send_h2);
						memcpy(buf_tobe_encrypted, (unsigned char *)packet_send_h2, 16);
						buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
						// attach_header((packet_send_h2->hdr).packet_type, (packet_send_h2->hdr).src_addr, (packet_send_h2->hdr).dest_addr, (packet_send_h2->hdr).seq_no, packet_send_h2->body.data, packet_send_h2->body.dest, packet_send_h2->instruction, buf_tobe_encrypted);
						memcpy(buf_encrypted + 16, (unsigned char *)&packet_send_h2->flag, 2 * sizeof(char));
						memset(buf + sizeof(dh_packet), 0, 18);
						memcpy(buf + sizeof(dh_packet), buf_encrypted, 18);
						printf("After encryption, second part...\n");
						dh_packet *enc_pckt = (dh_packet*)(buf + sizeof(dh_packet));
						print_dhPacket(enc_pckt);
					}
					else
					{
						skip_send = 1;
					}
					pthread_mutex_unlock(&skipDH_mutex);
				}
				else
				{
					pthread_mutex_lock(&skipDH_mutex);
					if (skipArray[sourceRoute[currentNode][threadArg->dest_node].second_hop] == 0)
					{
						skipArray[sourceRoute[currentNode][threadArg->dest_node].second_hop] = 1;
						packet_send->instruction = DECRYPT_FORWARD;
						packet_send->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;
						packet_send->body.dest = 0;

						a = secretInteger(N);
						modifyDH_Packet(packet_send_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, sourceRoute[currentNode][threadArg->dest_node].second_hop, (packet_send->hdr).seq_no, (int)((GENERATOR)^a)%N, sourceRoute[currentNode][threadArg->dest_node].second_hop, REPLY_KEY, 0);
						hold_key = packet_send_h2->body.data;

						printf("First part...\n");
						print_dhPacket(packet_send);
						printf("Before encryption, second part...\n");
						print_dhPacket(packet_send_h2);
						memcpy(buf_tobe_encrypted, (unsigned char *)packet_send_h2, 16);
						buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
						// attach_header((packet_send_h2->hdr).packet_type, (packet_send_h2->hdr).src_addr, (packet_send_h2->hdr).dest_addr, (packet_send_h2->hdr).seq_no, packet_send_h2->body.data, packet_send_h2->body.dest, packet_send_h2->instruction, buf_tobe_encrypted);
						memcpy(buf_encrypted + 16, (unsigned char *)&packet_send_h2->flag, 2 * sizeof(char));
						memset(buf + sizeof(dh_packet), 0, 18);
						memcpy(buf + sizeof(dh_packet), buf_encrypted, 18);
						printf("After encryption, second part...\n");
						dh_packet *enc_pckt1 = (dh_packet*)(buf + sizeof(dh_packet));
						print_dhPacket(enc_pckt1);
					}
					else
					{
						skip_send = 1;
					}
					pthread_mutex_unlock(&skipDH_mutex);
				}
				break;
			case 3:
				printf("Third hop...\n");
				pthread_mutex_lock(&skipDH_mutex);
				if (skipArray[threadArg->dest_node] == 0)
				{
					skipArray[threadArg->dest_node] = 1;
					packet_send->instruction = DECRYPT_FORWARD;
					packet_send->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;
					packet_send->body.dest = sourceRoute[currentNode][threadArg->dest_node].second_hop;
					
					modifyDH_Packet(packet_send_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, sourceRoute[currentNode][threadArg->dest_node].second_hop, (packet_send->hdr).seq_no, 0, threadArg->dest_node, DECRYPT_FORWARD, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->flag);

					a = secretInteger(N);
					modifyDH_Packet(packet_send_h3, 0, sourceRoute[currentNode][threadArg->dest_node].second_hop, threadArg->dest_node, (packet_send->hdr).seq_no, (int)((GENERATOR)^a)%N, 0, REPLY_KEY, 0);
					hold_key = packet_send_h3->body.data;

					printf("First part...\n");
					print_dhPacket(packet_send);
					printf("Before encryption, second part...\n");
					print_dhPacket(packet_send_h2);
					memcpy(buf_tobe_encrypted, (unsigned char *)packet_send_h2, 16);
					buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
					// attach_header((packet_send_h2->hdr).packet_type, (packet_send_h2->hdr).src_addr, (packet_send_h2->hdr).dest_addr, (packet_send_h2->hdr).seq_no, packet_send_h2->body.data, packet_send_h2->body.dest, packet_send_h2->instruction, buf_tobe_encrypted);
					memcpy(buf_encrypted + 16, (unsigned char *)&packet_send_h2->flag, 2 * sizeof(char));
					memset(buf + sizeof(dh_packet), 0, 18);
					memcpy(buf + sizeof(dh_packet), buf_encrypted, 18);
					printf("After encryption, second part...\n");
					dh_packet *enc_pckt2 = (dh_packet*)(buf + sizeof(dh_packet));
					print_dhPacket(enc_pckt2);

					printf("Before encryption, third part...\n");
					print_dhPacket(packet_send_h3);
					memcpy(buf_tobe_encrypted, (unsigned char *)packet_send_h3, 16);
					buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key, buf_tobe_encrypted);
					// attach_header((packet_send_h3->hdr).packet_type, (packet_send_h3->hdr).src_addr, (packet_send_h3->hdr).dest_addr, (packet_send_h3->hdr).seq_no, packet_send_h3->body.data, packet_send_h3->body.dest, packet_send_h3->instruction, buf_tobe_encrypted);
					memcpy(buf_encrypted + 16, (unsigned char *)&packet_send_h3->flag, 2 * sizeof(char));
					memset(buf + sizeof(dh_packet) + sizeof(dh_packet), 0, 18);
					memcpy(buf + sizeof(dh_packet) + sizeof(dh_packet), buf_encrypted, 18);
					printf("After encryption, third part...\n");
					dh_packet *enc_pckt3 = (dh_packet*)(buf + sizeof(dh_packet) + sizeof(dh_packet));
					print_dhPacket(enc_pckt3);
				}
				else
				{
					skip_send = 1;
				}
				pthread_mutex_unlock(&skipDH_mutex);
				break;
		}
	//
		if (skip_send == 0)
		{
			printf("Going from node1 to router\n");
			switch(i)
			{
				case 1:
					print_dhPacket(packet_send);
					break;
				case 2:
					print_dhPacket(packet_send);
					printf("Second part of the packet...\n");
					print_dhPacket(packet_send_h2);
					break;
				case 3:
					print_dhPacket(packet_send);
					printf("Second part of the packet...\n");
					print_dhPacket(packet_send_h2);
					printf("Third part of the packet...\n");
					print_dhPacket(packet_send_h3);
					break;
			}
			int n = sendto(sockfd, buf, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
			if (n == -1)
			{
				printf("Send error\n");
			}

			memset (buffer, 0, sizeof (FRAME_LEN));
			printf("Packet sent to %d\n", (packet_send->hdr).dest_addr);
			while(1)
			{
				data_size = recvfrom(sockfd, buffer, FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
				if(pckt->hdr.packet_type == DH_PACKET_TYPE_R1A 
					&& pckt->hdr.dest_addr == currentNode
					&& pckt->instruction == KEY_ACK
					&& pckt->hdr.seq_no == ((packet_send->hdr).seq_no + 1)
					&& pckt->hdr.src_addr == (packet_send->hdr).dest_addr)
				{
					printf("Receiving from router to node1\n");
					print_dhPacket(pckt);
					switch(i)
					{
						case 1:
							pthread_mutex_lock(&sharedKey_mutex);
							if (shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key == 0)
							{
								shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key = (pckt->body).data * (packet_send->body).data;
								printf("The shared key with node%d is %d\n",sourceRoute[currentNode][threadArg->dest_node].first_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key);
							}
							pthread_mutex_unlock(&sharedKey_mutex);
							break;
						case 2:
							if (i == sourceRoute[currentNode][threadArg->dest_node].num_hops)
							{
								pthread_mutex_lock(&sharedKey_mutex);
								if (shared_key[threadArg->dest_node]->key == 0)
								{
									// shared_key[threadArg->dest_node]->key = (pckt->body).data * (packet_send_h2->body).data;
									printf("hold_key = %d\n", hold_key);
									shared_key[threadArg->dest_node]->key = (pckt->body).data * hold_key;
									printf("The shared key with node%d is %d\n", threadArg->dest_node, shared_key[threadArg->dest_node]->key);
								}
								pthread_mutex_unlock(&sharedKey_mutex);
							}
							else
							{
								pthread_mutex_lock(&sharedKey_mutex);
								if (shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key == 0)
								{
									printf("hold_key = %d\n", hold_key);
									// shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key = (pckt->body).data * (packet_send_h2->body).data;
									shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key = (pckt->body).data * hold_key;
									printf("The shared key with node%d is %d\n", sourceRoute[currentNode][threadArg->dest_node].second_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key);
								}
								pthread_mutex_unlock(&sharedKey_mutex);
							}
							break;
						case 3:
							pthread_mutex_lock(&sharedKey_mutex);
							if (shared_key[threadArg->dest_node]->key == 0)
							{
								printf("hold_key = %d\n", hold_key);
								// shared_key[threadArg->dest_node]->key = (pckt->body).data * (packet_send_h3->body).data;
								shared_key[threadArg->dest_node]->key = (pckt->body).data * hold_key;
								printf("The shared key with node%d is %d\n", threadArg->dest_node, shared_key[threadArg->dest_node]->key);
							}
							pthread_mutex_unlock(&sharedKey_mutex);
							break;
					}

					modifyPacket_part1(pckt, threadArg);
					pckt->hdr.seq_no = (packet_send->hdr).seq_no;
					switch(i)
					{
						case 1:
							pckt->body.data = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key;
							pckt->instruction = SAVE_FLAG;
							pckt->flag = secretInteger(N);
							shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag = pckt->flag;
							sprintf(file_write, "%d#%d#%d\n", sourceRoute[currentNode][threadArg->dest_node].first_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag);
							break;
						case 2:
							if (i == sourceRoute[currentNode][threadArg->dest_node].num_hops)
							{
								pckt->instruction = DECRYPT_FORWARD;
								pckt->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;
								modifyDH_Packet(pckt_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, threadArg->dest_node, (packet_send->hdr).seq_no, shared_key[threadArg->dest_node]->key, 0, SAVE_FLAG, secretInteger(N));
								shared_key[threadArg->dest_node]->flag = pckt_h2->flag;
								sprintf(file_write, "%d#%d#%d\n", threadArg->dest_node, shared_key[threadArg->dest_node]->key, shared_key[threadArg->dest_node]->flag);
								
								printf("First part...\n");
								print_dhPacket(pckt);
								printf("Before encryption, second part...\n");
								print_dhPacket(pckt_h2);
								memcpy(buf_tobe_encrypted, (unsigned char *)pckt_h2, 16);
								buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
								// attach_header((pckt_h2->hdr).packet_type, (pckt_h2->hdr).src_addr, (pckt_h2->hdr).dest_addr, (pckt_h2->hdr).seq_no, pckt_h2->body.data, pckt_h2->body.dest, pckt_h2->instruction, buf_tobe_encrypted);
								memcpy(buf_encrypted + 16, (unsigned char *)&pckt_h2->flag, 2 * sizeof(char));
								memset(buffer + sizeof(dh_packet), 0, 18);
								memcpy(buffer + sizeof(dh_packet), buf_encrypted, 18);

								printf("After encryption, second part...\n");
								dh_packet *enc_pckt_flag = (dh_packet*)(buffer + sizeof(dh_packet));
								print_dhPacket(enc_pckt_flag);
							}
							else
							{
								pckt->instruction = DECRYPT_FORWARD;
								pckt->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;

								modifyDH_Packet(pckt_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, sourceRoute[currentNode][threadArg->dest_node].second_hop, pckt->hdr.seq_no, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key, sourceRoute[currentNode][threadArg->dest_node].second_hop, SAVE_FLAG, secretInteger(N));

								shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->flag = pckt_h2->flag;
								sprintf(file_write, "%d#%d#%d\n", sourceRoute[currentNode][threadArg->dest_node].second_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->flag);
								
								printf("First part...\n");
								print_dhPacket(pckt);
								printf("Before encryption, second part...\n");
								print_dhPacket(pckt_h2);
								memcpy(buf_tobe_encrypted, (unsigned char *)pckt_h2, 16);
								buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
								// attach_header((pckt_h2->hdr).packet_type, (pckt_h2->hdr).src_addr, (pckt_h2->hdr).dest_addr, (pckt_h2->hdr).seq_no, pckt_h2->body.data, pckt_h2->body.dest, pckt_h2->instruction, buf_tobe_encrypted);
								memcpy(buf_encrypted + 16, (unsigned char *)&pckt_h2->flag, 2 * sizeof(char));
								memset(buffer + sizeof(dh_packet), 0, 18);
								memcpy(buffer + sizeof(dh_packet), buf_encrypted, 18);

								printf("After encryption, second part...\n");
								dh_packet *enc_pckt_flag1 = (dh_packet*)(buffer + sizeof(dh_packet));
								print_dhPacket(enc_pckt_flag1);

							}
							break;
						case 3:
							pckt->instruction = DECRYPT_FORWARD;
							pckt->flag = shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag;

							modifyDH_Packet(pckt_h2, 0, sourceRoute[currentNode][threadArg->dest_node].first_hop, sourceRoute[currentNode][threadArg->dest_node].second_hop, (packet_send->hdr).seq_no, 0, threadArg->dest_node, DECRYPT_FORWARD, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->flag);

							modifyDH_Packet(pckt_h3, 0, sourceRoute[currentNode][threadArg->dest_node].second_hop, threadArg->dest_node, (packet_send->hdr).seq_no, shared_key[threadArg->dest_node]->key, 0, SAVE_FLAG, secretInteger(N));

							shared_key[threadArg->dest_node]->flag = pckt_h3->flag;
							sprintf(file_write, "%d#%d#%d\n", threadArg->dest_node, shared_key[threadArg->dest_node]->key, shared_key[threadArg->dest_node]->flag);
							
							printf("First part...\n");
							print_dhPacket(pckt);
							printf("Before encryption, second part...\n");
							print_dhPacket(pckt_h2);
							memcpy(buf_tobe_encrypted, (unsigned char *)pckt_h2, 16);
							buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->key, buf_tobe_encrypted);
							// attach_header((pckt_h2->hdr).packet_type, (pckt_h2->hdr).src_addr, (pckt_h2->hdr).dest_addr, (pckt_h2->hdr).seq_no, pckt_h2->body.data, pckt_h2->body.dest, pckt_h2->instruction, buf_tobe_encrypted);
							memcpy(buf_encrypted + 16, (unsigned char *)&pckt_h2->flag, 2 * sizeof(char));
							memset(buffer + sizeof(dh_packet), 0, 18);
							memcpy(buffer + sizeof(dh_packet), buf_encrypted, 18);
							printf("After encryption, second part...\n");
							dh_packet *enc_pckt2_flag = (dh_packet*)(buffer + sizeof(dh_packet));
							print_dhPacket(enc_pckt2_flag);

							printf("Before encryption, third part...\n");
							print_dhPacket(pckt_h3);
							memcpy(buf_tobe_encrypted, (unsigned char *)pckt_h3, 16);
							buf_encrypted = encrypt_data_func(aes_key, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->key, buf_tobe_encrypted);
							// attach_header((pckt_h3->hdr).packet_type, (pckt_h3->hdr).src_addr, (pckt_h3->hdr).dest_addr, (pckt_h3->hdr).seq_no, pckt_h3->body.data, pckt_h3->body.dest, pckt_h3->instruction, buf_tobe_encrypted);
							memcpy(buf_encrypted + 16, (unsigned char *)&pckt_h3->flag, 2 * sizeof(char));
							memset(buffer + sizeof(dh_packet) + sizeof(dh_packet), 0, 18);
							memcpy(buffer + sizeof(dh_packet) + sizeof(dh_packet), buf_encrypted, 18);
							printf("After encryption, third part...\n");
							dh_packet *enc_pckt3_flag = (dh_packet*)(buffer + sizeof(dh_packet) + sizeof(dh_packet));
							print_dhPacket(enc_pckt3_flag);
							break;
					}
					printf("sending flag from node1 to router\n");
					switch(i)
					{
						case 1:
							print_dhPacket(pckt);
							printf("The flag for node%d is %d\n",sourceRoute[currentNode][threadArg->dest_node].first_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].first_hop]->flag);
							break;
						case 2:
							print_dhPacket(pckt);
							printf("Second part of the packet...\n");
							print_dhPacket(pckt_h2);
							if (i == sourceRoute[currentNode][threadArg->dest_node].num_hops)
							{
								printf("The flag for node%d is %d\n",threadArg->dest_node, shared_key[threadArg->dest_node]->flag);
							}
							else
							{
								printf("The flag for node%d is %d\n",sourceRoute[currentNode][threadArg->dest_node].second_hop, shared_key[sourceRoute[currentNode][threadArg->dest_node].second_hop]->flag);
							}
							break;
						case 3:
							print_dhPacket(pckt);
							printf("Second part of the packet...\n");
							print_dhPacket(pckt_h2);
							printf("Third part of the packet...\n");
							print_dhPacket(pckt_h3);
							printf("The flag for node%d is %d\n",threadArg->dest_node, shared_key[threadArg->dest_node]->flag);
							break;
					}
					n = sendto(sockfd, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
					if (n == -1)
					{
						printf("Send error\n");
					}
					fwrite(file_write, 1, sizeof(file_write), fp);
					fflush(fp);
					break;
				}
			}
		}
	}
	close(sockfd);
	// return 0;
}

void *ReceiveDH_Keys(void *inputargv)
{
	int sock_sniff;
	sock_sniff = createSocket(ETHERNET_PORT_NODE);

	int saddr_size , data_size=0;
	int a=0;
	struct sockaddr saddr;

	unsigned char *buffer;
	buffer = (unsigned char *)malloc(FRAME_LEN);

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = NODEID_MAC0;
	device.sll_addr[1]  = NODEID_MAC1;
	device.sll_addr[2]  = NODEID_MAC2;
	device.sll_addr[3]  = NODEID_MAC3;
	device.sll_addr[4]  = NODEID_MAC4;
	device.sll_addr[5]  = NODEID_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	saddr_size = sizeof saddr;

	printf("Sniffing DH started...\n");
	while(1)
	{
		memset (buffer, 0, FRAME_LEN);
		data_size= recvfrom(sock_sniff, buffer, FRAME_LEN, 0, &saddr, (socklen_t*)&saddr_size);
		dh_packet *pckt=(dh_packet*)(buffer);
		if(pckt->hdr.packet_type == DH_PACKET_TYPE_R1A 
			&& pckt->hdr.dest_addr == currentNode
			&& (pckt->instruction == REPLY_KEY || pckt->instruction == SAVE_FLAG))
			// && pckt->hdr.seq_no == DH_REQUEST)
		{
			printf("Received from router to node1\n");
			printf("pckt value of packet_type is %d\n", pckt->hdr.packet_type);
			printf("pckt value of src address is %d\n", pckt->hdr.src_addr);
			printf("pckt value of dst address is %d\n", pckt->hdr.dest_addr);
			printf("pckt value of sequence number is %d\n", pckt->hdr.seq_no);
			printf("pckt value of key is %d\n", pckt->body.data);
			printf("pckt value of dest is %d\n", pckt->body.dest);
			printf("pckt value of instruction is %d\n", pckt->instruction);
			printf("pckt value of flag is %d\n", pckt->flag);
		    // printf("pckt value of flag is %d\n",pckt->flag);
		    int recvKey = pckt->body.data;
		    int flag = pckt->flag;
	    	if ((device.sll_ifindex = if_nametoindex (ETHERNET_PORT_NODE)) == 0)
			{
				perror ("if_nametoindex() failed to obtain interface index ");
				exit(1);
			}

			if (pckt->instruction == SAVE_FLAG)
			{
				sprintf(file_write, "%d#%d", pckt->body.data, flag);
				pthread_mutex_unlock(&file_mutex);
				fwrite(file_write, 1, sizeof(file_write), fp);
				usleep(100);
				fwrite("\n", sizeof(char), 1, fp);
				fflush(fp);
				pthread_mutex_lock(&file_mutex);
			}
			else
			{
				// Preparing send packet
				(pckt->hdr).packet_type = currentNode_packetType;
				(pckt->hdr).dest_addr = (pckt->hdr).src_addr;
				(pckt->hdr).src_addr = currentNode;
				(pckt->hdr).seq_no = (pckt->hdr).seq_no + 1;
				pckt->instruction = KEY_ACK;

				/*==Diffie hellman start ==*/
				a = secretInteger(N);
				pckt->body.data=(int)((GENERATOR)^a)%N;
				// printf("packet_send->key = %d\n", pckt->body.data);
				/*==Diffie hellman end==*/

				printf("Going from node1 to router\n");
				printf("pckt value of packet_type is %d\n", pckt->hdr.packet_type);
				printf("pckt value of src address is %d\n", pckt->hdr.src_addr);
				printf("pckt value of dst address is %d\n", pckt->hdr.dest_addr);
				printf("pckt value of sequence number is %d\n", pckt->hdr.seq_no);
				printf("pckt value of key is %d\n", pckt->body.data);
				printf("pckt value of dest is %d\n", pckt->body.dest);
				printf("pckt value of instruction is %d\n", pckt->instruction);
				printf("pckt value of flag is %d\n", pckt->flag);

			    int send_result = sendto(sock_sniff, buffer, FRAME_LEN, 0, (struct sockaddr*)&device, sizeof(device));
		    	if (send_result == -1)
				{
					printf("Send error\n");
				}
				pthread_mutex_lock(&sharedKey_mutex);
				shared_key_counter++;
				shared_key[shared_key_counter]->key = recvKey * pckt->body.data;
				printf("The shared key with node%d is %d\n", shared_key_counter, shared_key[shared_key_counter]->key);
			    pthread_mutex_unlock(&sharedKey_mutex);
			}
		}
	}
	close(sock_sniff);
	return 0;
}

int main(int argc, char const *argv[])
{
	srand (time(0));

	fp = fopen(FILENAME, "w");
	file_write = (char *)malloc(sizeof(50));
	memset(file_write, 0, sizeof(file_write));

	int k=0;
	for (k = 1; k <= 10; ++k)
	{
		shared_key[k] = (struct sharedKey*)malloc(sizeof(struct sharedKey));
		memset(shared_key[k], 0, sizeof(struct sharedKey));
	}
	setupSourceRoute();

	int sender_dh_active = 0;
	int i = 0;
	pthread_create(&receiver_thread, 0, ReceiveDH_Keys, (void *)argv);
	if (argc > 1)
	{
		if (!strcmp(argv[1], "send"))
		{
			sender_dh_active = 1;
			for (i = 2; i < argc; ++i)
			{
				arg = (struct threadArguments*)malloc(sizeof(struct threadArguments));
				arg->dest_node = argv[i][0] - '0';
				// printf("arg->dest_node = %d\n", arg->dest_node);
				threadCount++;
				// pthread_create(&sender_thread[threadCount], 0, SendDH_Keys, (void *)arg);
				SendDH_Keys((void *)arg);
			}
		}
	}
	// if (sender_dh_active == 1)
	// {
	// 	int j = 0;
	// 	for (j = 0; j <= threadCount; ++j)
	// 	{
	// 		pthread_join(sender_thread[j], 0);
	// 	}
	// }
	printf("All sent.\n");
	pthread_join(receiver_thread, 0);
	fclose(fp);
	return 0;
}
