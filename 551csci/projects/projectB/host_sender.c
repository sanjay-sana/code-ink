#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <linux/if_ether.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <linux/if_arp.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h> // for inet_ntoa()

#define PAYLOAD_SIZE          1460

struct fileDetails
{
   char dest_ip[20];
   int dest_port;
   int fileSize_inBytes;
};
struct fileDetails *file_input[50];

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

char *hostIP;
char *hostPort;
char *hostControllerIP;

pthread_t senderThread[50], controllerThread;

FILE *fp = NULL;

// int ports_to_listen[10];
int portCounter = 0;

// Partially obtained from StackOverflow
void getListOfEthernetInterfaces()
{
	hostPort = (char *)malloc(15);
	hostIP = (char *)malloc(20);
	hostControllerIP = (char *)malloc(20);
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family==AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			addr = inet_ntoa(sa->sin_addr);
			if (ifa->ifa_name[strlen(ifa->ifa_name)-1] == '0')
			{
				strcpy(hostPort, ifa->ifa_name);
				strcpy(hostIP, addr);
			}
			else if (ifa->ifa_name[strlen(ifa->ifa_name)-1] == '1')
			{
				strcpy(hostControllerIP, addr);
			}
		}
	}
	freeifaddrs(ifap);
}

// Imported code with some modifications from the below link - Allows to receive and reply 
// back with a UDP packet to make a communication with the controller.
// https://www.abc.se/~m6695/udp.html
void *get_flow_state()
{
	printf("controllerThread.\n");
	struct sockaddr_in si_me, si_other;
	 
	int s, i, slen = sizeof(si_other) , recv_len;
	char buf[2000];
	 
	//create a UDP socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket");
	}
	 
	memset((char *) &si_me, 0, sizeof(si_me));
	 
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(5000);
	si_me.sin_addr.s_addr = inet_addr(hostControllerIP);
	 
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		perror("bind");
	}
	 
	while(1)
	{
		printf("Waiting for data...\n");
		fflush(stdout);
		 
		if ((recv_len = recvfrom(s, buf, 2000, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			perror("recvfrom()");
		}
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);
		strcpy(buf, "I got you...");
		si_other.sin_port = htons(5010);
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			perror("sendto()");
		}
	}
 
	close(s);
	return 0;
}

void *SendData(void *input)
{
	int flowIdentifier = *((int *)input);
	printf("senderThread for flow %d\n", flowIdentifier);

	int sockSend;
	struct sockaddr_in server;

	unsigned char *buffer_send;
	buffer_send = (unsigned char *)malloc(PAYLOAD_SIZE);
	memset(buffer_send, '\0', PAYLOAD_SIZE);

	int seq_no_max = 0;
	if((file_input[flowIdentifier]->fileSize_inBytes % PAYLOAD_SIZE) == 0)
	{
		seq_no_max = (file_input[flowIdentifier]->fileSize_inBytes / PAYLOAD_SIZE);
	}
	else
	{
		seq_no_max = (file_input[flowIdentifier]->fileSize_inBytes / PAYLOAD_SIZE) + 1;
	}
	printf("seq_no_max = %d\n", seq_no_max);

	sockSend = socket(AF_INET , SOCK_STREAM , 0);
	if (sockSend == -1)
	{
		printf("Could not create socket");
	}
	printf("Socket created\n");
	 
	server.sin_addr.s_addr = inet_addr(file_input[flowIdentifier]->dest_ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(file_input[flowIdentifier]->dest_port);

	while(connect(sockSend , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error.");
		printf("Try again\n");
	}
	printf("Connected\n");

	int currentSeq = 0;
	while(currentSeq < seq_no_max)
	{
		printf("currentSeq = %d\n", currentSeq);
		memset(buffer_send, '\0', PAYLOAD_SIZE);

		if (file_input[flowIdentifier]->fileSize_inBytes - (currentSeq * PAYLOAD_SIZE) > PAYLOAD_SIZE)
		{
			memset(buffer_send, '$', PAYLOAD_SIZE);
		}
		else
		{
			memset(buffer_send, '$', file_input[flowIdentifier]->fileSize_inBytes - (currentSeq * PAYLOAD_SIZE));
		}
		printf("%zu\n", strlen(buffer_send));

		if( send(sockSend , buffer_send , strlen(buffer_send) , 0) < 0)
		{
			puts("Send failed");
		}
		currentSeq++;
		usleep(1000);
	}
	sleep(1);
	close(sockSend);
}

void command_parser(char **argv)
{
	printf("argv = %s\n", argv[1]);
	fp = fopen(argv[1], "r");

	char *fileLine = malloc(100);
	memset(fileLine, '\0', 100);

	while (fgets(fileLine, 100, fp))
	{
		file_input[portCounter] = (struct fileDetails*)malloc(sizeof(struct fileDetails));
		printf("%s", fileLine);
		const char s[2] = " ";
		char *token;
		token = strtok(fileLine, s);
		while( token != NULL ) 
		{
			if (strcmp(token, "-d") == 0)
			{
				token = strtok(NULL, s);
				strcpy(file_input[portCounter]->dest_ip, token);
				printf("file_input[%d]->dest_ip = %s\n", portCounter, file_input[portCounter]->dest_ip);
			}
			else if (strcmp(token, "-p") == 0)
			{
				token = strtok(NULL, s);
				file_input[portCounter]->dest_port = atoi(token);
				printf("file_input[%d]->dest_port = %d\n", portCounter, file_input[portCounter]->dest_port);
			}
			else if(strcmp(token, "-n") == 0)
			{
				token = strtok(NULL, s);
				int i=0;
				for (i = 0; i < strlen(token); ++i)
				{
					if (token[i] >= 'A')
					{
						char *numTemp = malloc(10);
						strncpy(numTemp, token, (i));
						file_input[portCounter]->fileSize_inBytes = atoi(numTemp);
						if (token[i] == 'M')
						{
							file_input[portCounter]->fileSize_inBytes *= 1000000;
							printf("file_input[%d]->fileSize_inBytes = %d\n", portCounter, file_input[portCounter]->fileSize_inBytes);
						}
						else if(token[i] == 'K')
						{
							file_input[portCounter]->fileSize_inBytes *= 1000;
							printf("file_input[%d]->fileSize_inBytes = %d\n", portCounter, file_input[portCounter]->fileSize_inBytes);
						}
						break;
					}
				}
			}
			token = strtok(NULL, s);
		}
		portCounter++;
	}
	printf("portCounter = %d\n", portCounter);
}

int main(int argc, char *argv[])
{
	getListOfEthernetInterfaces();

	printf("hostIP = %s\n", hostIP);
	printf("hostPort = %s\n", hostPort);

	printf("hostControllerIP = %s\n", hostControllerIP);
	
	if (argc == 2)
	{
		command_parser(argv);
	}
	else
	{
		printf("Invalid arguments on command line.\n");
		exit(1);
	}

	pthread_create(&controllerThread, 0, get_flow_state, (void *)1);

	int i=0;
	for (i = 0; i < portCounter; ++i)
	{
		int *temp = malloc(sizeof(*temp));
		*temp = i;
		pthread_create(&senderThread[i], 0, SendData, temp);
	}
	for (i = 0; i < portCounter; ++i)
	{
		pthread_join(senderThread[i], 0);
	}
	pthread_join(controllerThread, 0);

	return 0;
}
