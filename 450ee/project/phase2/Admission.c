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

#define SERVER_PORT "4197"
#define BACKLOG      10
#define DEPT_NUM      2
#define PROG_NUM      3
#define STUD_NUM      5
#define UDP_STUDENT  "22297"
#define UDP_DEPT     "22097"
#define NUNKI_SERVER "nunki.usc.edu"

struct student
{
	char stud_id;
	char gpa[6];
	char interest1[4];
	char interest2[4];
};

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

/* Obtain socket information for UDP using getsockname function */
void udp_sock_addr(int s, char *ip)
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
	snprintf(buf,bufsiz, "The admission office has UDP port %u and IP address %s for Phase 2", (unsigned)ntohs(adr_inet.sin_port), ip);
	printf("%s\n", buf);
}

/* Manipulate received data from client and store it in a file */
void setupDatabase(char buffer[256], FILE* fp)
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
void processData (int sock, int count_temp, FILE* fp)
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
			setupDatabase(buffer, fp);
			if (buffer[0] == 'B')
			{
				printf("End of Phase 1 for the admission office\n");
			}
		}
	}
}

char getProgramMatch(char *gpa, char *program, char *fileBuff, char* stud_res, char prog_match)
{
	char fileBuff_copy[1024];
	strcpy(fileBuff_copy,fileBuff);
	char* line = NULL;
	line = strtok(fileBuff_copy, "\n");
	while(line != NULL)
	{
		char line_copy[256];
		strcpy(line_copy,line);
		char *start_ptr = line_copy;
		char *tab_ptr = strchr(start_ptr, '#');
		if (tab_ptr != NULL)
		{
			*tab_ptr++ = '\0';
			start_ptr = tab_ptr;
			tab_ptr = strchr(start_ptr, '#');
			if (tab_ptr != NULL)
			{
				*tab_ptr++ = '\0';
				if (strcmp(program, start_ptr) == 0)
				{
					prog_match = 'y';
					float student_gpa = atof(gpa);
					float temp_gpa = atof(tab_ptr);
					if (student_gpa >= temp_gpa)
					{
						strcat(stud_res, "Accept#");
						strcat(stud_res, start_ptr);
						strcat(stud_res, "#department");
						stud_res[strlen(stud_res)] = start_ptr[0];
						stud_res[strlen(stud_res)+1] = '\0';
						break;
					}
				}
			}
		}
		line = strtok(NULL, "\n");
	}
	return prog_match;
}

void sendDecisionToStudent(char *stud_res, char *ip, char student_name)
{
	struct addrinfo udp_hints, *udp_serv_addr;
	int return_val;
	int udp_sockfd;
	int numbytes;
	memset(&udp_hints, 0, sizeof(udp_hints));
	udp_hints.ai_family = AF_INET;
	udp_hints.ai_socktype = SOCK_DGRAM;
	udp_hints.ai_flags = AI_PASSIVE;
	if ((return_val = getaddrinfo(ip, UDP_STUDENT, &udp_hints, &udp_serv_addr)) != 0)
	{
		perror("getaddrinfo() error");
		exit(1);
	}
	if ((udp_sockfd = socket(udp_serv_addr->ai_family, udp_serv_addr->ai_socktype, udp_serv_addr->ai_protocol)) == -1)
	{
		perror("socket opening error");
		exit(1);
	}
	if ((numbytes = sendto(udp_sockfd, stud_res, strlen(stud_res), 0, udp_serv_addr->ai_addr, udp_serv_addr->ai_addrlen)) == -1)
	{
		perror("sendto() error");
		exit(1);
	}
	printf("The admission office has sent the application result to Student %c\n", student_name);
	freeaddrinfo(udp_serv_addr);
	close(udp_sockfd);
}

void sendDecisionToDepartment(char *dept_res, char *ip)
{
	struct addrinfo udp_hints, *udp_serv_addr;
	int return_val;
	int udp_sockfd;
	int numbytes;
	memset(&udp_hints, 0, sizeof(udp_hints));
	udp_hints.ai_family = AF_INET;
	udp_hints.ai_socktype = SOCK_DGRAM;
	udp_hints.ai_flags = AI_PASSIVE;
	if ((return_val = getaddrinfo(ip, UDP_DEPT, &udp_hints, &udp_serv_addr)) != 0)
	{
		perror("getaddrinfo() error");
		exit(1);
	}
	if ((udp_sockfd = socket(udp_serv_addr->ai_family, udp_serv_addr->ai_socktype, udp_serv_addr->ai_protocol)) == -1)
	{
		perror("socket opening error");
		exit(1);
	}
	if ((numbytes = sendto(udp_sockfd, dept_res, strlen(dept_res), 0, udp_serv_addr->ai_addr, udp_serv_addr->ai_addrlen)) == -1)
	{
		perror("sendto() error");
		exit(1);
	}
	if (strlen(dept_res) > 0)
	{
		char *sent_dept;
		sent_dept = strrchr(dept_res,'#');
		sent_dept++;
		printf("The admission office has send one admitted student to Department %c\n", sent_dept[0]);
	}
	freeaddrinfo(udp_serv_addr);
	close(udp_sockfd);
}

void evaluateStudent(struct student *stud_info, char* fileBuff, char *ip)
{
	char stud_res[256];
	char dept_res[256];
	char prog_match = 'n';
	memset(&stud_res, 0, sizeof(stud_res));
	memset(&dept_res, 0, sizeof(dept_res));
	prog_match = getProgramMatch(stud_info->gpa, stud_info->interest1, fileBuff, stud_res, prog_match);
	if (strlen(stud_res) > 0)
	{
		strcat(dept_res, "Student");
		dept_res[strlen(dept_res)] = stud_info->stud_id;
		dept_res[strlen(dept_res)+1] = '\0';
		strcat(dept_res, "#");
		strcat(dept_res, stud_info->gpa);
		strcat(dept_res, "#");
		strcat(dept_res, stud_info->interest1);
	}
	else
	{
		prog_match = getProgramMatch(stud_info->gpa, stud_info->interest2, fileBuff, stud_res, prog_match);
		if (strlen(stud_res) > 0)
		{
			strcat(dept_res, "Student");
			dept_res[strlen(dept_res)] = stud_info->stud_id;
			dept_res[strlen(dept_res)+1] = '\0';
			strcat(dept_res, "#");
			strcat(dept_res, stud_info->gpa);
			strcat(dept_res, "#");
			strcat(dept_res, stud_info->interest2);
		}
		else
		{
			if (strlen(stud_res) == 0 and prog_match == 'y')
			{
				strcat(stud_res, "Reject");
			}
			else
			{
				strcat(stud_res, "0");
			}
		}
	}
	sendDecisionToStudent(stud_res, ip, stud_info->stud_id);
	sendDecisionToDepartment(dept_res, ip);
	if (stud_info->stud_id == '5')
	{
		printf("End of Phase 2 for the admission office\n");
	}
}

/* Manipulate received data from client and store it in a file */
void gradeStudent(char buffer[256], char* fileBuff, char *ip)
{
	char buffer_copy[256];
	strcpy(buffer_copy, buffer);
	struct student stud_info;
	char *start_ptr = buffer_copy;
	char *tab_ptr = strchr(start_ptr, '\n');
	if (tab_ptr != NULL)
	{
		*tab_ptr++ = '\0';
		stud_info.stud_id = start_ptr[7];
		printf("Admission office received the application from Student %c\n", stud_info.stud_id);
		char *temp_gpa;
		temp_gpa = strrchr(start_ptr,'#');
		strcpy(stud_info.gpa, temp_gpa+1);
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\n');
		if (tab_ptr != NULL)
		{
			*tab_ptr++ = '\0';
			char *temp_int1;
			temp_int1 = strrchr(start_ptr,'#');
			strcpy(stud_info.interest1, temp_int1+1);
			start_ptr = tab_ptr;
			tab_ptr = strchr(start_ptr, '\n');
			if (tab_ptr != NULL)
			{
				*tab_ptr++ = '\0';
				char *temp_int2;
				temp_int2 = strrchr(start_ptr,'#');
				strcpy(stud_info.interest2,temp_int2+1);
				evaluateStudent(&stud_info, fileBuff, ip);
			}
		}
	}
}

/* Get messages through the socket */
void processStudentData (int sock, int count_temp, char* fileBuff, char *ip)
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
			gradeStudent(buffer, fileBuff, ip);
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
	int ploop;
	for(ploop=0; ploop<DEPT_NUM; ploop++)
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
			processData(newsockfd, count_temp, fp);
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
	sleep(15);
	fp = fopen("database.txt", "r");
	if (fp == NULL)
	{
		perror("File opening error");
		exit(0);
	}
	char fileBuff[1024];
	size_t length = fread(fileBuff, 1, sizeof(fileBuff), fp);
	udp_sock_addr(sockfd, ip);
	while(1)
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
			processStudentData(newsockfd, count_temp, fileBuff, ip);
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
