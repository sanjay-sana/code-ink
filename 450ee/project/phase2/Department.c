#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT  "4197"
#define DEPT_NUM      2
#define PROG_NUM      3
#define NUNKI_SERVER "nunki.usc.edu"
#define UDP_DEPT     "22097"

/* Function declarations */
void processData(int, FILE *, int, char*);

/* Obtain socket information using getsockname function */
void sock_addr(int s, char dept)
{
	int z;
	struct sockaddr_in adr_inet;
	socklen_t len_inet;
	char buf[100];
	size_t bufsiz = sizeof(buf);
	len_inet = sizeof(adr_inet);
	z = getsockname(s, (struct sockaddr *)&adr_inet, &len_inet);
	if ( z == -1 )
	{
		perror("getsockname() error");
		exit(1);
	}
	snprintf(buf,bufsiz, "Department %c has TCP port %u and IP address %s for Phase 1", dept, (unsigned)ntohs(adr_inet.sin_port), inet_ntoa(adr_inet.sin_addr));
	printf("%s\n", buf);
}

/* Obtain socket information using getsockname function */
void udp_sock_addr(int s, char dept)
{
	int z;
	struct sockaddr_in adr_inet;
	socklen_t len_inet;
	char buf[100];
	size_t bufsiz = sizeof(buf);
	len_inet = sizeof(adr_inet);
	z = getsockname(s, (struct sockaddr *)&adr_inet, &len_inet);
	if ( z == -1 )
	{
		perror("getsockname() error");
		exit(1);
	}
	snprintf(buf,bufsiz, "Department %c has UDP port %u and IP address %s for Phase 2", dept, (unsigned)ntohs(adr_inet.sin_port), inet_ntoa(adr_inet.sin_addr));
	printf("%s\n", buf);
}

/* Create socket to connect to the server */
int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	int pid, pid_return;
	int sockfd;
	int loop;
	struct hostent *he;
	char ip[100];
	he = gethostbyname(NUNKI_SERVER);
	struct in_addr **addr_list;
	addr_list = (struct in_addr **) he->h_addr_list;
	int i;
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		strcpy(ip , inet_ntoa(*addr_list[i]) );
	}

	for (loop = 0; loop < DEPT_NUM; loop++)
	{
		pid = fork();
		if (pid < 0)
		{
			perror("ERROR on fork");
			exit(1);
		}
		if (pid == 0)
		{
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				perror("ERROR opening socket");
				exit(1);
			}
			processData(sockfd, fp, loop, ip);
			exit(0);
		}
		else
		{
			waitpid(pid, &pid_return, 0);
			if (pid_return == 0)
			{
				if (loop == 0)
				{
					printf("End of phase 1 for Department A\n");
				}
				else if (loop == 1)
				{
					printf("End of phase 1 for Department B\n");
				}
			}
			else
			{
				printf("Process terminated with an error\n");
			}
		}
	}
	return 0;
}

void processDeptResults(char *ip)
{
	int udp_sockfd, udp_newsockfd;
	int udp_pid, udp_pid_return;
	struct addrinfo udp_hints, *udp_serv_addr, udp_cli_addr;
	socklen_t udp_clilen;
	int yes = 1;
	int return_val;
	int numbytes;
	memset(&udp_hints, 0, sizeof(udp_hints));
	udp_hints.ai_family = AF_INET;
	udp_hints.ai_socktype = SOCK_DGRAM;
	udp_hints.ai_flags = AI_PASSIVE;

	if ((return_val = getaddrinfo(ip, UDP_DEPT, &udp_hints, &udp_serv_addr)) != 0)
	{
		perror("udp getaddrinfo() error");
		exit(1);
	}
	if ((udp_sockfd = socket(udp_serv_addr->ai_family, udp_serv_addr->ai_socktype, udp_serv_addr->ai_protocol)) == -1 )
	{
		perror("udp socket opening error");
		exit(1);
	}
	if (setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("udp setsockopt() error");
		exit(1);
	}
	if (bind(udp_sockfd, udp_serv_addr->ai_addr, udp_serv_addr->ai_addrlen) == -1)
	{
		close(udp_sockfd);
		perror("udp bind() error");
		exit(1);
	}
	freeaddrinfo(udp_serv_addr);
	int printA_check=0;
	int printB_check=0;
	while(1)
	{
		char buf[256];
		memset(&buf, 0, sizeof(buf));
		udp_clilen = sizeof(udp_cli_addr);
		if ((numbytes = recvfrom(udp_sockfd, buf, 255 , 0, (struct sockaddr *)&udp_cli_addr, &udp_clilen)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}
		if (numbytes > 0)
		{
			char *temp;
			temp = strrchr(buf,'#');
			temp++;
			if (temp[0] == 'A')
			{
				if (printA_check != 1)
				{
					printA_check = 1;
					udp_sock_addr(udp_sockfd, temp[0]);
				}
			}
			else if (temp[0] == 'B')
			{
				if (printB_check != 1)
				{
					printB_check = 1;
					udp_sock_addr(udp_sockfd, temp[0]);
				}
			}
			printf("Student %c has been admitted to Department %c\n", buf[7], temp[0]);
			if (buf[7] == '5')
			{
				printf("End of Phase 2 for Department A\n");
				printf("End of Phase 2 for Department B\n");
			}
		}
	}
	close(udp_sockfd);
}

/* Connect to Admission and send program information */
void processData (int sockfd, FILE *fp, int loop, char *ip)
{
	int portno;
	struct addrinfo hints, *serv_addr;
	int n, return_val;
	char buffer[256];
	int i;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((return_val = getaddrinfo(ip, SERVER_PORT, &hints, &serv_addr)) != 0)
	{
		perror("getaddrinfo() error");
		exit(1);
	}
	if (connect(sockfd, serv_addr->ai_addr, serv_addr->ai_addrlen) == -1)
	{
		close(sockfd);
		perror("connect() error");
		exit(1);
	}
	if (loop == 0)
	{
		fp = fopen("departmentA.txt", "r");
		if (fp == NULL)
		{
			perror("Error in Department A file opening");
			exit(1);
		}
	}
	else
	{
		fp = fopen("departmentB.txt", "r");
		if (fp == NULL)
		{
			perror("Error in Department B file opening");
			exit(1);
		}
	}
	char buff[1024];
	size_t length = fread(buff, 1, sizeof(buff), fp);
	char *line = NULL;
	char buff_temp;
	line = strtok(buff, "\n");
	while (line != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		if (loop == 0)
		{
			strcat(buffer, "A#");
		}
		else if (loop == 1)
		{
			strcat(buffer, "B#");	
		}
		strcat(buffer, line);
		strcat(buffer, "\n");
		n = send(sockfd, buffer, strlen(buffer), 0);
		if (buff_temp != buffer[0])
		{
			sock_addr(sockfd, buffer[0]);
			printf("Department %c is now connected to the admission office\n", buffer[0]);
		}
		if (n < 0) 
		{
			perror("send() error");
			exit(1);
		}
		else
		{
			printf("Department %c has sent program %c%c to the admission office\n", buffer[0], buffer[2], buffer[3]);
		}
		buff_temp = buffer[0];
		line = strtok(NULL, "\n");
	}
	printf("Updating the admission office is done for Department %.1s\n", buffer);
	fclose(fp);
	close(sockfd);
	if (buffer[0] == 'B')
	{
		printf("End of phase 1 for Department B\n");
		int udp_pid, udp_pid_return;
		int ploop;
		for(ploop=0; ploop<DEPT_NUM; ploop++)
		{
			udp_pid = fork();
			if (udp_pid < 0)
			{
				perror("fork() error");
				exit(1);
			}
			if (udp_pid == 0)
			{
				processDeptResults(ip);
				exit(0);
			}
			else
			{
				waitpid(udp_pid, &udp_pid_return, 0);
				if (udp_pid_return != 0)
				{
					printf("Process terminted with an error\n");
				}
			}
		}	
	}
}
