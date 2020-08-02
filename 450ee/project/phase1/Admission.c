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

#define SERVER_PORT "13197"
#define BACKLOG      10
#define DEPT_NUM      2
#define PROG_NUM      3
#define NUNKI_SERVER "nunki.usc.edu"

/* Function declarations */
void processData(int, int, char* [], FILE*);
void setupDatabase(char*, char* [], FILE*);
void sock_addr(int, char*);

/* Obtain socket information using getsockname function */
void sock_addr(int s, char *ip)
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
	snprintf(buf,bufsiz, "The admission office has TCP port %u and IP address %s", (unsigned)ntohs(adr_inet.sin_port), ip);
	printf("%s\n", buf);
}

/* Manipulate received data from client and store it in a file */
void setupDatabase(char buffer[256], char *data[], FILE* fp)
{
	char buffer_copy[256];
	strcpy(buffer_copy,buffer);
	char *start_ptr = buffer_copy;
	char *tab_ptr = strchr(start_ptr, '\n');
	if (tab_ptr != NULL)
	{
		*tab_ptr++ = '\0';
		fprintf(fp, "%s\n", start_ptr);
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\n');
		if (tab_ptr != NULL)
		{
			*tab_ptr++ = '\0';
			fprintf(fp, "%s\n", start_ptr);
			start_ptr = tab_ptr;
			tab_ptr = strchr(start_ptr, '\n');
			if (tab_ptr != NULL)
			{
				*tab_ptr++ = '\0';
				fprintf(fp, "%s\n", start_ptr);
			}
		}
	}
}

/* Get messages through the socket */
void processData (int sock, int count_temp,char *data[], FILE* fp)
{
	int n;
	char buffer[256];
	int i;
	for(i=0; i<DEPT_NUM; i++)
	{
		memset(buffer, 0, sizeof(buffer));
		n = read(sock,buffer,255);
		if (n < 0)
		{
			perror("ERROR reading from socket");
			exit(1);
		}
		if (n > 0)
		{
			printf("Received the program list from Department %c\n", buffer[0]);
			setupDatabase(buffer, data, fp);
			if (buffer[0] == 'B')
			{
				printf("End of Phase 1 for the admission office\n");
			}
		}
	}
}

/* Create socket to obtain data from Departments */

int main(int argc, char *argv[])
{
	FILE *fp = fopen("database.txt", "w");
	if (fp == NULL)
	{
		perror("File write error");
		exit(1);
	}
	int sockfd, newsockfd;
	int pid, pid_return;
	struct addrinfo hints, *serv_addr, cli_addr;
	int yes=1;
	socklen_t clilen;
	int return_val;
	int count_temp = 0;
	char *adOffice_database[6];

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

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((return_val = getaddrinfo(NULL, SERVER_PORT, &hints, &serv_addr)) != 0)
	{
		perror("getaddrinfo() error");
		exit(1);
		return 1;
	}
	if ((sockfd = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol)) == -1 )
	{
		perror("socket opening error");
		exit(1);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt() error");
		exit(1);
	}
	if (bind(sockfd, serv_addr->ai_addr, serv_addr->ai_addrlen) == -1)
	{
		close(sockfd);
		perror("bind() error");
		exit(1);
	}
	sock_addr(sockfd, ip);
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen() error");
		exit(1);
	}
	clilen = sizeof(cli_addr);
	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
		{ 
			perror("accept() error");
			exit(1);
		}
		pid = fork();
		if (pid < 0)
		{
			perror("fork() error");
			exit(1);
		}
		if (pid == 0)
		{
			close(sockfd);
			processData(newsockfd, count_temp, adOffice_database, fp);
			exit(0);
		}
		else
		{
			waitpid(pid, &pid_return, 0);
			if (pid_return != 0)
			{
				printf("Process terminted with an error\n");
			}
			close(newsockfd);
		}
	}
	fclose(fp);
	return 0;
}
