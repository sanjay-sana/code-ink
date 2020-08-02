#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <linux/if_ether.h>
#include <netinet/ip_icmp.h>    //Provides declarations for icmp header
#include <netinet/udp.h>        //Provides declarations for udp header
#include <netinet/tcp.h>   		//Provides declarations for tcp header
#include <netinet/ip.h>    		//Provides declarations for ip header
#include <linux/if_arp.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h> // for inet_ntoa()

pthread_t receiverThread[50], controllerThread;

FILE *fp = NULL;

int ports_to_listen[50];
int portCounter = 0;

char *hostIP;
char *hostPort;

// Partially obtained from StackOverflow
void getListOfEthernetInterfaces()
{
	hostPort = (char *)malloc(15);
	hostIP = (char *)malloc(20);
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;
	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family==AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			addr = inet_ntoa(sa->sin_addr);
			if (ifa->ifa_name[strlen(ifa->ifa_name)-1] == '0')
			{
				strcpy(hostPort, ifa->ifa_name);
				strcpy(hostIP, addr);
				break;
			}
		}
	}
	freeifaddrs(ifap);
}

void *connection_handler(void *socket_desc)
{
	int sock = *((int *)socket_desc);
	int read_size;
	char *message , client_message[2000];

	int packetCounter=0;
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		packetCounter++;
		printf("packetCounter = %d.\n", packetCounter);
	}
	free(socket_desc);	 
	return 0;
}

// Imported from Binary tides page which supports receiving data from multiple clients
// http://www.binarytides.com/socket-programming-c-linux-tutorial/
void *ReceiveData(void *input)
{
	int portNum = *((int *)input);
	printf("receiverThread on portNum %d\n", portNum);

	int sock_sniff, client_sock;
	int *new_sock;
	struct sockaddr_in server, client;
	char buffer_recv[2000];

	sock_sniff = socket(AF_INET , SOCK_STREAM , 0);
	if (sock_sniff == -1)
	{
		printf("Could not create socket");
		exit(1);
	}
	printf("Socket created Successfully\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(hostIP);
	server.sin_port = htons(portNum);

	if( bind(sock_sniff,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind failed. Error");
		exit(1);
	}
	puts("Bind Successful");

	listen(sock_sniff , 20);

	printf("Waiting for incoming connections...\n");
	int c=0;;
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(sock_sniff, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		 
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;
		 
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("could not create thread");
		}

		pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}

	return 0;
}

void command_parser(char **argv)
{
	printf("File = %s\n", argv[1]);
	fp = fopen(argv[1], "r");
	while (!feof(fp))
	{
		int tempPort=0;
		fscanf(fp, "%d", &tempPort);
		if (tempPort > 0)
		{
			ports_to_listen[portCounter] = tempPort;
			portCounter++;
		}
	}
}

int main(int argc, char *argv[])
{
	getListOfEthernetInterfaces();
	printf("hostIP = %s\n", hostIP);
	printf("hostPort = %s\n", hostPort);

	if (argc == 2)
	{
		command_parser(argv);
	}
	else
	{
		printf("Invalid arguments on command line.\n");
		exit(1);
	}

	int i = 0;
	for (i = 0; i < portCounter; ++i)
	{
		int *temp = malloc(sizeof(*temp));
		*temp = ports_to_listen[i];
		pthread_create(&receiverThread[i], 0, ReceiveData, temp);
	}
	for (i = 0; i < portCounter; ++i)
	{
		pthread_join(receiverThread[i], 0);
	}

	return 0;
}
