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
#define BACKLOG      10
#define DEPT_NUM      2
#define PROG_NUM      3
#define STUD_NUM      5
#define NUNKI_SERVER "nunki.usc.edu"
#define UDP_STUDENT  "22297"

/* Obtain socket information using getsockname function */
void sock_addr(int s, char student)
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
	snprintf(buf,bufsiz, "Student %c has TCP port %u and IP address %s", student, (unsigned)ntohs(adr_inet.sin_port), inet_ntoa(adr_inet.sin_addr));
	printf("%s\n", buf);
}

/* Obtain socket information using getsockname function */
void udp_sock_addr(int s, int student)
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
	snprintf(buf,bufsiz, "Student %d has UDP port %u and IP address %s for Phase 2", student, (unsigned)ntohs(adr_inet.sin_port), inet_ntoa(adr_inet.sin_addr));
	printf("%s\n", buf);
}

/* Connect to Admission and send program information */
unsigned int processData (int sockfd, FILE *fp, int loop, char *ip)
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
	switch(loop)
	{
		case 0:
			fp = fopen("student1.txt", "r");
			if (fp == NULL)
			{
				perror("Error in Student 1 file opening");
				exit(1);
			}
			break;
		case 1:
			fp = fopen("student2.txt", "r");
			if (fp == NULL)
			{
				perror("Error in Student 2 file opening");
				exit(1);
			}
			break;
		case 2:
			fp = fopen("student3.txt", "r");
			if (fp == NULL)
			{
				perror("Error in Student 3 file opening");
				exit(1);
			}
			break;
		case 3:
			fp = fopen("student4.txt", "r");
			if (fp == NULL)
			{
				perror("Error in Student 4 file opening");
				exit(1);
			}
			break;
		case 4:
			fp = fopen("student5.txt", "r");
			if (fp == NULL)
			{
				perror("Error in Student 5 file opening");
				exit(1);
			}
			break;
		default:
			perror("Bad input file");
			exit(1);
	}

	char buff[1024];
	size_t length = fread(buff, 1, sizeof(buff), fp);
	char *line = NULL;
	char buff_temp;
	line = strtok(buff, "\n");
	while (line != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		switch(loop)
		{
			case 0:
				strcat(buffer, "Student1#");
				break;
			case 1:
				strcat(buffer, "Student2#");
				break;
			case 2:
				strcat(buffer, "Student3#");
				break;
			case 3:
				strcat(buffer, "Student4#");
				break;
			case 4:
				strcat(buffer, "Student5#");
				break;
			default:
				perror("Bad input file");
				exit(1);
		}

		strcat(buffer, line);
		strcat(buffer, "\n");
		n = send(sockfd, buffer, strlen(buffer), 0);
		if (buff_temp != buffer[7])
		{
			sock_addr(sockfd, buffer[7]);
		// 	// printf("Student %c is now connected to the admission office\n", buffer[0]);
		}
		if (n < 0) 
		{
			perror("send() error");
			exit(1);
		}
		buff_temp = buffer[7];
		line = strtok(NULL, "\n");
	}
	printf("Completed sending application for Student %c\n", buff_temp);
	fclose(fp);
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

	for (loop = 0; loop < STUD_NUM; loop++)
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
			if (pid_return != 0)
			{
				printf("Process terminated with an error\n");
			}
		}
	}

/* UDP Socket to receive data from Admission office */
	int stud_count = 0;
	while(1)
	{
		stud_count++;
		int udp_sockfd, udp_newsockfd;
		struct addrinfo udp_hints, *udp_serv_addr, udp_cli_addr;
		socklen_t udp_clilen;
		int yes = 1;
		int return_val;
		int numbytes;
		memset(&udp_hints, 0, sizeof(udp_hints));
		udp_hints.ai_family = AF_INET;
		udp_hints.ai_socktype = SOCK_DGRAM;
		udp_hints.ai_flags = AI_PASSIVE;
		if ((return_val = getaddrinfo(ip, UDP_STUDENT, &udp_hints, &udp_serv_addr)) != 0)
		{
			perror("udp getaddrinfo() error");
			exit(1);
			return 1;
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
		int buf[256];
		memset(&buf, 0, sizeof(buf));
		udp_clilen = sizeof(udp_cli_addr);
		if ((numbytes = recvfrom(udp_sockfd, buf, 255 , 0, (struct sockaddr *)&udp_cli_addr, &udp_clilen)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}
		printf("Student %d has received the reply from the admission office\n", stud_count);
		udp_sock_addr(udp_sockfd, stud_count);
		printf("Student %d has received the application result\n", stud_count);
		printf("End of Phase 2 for Student %d\n", stud_count);
		close(udp_sockfd);
	}
	return 0;
}
