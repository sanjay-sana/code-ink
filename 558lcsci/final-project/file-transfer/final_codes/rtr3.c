#include "custom.h"
#include "header.h"

#define MY0_SRC_MAC0 0x00
#define MY0_SRC_MAC1 0x04
#define MY0_SRC_MAC2 0x23
#define MY0_SRC_MAC3 0xbb
#define MY0_SRC_MAC4 0x20
#define MY0_SRC_MAC5 0xb3 

int active_connection[30] = {0};

struct threadArguments
{
	char sniffEthPort[5];
};
struct threadArguments *arg;

struct routingTable
{
	int destNode;
	char sendInterface[5];
	int DHpacketType_send;
	int FTpacketType_send;
};
struct routingTable *route[10];

pthread_t forwardDHThread[SIMUL_FLOWS], forwardFTThread[SIMUL_FLOWS];

struct sharedKey
{
	int key;
	int flag;
};
struct sharedKey *shared_key[10];
int shared_key_counter = 0;

pthread_mutex_t sharedKey_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

void setupRoutingTable()
{
	int i;
	for (i = 1; i <= 10; ++i)
	{
		route[i] = (struct routingTable *)malloc(sizeof(struct routingTable));
	}
	route[1]->destNode = 1;
	strcpy(route[1]->sendInterface, "eth3");
	route[1]->DHpacketType_send = DH_PACKET_TYPE_R2A;
	route[1]->FTpacketType_send = FT_PACKET_TYPE_R2A;

	route[2]->destNode = 2;
	strcpy(route[2]->sendInterface, "eth3");
	route[2]->DHpacketType_send = DH_PACKET_TYPE_R2A;
	route[2]->FTpacketType_send = FT_PACKET_TYPE_R2A;

	route[3]->destNode = 3;
	strcpy(route[3]->sendInterface, "eth4");
	route[3]->DHpacketType_send = DH_PACKET_TYPE_R2C;
	route[3]->FTpacketType_send = FT_PACKET_TYPE_R2C;

	route[4]->destNode = 4;
	strcpy(route[4]->sendInterface, "eth1");
	route[4]->DHpacketType_send = DH_PACKET_TYPE_R2D;
	route[4]->FTpacketType_send = FT_PACKET_TYPE_R2D;

	route[5]->destNode = 5;
	strcpy(route[5]->sendInterface, "eth2");
	route[5]->DHpacketType_send = DH_PACKET_TYPE_R2B;
	route[5]->FTpacketType_send = FT_PACKET_TYPE_R2B;

	route[6]->destNode = 6;
	strcpy(route[6]->sendInterface, "eth2");
	route[6]->DHpacketType_send = DH_PACKET_TYPE_R2B;
	route[6]->FTpacketType_send = FT_PACKET_TYPE_R2B;

	route[7]->destNode = 7;
	strcpy(route[7]->sendInterface, "eth3");
	route[7]->DHpacketType_send = DH_PACKET_TYPE_R2A;
	route[7]->FTpacketType_send = FT_PACKET_TYPE_R2A;

	route[8]->destNode = 8;
	strcpy(route[8]->sendInterface, "eth4");
	route[8]->DHpacketType_send = DH_PACKET_TYPE_R2C;
	route[8]->FTpacketType_send = FT_PACKET_TYPE_R2C;

	route[9]->destNode = 9;
	strcpy(route[9]->sendInterface, "eth2");
	route[9]->DHpacketType_send = DH_PACKET_TYPE_R2B;
	route[9]->FTpacketType_send = FT_PACKET_TYPE_R2B;
}

int fetch_DHpacket_SendDetails(int dest, char *interface)
{
	strcpy(interface, route[dest]->sendInterface);
	// printf("dest = %d\n", dest);
	// printf("route[dest]->DHpacketType_send = %d\n", route[dest]->DHpacketType_send);
	// printf("route[dest]->sendInterface = %s\n", route[dest]->sendInterface);
	return route[dest]->DHpacketType_send;
}

int fetch_FTpacket_SendDetails(int dest, char *interface)
{
	strcpy(interface, route[dest]->sendInterface);
	return route[dest]->FTpacketType_send;
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

int generateRandomKey()
{
	int a = secretInteger(N);
	return (int)((GENERATOR)^a)%N;
}
void modify_packetForReply(dh_packet *pckt)
{
	short int temp=0;
	temp = pckt->hdr.dest_addr;
	pckt->hdr.dest_addr = pckt->hdr.src_addr;
	pckt->hdr.src_addr = temp;
	pckt->hdr.seq_no = pckt->hdr.seq_no + 1;
	pckt->instruction = KEY_ACK;
	int recvKey = pckt->body.data;
	pckt->body.data = generateRandomKey();
	shared_key_counter++;
	// pthread_mutex_lock(&sharedKey_mutex);
	if (shared_key[shared_key_counter]->key == 0)
	{
		shared_key[shared_key_counter]->key = recvKey * pckt->body.data;
		printf("The shared key with node%d is %d\n", shared_key_counter, shared_key[shared_key_counter]->key);
	}
	// pthread_mutex_unlock(&sharedKey_mutex);
}

void modify_packetForForward(dh_packet *pckt)
{
	pckt->hdr.dest_addr = pckt->body.dest;
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

void send_data(int sock_sniff, struct sockaddr_ll *device, unsigned char *buffer, dh_packet *pckt, char *send_interf)
{
	pckt->hdr.packet_type = fetch_DHpacket_SendDetails(pckt->hdr.dest_addr, send_interf);
	//printf("Send interface = %s\n", send_interf);
	if ((device->sll_ifindex = if_nametoindex (send_interf)) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit(1);
	}

	printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);
	printf("pckt value of src address is %d\n",pckt->hdr.src_addr);
	printf("pckt value of dst address is %d\n",pckt->hdr.dest_addr);
	printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);
	printf("pckt value of key is %d\n", pckt->body.data);
	printf("pckt value of dest is %d\n", pckt->body.dest);
	printf("pckt value of instruction is %d\n", pckt->instruction);
	printf("pckt value of flag is %d\n", pckt->flag);

	sendto(sock_sniff, buffer, frame_len, 0, (struct sockaddr*)device, sizeof(*device));
}

void send_file_data(int sock_sniff, struct sockaddr_ll *device, unsigned char *buffer, char *send_interf)
{
	// pckt->hdr.packet_type = fetch_FTpacket_SendDetails(pckt->hdr.dest_addr, send_interf);
	//printf("Send interface = %s\n", send_interf);
	if ((device->sll_ifindex = if_nametoindex (send_interf)) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit(1);
	}

	// printf("pckt value of packet_type is %d\n",pckt->hdr.packet_type);
	// printf("pckt value of src address is %d\n",pckt->hdr.src_addr);
	// printf("pckt value of dst address is %d\n",pckt->hdr.dest_addr);
	// printf("pckt value of sequence number is %d\n",pckt->hdr.seq_no);

	sendto(sock_sniff, buffer, frame_len, 0, (struct sockaddr*)device, sizeof(*device));
}

int fetchKey_usingFlag(short int flag)
{
	int i=1;
	// pthread_mutex_lock(&sharedKey_mutex);
	for (i = 1; i <= shared_key_counter; ++i)
	{
		if (flag == shared_key[i]->flag)
		{
			return shared_key[i]->key;
		}
	}
	// pthread_mutex_unlock(&sharedKey_mutex);
	return 0;
}

unsigned char *keyCalculate(int key)
{
    unsigned char *aes_key=(char*)malloc(16*sizeof(unsigned char));
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

unsigned char* AESdecrypt(unsigned char *aes_input, unsigned char aes_key[], int size, unsigned char *dec_out)
{
    unsigned char iv_dec[AES_BLOCK_SIZE];
    int i;
    memset(iv_dec, 0x00, AES_BLOCK_SIZE);
    int keylength=128;
    AES_KEY dec_key;
    memset(dec_out, 0, sizeof(frame_len));
    AES_set_decrypt_key(aes_key, keylength, &dec_key);
    AES_cbc_encrypt(aes_input, dec_out, size, &dec_key, iv_dec, AES_DECRYPT);
    return dec_out;
}

unsigned char* decrypt_data_func(unsigned char* aes_key, int getKey, unsigned char* buf_tobe_decrypted, unsigned char *dec_out)
{
	aes_key = keyCalculate(getKey);
	return AESdecrypt(buf_tobe_decrypted, aes_key, 16, dec_out);
}

void *sniffDH_intf_func(void* arg)
{
	struct threadArguments *threadArg = (struct threadArguments*)arg;

	int sock_sniff;
	sock_sniff = createSocket(threadArg->sniffEthPort);

	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer;
	buffer = (unsigned char *)malloc(frame_len);

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY0_SRC_MAC0;
	device.sll_addr[1]  = MY0_SRC_MAC1;
	device.sll_addr[2]  = MY0_SRC_MAC2;
	device.sll_addr[3]  = MY0_SRC_MAC3;
	device.sll_addr[4]  = MY0_SRC_MAC4;
	device.sll_addr[5]  = MY0_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	int send_result = 0;
	saddr_size = sizeof saddr;
	char send_interf[5];	
	dh_packet *pckt=(dh_packet*)(buffer);
	dh_packet *pckt2 = (dh_packet*)(buffer + sizeof(dh_packet));

	unsigned char *aes_key = (unsigned char *)malloc(AES_BLOCK_SIZE * sizeof(unsigned char));
	unsigned char *buf_tobe_decrypted = (unsigned char *)malloc(frame_len);
	unsigned char *buf_decrypted = (unsigned char *)malloc(frame_len);

	unsigned char *dec_out = (unsigned char *)malloc(frame_len * sizeof(unsigned char));

	int i=1;
	printf("Sniffing DH on %s...\n", threadArg->sniffEthPort);
	while(1)
	{
		memset(buffer, 0, frame_len);
		memset(buf_tobe_decrypted, 0, frame_len);
		memset(buf_decrypted, 0, frame_len);
		data_size= recvfrom(sock_sniff, buffer, frame_len, 0, &saddr, (socklen_t*)&saddr_size);
		if	(!strcmp(threadArg->sniffEthPort, "eth4") && pckt->hdr.packet_type == DH_PACKET_TYPE_N3)
		{
			printf("sniffed on eth4\n");
			printf("4pckt value of packet_type is %d\n", pckt->hdr.packet_type);
			printf("4pckt value of src address is %d\n", pckt->hdr.src_addr);
			printf("4pckt value of dst address is %d\n", pckt->hdr.dest_addr);
			printf("4pckt value of sequence number is %d\n", pckt->hdr.seq_no);
			printf("4pckt value of key is %d\n", pckt->body.data);
			printf("4pckt value of dest is %d\n", pckt->body.dest);
			printf("4pckt value of instruction is %d\n", pckt->instruction);
			printf("4pckt value of flag is %d\n", pckt->flag);
			switch(pckt->instruction)
			{
				case REPLY_KEY:
					printf("sniffed on eth4 REPLY_KEY\n");
					modify_packetForReply(pckt);
					send_data(sock_sniff, &device, buffer, pckt, send_interf);
					break;
				case DECRYPT_FORWARD:
					printf("sniffed on eth4 DECRYPT_FORWARD\n");
					
					int getKey = fetchKey_usingFlag(pckt->flag);
					memcpy(buf_tobe_decrypted, buffer + sizeof(dh_packet), 16);
					buf_decrypted = decrypt_data_func(aes_key, getKey, buf_tobe_decrypted, dec_out);
					memcpy(buffer + sizeof(dh_packet), buf_decrypted, 16);

					send_data(sock_sniff, &device, buffer + sizeof(dh_packet), (dh_packet*)(buffer + sizeof(dh_packet)), send_interf);
					// pthread_mutex_lock(&active_mutex);
					active_connection[pckt2->hdr.seq_no+1] = pckt->hdr.src_addr;
					// pthread_mutex_unlock(&active_mutex);
					break;
				case KEY_ACK:
					printf("sniffed on eth4 KEY_ACK\n");
					(pckt->hdr).src_addr = R2;
					// pthread_mutex_lock(&active_mutex);
					(pckt->hdr).dest_addr = active_connection[pckt->hdr.seq_no];
					// pthread_mutex_unlock(&active_mutex);
					send_data(sock_sniff, &device, buffer, pckt, send_interf);
					break;
				case SAVE_FLAG:
					printf("sniffed on eth4 SAVE_FLAG\n");
					// pthread_mutex_lock(&sharedKey_mutex);
					for (i = 1; i <= shared_key_counter; ++i)
					{
						if (shared_key[i]->key == pckt->body.data)
						{
							shared_key[i]->flag = pckt->flag;
							printf("flag updated.\n");
						}
					}
					// pthread_mutex_unlock(&sharedKey_mutex);
					break;
			}
		}
		else
		{
			if (!strcmp(threadArg->sniffEthPort, "eth3") && pckt->hdr.packet_type == DH_PACKET_TYPE_R1D)
			{
				printf("sniffed on eth3\n");
				printf("3pckt value of packet_type is %d\n", pckt->hdr.packet_type);
				printf("3pckt value of src address is %d\n", pckt->hdr.src_addr);
				printf("3pckt value of dst address is %d\n", pckt->hdr.dest_addr);
				printf("3pckt value of sequence number is %d\n", pckt->hdr.seq_no);
				printf("3pckt value of key is %d\n", pckt->body.data);
				printf("3pckt value of dest is %d\n", pckt->body.dest);
				printf("3pckt value of instruction is %d\n", pckt->instruction);
				printf("3pckt value of flag is %d\n", pckt->flag);
				switch(pckt->instruction)
				{
					case REPLY_KEY:
						printf("sniffed on eth3 REPLY_KEY\n");
						modify_packetForReply(pckt);
						send_data(sock_sniff, &device, buffer, pckt, send_interf);
						break;
					case DECRYPT_FORWARD:
						printf("sniffed on eth3 DECRYPT_FORWARD\n");

						int getKey = fetchKey_usingFlag(pckt->flag);
						memcpy(buf_tobe_decrypted, buffer + sizeof(dh_packet), 16);
						buf_decrypted = decrypt_data_func(aes_key, getKey, buf_tobe_decrypted, dec_out);
						memcpy(buffer + sizeof(dh_packet), buf_decrypted, 16);

						send_data(sock_sniff, &device, buffer + sizeof(dh_packet), (dh_packet*)(buffer + sizeof(dh_packet)), send_interf);
						// pthread_mutex_lock(&active_mutex);
						active_connection[pckt2->hdr.seq_no+1] = pckt->hdr.src_addr;
						// pthread_mutex_unlock(&active_mutex);
						break;
					case KEY_ACK:
						printf("sniffed on eth3 KEY_ACK\n");
						(pckt->hdr).src_addr = R2;
						// pthread_mutex_lock(&active_mutex);
						(pckt->hdr).dest_addr = active_connection[pckt->hdr.seq_no];
						// pthread_mutex_unlock(&active_mutex);
						send_data(sock_sniff, &device, buffer, pckt, send_interf);
						break;
					case SAVE_FLAG:
						printf("sniffed on Eth6 SAVE_FLAG\n");
						// pthread_mutex_lock(&sharedKey_mutex);
						for (i = 1; i <= shared_key_counter; ++i)
						{
							if (shared_key[i]->key == pckt->body.data)
							{
								shared_key[i]->flag = pckt->flag;
								printf("flag updated.\n");
							}
						}
						// pthread_mutex_unlock(&sharedKey_mutex);
						break;
				}
			}
			else
			{
				if (!strcmp(threadArg->sniffEthPort, "eth1") && pckt->hdr.packet_type == DH_PACKET_TYPE_N4)
				{
					printf("sniffed on eth1\n");
					printf("1pckt value of packet_type is %d\n", pckt->hdr.packet_type);
					printf("1pckt value of src address is %d\n", pckt->hdr.src_addr);
					printf("1pckt value of dst address is %d\n", pckt->hdr.dest_addr);
					printf("1pckt value of sequence number is %d\n", pckt->hdr.seq_no);
					printf("1pckt value of key is %d\n", pckt->body.data);
					printf("1pckt value of dest is %d\n", pckt->body.dest);
					printf("1pckt value of instruction is %d\n", pckt->instruction);
					printf("1pckt value of flag is %d\n", pckt->flag);
					switch(pckt->instruction)
					{
						case REPLY_KEY:
							printf("sniffed on eth1 REPLY_KEY\n");
							modify_packetForReply(pckt);
							send_data(sock_sniff, &device, buffer, pckt, send_interf);
							break;
						case DECRYPT_FORWARD:
							printf("sniffed on eth1 DECRYPT_FORWARD\n");

							int getKey = fetchKey_usingFlag(pckt->flag);
							memcpy(buf_tobe_decrypted, buffer + sizeof(dh_packet), 16);
							buf_decrypted = decrypt_data_func(aes_key, getKey, buf_tobe_decrypted, dec_out);
							memcpy(buffer + sizeof(dh_packet), buf_decrypted, 16);
							send_data(sock_sniff, &device, buffer + sizeof(dh_packet), (dh_packet*)(buffer + sizeof(dh_packet)), send_interf);
							// pthread_mutex_lock(&active_mutex);
							active_connection[pckt2->hdr.seq_no+1] = pckt->hdr.src_addr;
							// pthread_mutex_unlock(&active_mutex);
							break;
						case KEY_ACK:
							printf("sniffed on eth1 KEY_ACK\n");
							(pckt->hdr).src_addr = R2;
							// pthread_mutex_lock(&active_mutex);
							(pckt->hdr).dest_addr = active_connection[pckt->hdr.seq_no];
							// pthread_mutex_unlock(&active_mutex);
							send_data(sock_sniff, &device, buffer, pckt, send_interf);
							break;
						case SAVE_FLAG:
							printf("sniffed on eth1 SAVE_FLAG\n");
							// pthread_mutex_lock(&sharedKey_mutex);
							for (i = 1; i <= shared_key_counter; ++i)
							{
								if (shared_key[i]->key == pckt->body.data)
								{
									shared_key[i]->flag = pckt->flag;
									printf("flag updated.\n");
								}
							}
							// pthread_mutex_unlock(&sharedKey_mutex);
							break;
					}
				}
				else
				{
					if (!strcmp(threadArg->sniffEthPort, "eth2") && pckt->hdr.packet_type == DH_PACKET_TYPE_R3D)
					{
						printf("sniffed on eth2\n");
						printf("2pckt value of packet_type is %d\n", pckt->hdr.packet_type);
						printf("2pckt value of src address is %d\n", pckt->hdr.src_addr);
						printf("2pckt value of dst address is %d\n", pckt->hdr.dest_addr);
						printf("2pckt value of sequence number is %d\n", pckt->hdr.seq_no);
						printf("2pckt value of key is %d\n", pckt->body.data);
						printf("2pckt value of dest is %d\n", pckt->body.dest);
						printf("2pckt value of instruction is %d\n", pckt->instruction);
						printf("2pckt value of flag is %d\n", pckt->flag);
						switch(pckt->instruction)
						{
							case REPLY_KEY:
								printf("sniffed on eth2 REPLY_KEY\n");
								modify_packetForReply(pckt);
								send_data(sock_sniff, &device, buffer, pckt, send_interf);
								break;
							case DECRYPT_FORWARD:
								printf("sniffed on eth2 DECRYPT_FORWARD\n");

								int getKey = fetchKey_usingFlag(pckt->flag);
								memcpy(buf_tobe_decrypted, buffer + sizeof(dh_packet), 16);
								buf_decrypted = decrypt_data_func(aes_key, getKey, buf_tobe_decrypted, dec_out);
								memcpy(buffer + sizeof(dh_packet), buf_decrypted, 16);

								send_data(sock_sniff, &device, buffer + sizeof(dh_packet), (dh_packet*)(buffer + sizeof(dh_packet)), send_interf);
								// pthread_mutex_lock(&active_mutex);
								active_connection[pckt2->hdr.seq_no+1] = pckt->hdr.src_addr;
								// pthread_mutex_unlock(&active_mutex);
								break;
							case KEY_ACK:
								printf("sniffed on eth2 KEY_ACK\n");
								(pckt->hdr).src_addr = R2;
								// pthread_mutex_lock(&active_mutex);
								(pckt->hdr).dest_addr = active_connection[pckt->hdr.seq_no];
								// pthread_mutex_unlock(&active_mutex);
								send_data(sock_sniff, &device, buffer, pckt, send_interf);
								break;
							case SAVE_FLAG:
								printf("sniffed on eth2 SAVE_FLAG\n");
								// pthread_mutex_lock(&sharedKey_mutex);
								for (i = 1; i <= shared_key_counter; ++i)
								{
									if (shared_key[i]->key == pckt->body.data)
									{
										shared_key[i]->flag = pckt->flag;
										printf("flag updated.\n");
									}
								}
								// pthread_mutex_unlock(&sharedKey_mutex);
								break;
						}
					}
				}
			}
		}
	}
	close(sock_sniff);
}

void *sniffFT_intf_func(void* arg)
{
	unsigned char*hdr1_buffer=(unsigned char*)malloc(sizeof(unsigned char)*HDR_SIZE);
	unsigned char*hdr2_buffer=(unsigned char*)malloc(sizeof(unsigned char)*HDR_SIZE);
	unsigned char*hdr2_dec_buffer=(unsigned char*)malloc(sizeof(unsigned char)*HDR_SIZE);
	unsigned char*data_buffer=(unsigned char*)malloc(sizeof(unsigned char)*data_len);
//	unsigned char*new_packet_buffer=(unsigned char*)malloc(sizeof(unsigned char)*(frame_len-HDR_SIZE));
    unsigned char *aes_key=(unsigned char*)malloc(AES_BLOCK_SIZE*sizeof(unsigned char));
    
    aes_key = keyCalculate(5);

	memset(hdr2_buffer, 0, HDR_SIZE);
	memset(hdr2_dec_buffer, 0, HDR_SIZE);

	unsigned char *dec_out = (unsigned char *)malloc(frame_len * sizeof(unsigned char));
	struct threadArguments *threadArg = (struct threadArguments*)arg;
	// printf("threadNum = %d\n", threadArg->threadNum);
	// printf("sniffEthPort = %s\n", threadArg->sniffEthPort);
	int sock_sniff;
	sock_sniff = createSocket(threadArg->sniffEthPort);


	int sock_sniff1;
	sock_sniff1 = createSocket(threadArg->sniffEthPort);

	int saddr_size , data_size;
	struct sockaddr saddr;


	unsigned char *buffer,*tmp_hdr,*tmp_hdr_decrypted,*new_packet_buffer;
	buffer = (unsigned char *)malloc(frame_len+10);
	new_packet_buffer=(unsigned char*)malloc(frame_len);
	tmp_hdr = (unsigned char*)malloc(HDR_SIZE);
	tmp_hdr_decrypted = (unsigned char*)malloc(HDR_SIZE);

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY0_SRC_MAC0;
	device.sll_addr[1]  = MY0_SRC_MAC1;
	device.sll_addr[2]  = MY0_SRC_MAC2;
	device.sll_addr[3]  = MY0_SRC_MAC3;
	device.sll_addr[4]  = MY0_SRC_MAC4;
	device.sll_addr[5]  = MY0_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);
	header_format1 *pckt=(header_format1 *)buffer;
	int send_result = 0;
	saddr_size = sizeof saddr;
	char send_interf[5];
	printf("Sniffing FT on %s...\n", threadArg->sniffEthPort);


    EVP_CIPHER_CTX ctx_enc,ctx_dec;
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};
    int outlen1=HDR_SIZE;

	while(1)
	{
		//pckt=NULL;
		memset (buffer, 0, frame_len+10);
		data_size= recvfrom(sock_sniff, buffer, frame_len, 0, &saddr, (socklen_t*)&saddr_size);
		pckt = (header_format1*)buffer;
		//	printf("\ngetting paclets %d\n",pckt->packet_type );

		if (!strcmp(threadArg->sniffEthPort, "eth4") && (pckt->packet_type == FT_PACKET_TYPE_R3A))
		{

		EVP_CIPHER_CTX ctx_enc,ctx_dec;
    		int outlen1=HDR_SIZE;
    		int outlen2=frame_len;
    		unsigned char key[16] = {0};
    		unsigned char iv[16] = {0};
			tmp_hdr=buffer+HDR_SIZE;
	    	EVP_DecryptInit(&ctx_dec,EVP_aes_128_cbc(),key,iv);
    		EVP_DecryptUpdate(&ctx_dec,buffer+HDR_SIZE,&outlen1,tmp_hdr,HDR_SIZE);



    		//header_format1*t=(header_format1*)(buffer+HDR_SIZE);
			send_file_data(sock_sniff1, &device, buffer+HDR_SIZE , "eth2");
			//c++;
		
		}

		else if(!strcmp(threadArg->sniffEthPort, "eth4") && (pckt->packet_type == ACK_PACKET_TYPE_N2))
		{	 									
		}
		if (!strcmp(threadArg->sniffEthPort, "eth2") && (pckt->packet_type == FT_PACKET_TYPE_R3B))
		{
		}
		if(!strcmp(threadArg->sniffEthPort, "eth2") && (pckt->packet_type == ACK_PACKET_TYPE_N5))
		{	 
									
									//printf("ack packet" );
			pckt->packet_type = ACK_PACKET_TYPE_R3A;
			//strcpy(send_interf, route[pckt->dest_addr]->sendInterface); //FIX dest address in the node codes
			send_file_data(sock_sniff1, &device, buffer, "eth4");
			//usleep(1);
										
		}

		/*******************link failure******************/


	}
	close(sock_sniff);
}
/*
pthread_t broadcast;
void *random_packets()
{
	while(1)
	{
		printf("\nrandom_packets\n" );
		sleep(1);
	
	}
}*/
int main(int argc, char const *argv[])
{
	srand(time(NULL));

	char *sniffPort[4];
	sniffPort[0] = (char*)malloc(5*sizeof(char));
	sniffPort[1] = (char*)malloc(5*sizeof(char));
	sniffPort[2] = (char*)malloc(5*sizeof(char));
	sniffPort[3] = (char*)malloc(5*sizeof(char));
	strcpy(sniffPort[0], "eth1");
	strcpy(sniffPort[1], "eth2");
	strcpy(sniffPort[2], "eth3");
	strcpy(sniffPort[3], "eth4");

	int k=0;
	for (k = 1; k <= 10; ++k)
	{
		shared_key[k] = (struct sharedKey*)malloc(sizeof(struct sharedKey));
		memset(shared_key[k], 0, sizeof(struct sharedKey));
	}

	setupRoutingTable();

	printf("    destNode       ||    sendInterface    ||   DHpacketType_send    ||   FTpacketType_send\n");
	printf("       %d           ||",   route[1]->destNode);
	printf("        %s         ||",   route[1]->sendInterface);
	printf("           %d            ||", route[1]->DHpacketType_send);
	printf("      %d       \n", route[1]->FTpacketType_send);

	printf("       %d           ||",   route[2]->destNode);
	printf("        %s         ||",   route[2]->sendInterface);
	printf("           %d            ||", route[2]->DHpacketType_send);
	printf("      %d       \n", route[2]->FTpacketType_send);

	printf("       %d           ||",   route[3]->destNode);
	printf("        %s         ||",   route[3]->sendInterface);
	printf("           %d            ||", route[3]->DHpacketType_send);
	printf("      %d       \n", route[3]->FTpacketType_send);

	printf("       %d           ||",   route[4]->destNode);
	printf("        %s         ||",   route[4]->sendInterface);
	printf("           %d            ||", route[4]->DHpacketType_send);
	printf("      %d       \n", route[4]->FTpacketType_send);

	printf("       %d           ||",   route[5]->destNode);
	printf("        %s         ||",   route[5]->sendInterface);
	printf("           %d            ||", route[5]->DHpacketType_send);
	printf("      %d       \n", route[5]->FTpacketType_send);

	printf("       %d           ||",   route[6]->destNode);
	printf("        %s         ||",   route[6]->sendInterface);
	printf("           %d            ||", route[6]->DHpacketType_send);
	printf("      %d       \n", route[6]->FTpacketType_send);

	printf("       %d           ||",   route[7]->destNode);
	printf("        %s         ||",   route[7]->sendInterface);
	printf("           %d            ||", route[7]->DHpacketType_send);
	printf("      %d       \n", route[7]->FTpacketType_send);

	printf("       %d           ||",   route[8]->destNode);
	printf("        %s         ||",   route[8]->sendInterface);
	printf("           %d            ||", route[8]->DHpacketType_send);
	printf("      %d       \n", route[8]->FTpacketType_send);

	printf("       %d           ||",   route[9]->destNode);
	printf("        %s         ||",   route[9]->sendInterface);
	printf("           %d            ||", route[9]->DHpacketType_send);
	printf("      %d       \n", route[9]->FTpacketType_send);

	// Exchange DH symmetric keys
	int i = 0, j = 0;
/*	for (i = 0; i < SIMUL_FLOWS; ++i)
	{
		arg = malloc(sizeof(struct threadArguments));
		strcpy(arg->sniffEthPort, sniffPort[i]);
		pthread_create(&forwardDHThread[i], 0, sniffDH_intf_func, (void *)arg);
	}*/
	for (i = 0; i < SIMUL_FLOWS; ++i)
	{
		arg = malloc(sizeof(struct threadArguments));
		strcpy(arg->sniffEthPort, sniffPort[i]);
		pthread_create(&forwardFTThread[i], 0, sniffFT_intf_func, (void *)arg);
	}
/*
pthread_create(&broadcast,0,random_packets,(void*)arg);
		pthread_join(broadcast, 0);
*/
    int policy = SCHED_FIFO;
  struct sched_param    param;

 memset(&param, 0, sizeof(param));
  /* Bump the priority of the thread a small amount */
  //if (thePriority - BUMP_PRIO >= PRIORITY_MIN_NP) {
      param.sched_priority = 99;
 // }


	int rc = pthread_setschedparam(forwardFTThread[2], policy, &param);
	/*
	for (j = 0; j < SIMUL_FLOWS; ++j)
	{
		pthread_join(forwardDHThread[j], 0);
	}*/
	for (j = 0; j < SIMUL_FLOWS; ++j)
	{
		pthread_join(forwardFTThread[j], 0);
	}
	return 0;
}