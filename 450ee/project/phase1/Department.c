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

#define SERVER_PORT  "13197"
#define DEPT_NUM      2
#define PROG_NUM      3
#define NUNKI_SERVER "nunki.usc.edu"

/* Function declarations */
void processData(int, FILE *, int, char*);
void sock_addr(int, char);

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
}
