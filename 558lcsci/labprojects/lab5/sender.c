#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#define DATAGRAM_SIZE 1400

typedef struct input_arguments {
        int seq_num;
        char data[DATAGRAM_SIZE];
        int file_size;
} datagram;

pthread_mutex_t datagram_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ack_mutex = PTHREAD_MUTEX_INITIALIZER;

datagram datagram_recv, datagram_sent;

unsigned char *data_array;
struct stat buffer;
int packet_count=0;
int ack_count=0;
int *datagram_status;
pthread_t sender_thread, ack_thread;

int datagram_total=0;

struct timeval start, end;

int *retransmit_status;
int *retransmit_status1;

char set_flag = 'n';

FILE *fp;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *SendData(void *inputargv)
{
    char **argvar = (char**)inputargv;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socklen_t client_len;
    portno = atoi(argvar[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server = gethostbyname(argvar[1]);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    client_len= sizeof(serv_addr);
    int packet_num = 0;
    packet_count = buffer.st_size / DATAGRAM_SIZE;
    if (buffer.st_size % DATAGRAM_SIZE > 0)
    {
        packet_count++;
    }
    datagram_status=(int*)calloc(packet_count, sizeof(int));
    memset(datagram_status , 0 , packet_count*sizeof(int));
    do
    {
        memset(&datagram_sent , 0 , sizeof(datagram_sent));
        datagram_sent.seq_num = packet_num;
        if (buffer.st_size - (packet_num * DATAGRAM_SIZE) > DATAGRAM_SIZE)
        {
            memcpy( datagram_sent.data , &data_array[packet_num * DATAGRAM_SIZE] , DATAGRAM_SIZE);
        }
        else
        {
            memcpy( datagram_sent.data , &data_array[packet_num * DATAGRAM_SIZE] , buffer.st_size - (packet_num * DATAGRAM_SIZE));
        }
        datagram_sent.file_size = buffer.st_size;
        n = sendto(sockfd, &datagram_sent, sizeof(datagram_sent),0,(struct sockaddr *)&serv_addr, (socklen_t)client_len);
        packet_num++;
    }while(packet_num < packet_count);

/* Retransmission of data*/
    pthread_mutex_lock(&ack_mutex);
    if (ack_count < packet_count)
    {
        pthread_mutex_unlock(&ack_mutex);
        usleep(400);
        int retransmit_count=0;
        retransmit_status = (int*)calloc(packet_count, sizeof(int));
        int i;
        for (i = 0; i < packet_count; ++i)
        {
            pthread_mutex_lock(&datagram_mutex);
            if (datagram_status[i] == 0)
            {
                pthread_mutex_unlock(&datagram_mutex);
                retransmit_status[retransmit_count] = datagram_sent.seq_num;
                retransmit_count++;
                datagram_sent.seq_num = i;
                if (buffer.st_size - (i * DATAGRAM_SIZE) > DATAGRAM_SIZE)
                {
                    memcpy( datagram_sent.data , &data_array[i * DATAGRAM_SIZE] , DATAGRAM_SIZE);
                }
                else
                {
                    memcpy( datagram_sent.data , &data_array[i * DATAGRAM_SIZE] , buffer.st_size - (i * DATAGRAM_SIZE));
                }
                datagram_sent.file_size = buffer.st_size;
                n = sendto(sockfd, &datagram_sent, sizeof(datagram_sent),0,(struct sockaddr *)&serv_addr, (socklen_t)client_len);
            }
            else
            {
                pthread_mutex_unlock(&datagram_mutex);
            }
        }
        retransmit_status1 = (int*)calloc(retransmit_count, sizeof(int));
        set_flag = 'y';
        int iterator = retransmit_count;
        while(1)
        {
            usleep(300);
            int k=0;
            pthread_mutex_lock(&ack_mutex);
            if (ack_count < packet_count)
            {
                pthread_mutex_unlock(&ack_mutex);
                int i;
                for (i = 0; i < iterator; ++i)
                {
                    pthread_mutex_lock(&datagram_mutex);
                    if (datagram_status[retransmit_status[i]] == 0)
                    {
                        pthread_mutex_unlock(&datagram_mutex);
                        retransmit_status1[k] = retransmit_status[i];
                        datagram_sent.seq_num = retransmit_status1[k];
                        if (buffer.st_size - (retransmit_status[i] * DATAGRAM_SIZE) > DATAGRAM_SIZE)
                        {
                            memcpy( datagram_sent.data , &data_array[retransmit_status[i] * DATAGRAM_SIZE] , DATAGRAM_SIZE);
                        }
                        else
                        {
                            memcpy( datagram_sent.data , &data_array[retransmit_status[i] * DATAGRAM_SIZE] , buffer.st_size - (retransmit_status[i] * DATAGRAM_SIZE));
                        }
                        datagram_sent.file_size = buffer.st_size;
                        n = sendto(sockfd, &datagram_sent, sizeof(datagram_sent),0,(struct sockaddr *)&serv_addr, (socklen_t)client_len);
                        k++;
                    }
                    else
                    {
                        pthread_mutex_unlock(&datagram_mutex);
                    }
                }
                free(retransmit_status);
                retransmit_status = (int*)calloc(k, sizeof(int));
                memcpy( retransmit_status , retransmit_status1 , k * sizeof(int));
                free(retransmit_status1);
                retransmit_status1 = (int*)calloc(k, sizeof(int));
                iterator = k;
            }
            else
            {
                pthread_mutex_unlock(&ack_mutex);
                break;
            }
        }
    }
    pthread_mutex_unlock(&ack_mutex);
    close(sockfd);
    pthread_exit(0);
    return 0;
}

void *ReceiveAck(void *inputargv)
{
    char **argvar = (char**)inputargv;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    int dg_ack;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, "5555", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        // return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
        perror("listener: socket");
        continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;
    while(1)
    {
        if ((numbytes = recvfrom(sockfd, &dg_ack, sizeof(int) , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }
        if (dg_ack == 99999999)
        {
            break;
        }
        pthread_mutex_lock(&datagram_mutex);
        pthread_mutex_lock(&ack_mutex);
        if (datagram_status[dg_ack] != 1)
        {
            datagram_status[dg_ack]=1;
            ack_count++;
        }
        pthread_mutex_unlock(&ack_mutex);
        pthread_mutex_unlock(&datagram_mutex);
    }

    close(sockfd);
    pthread_cancel(sender_thread);
    pthread_exit(0);
    return 0;
}

int main(int argc, char *argv[])
{
    int file_des;
    int page_size;
    file_des = open(argv[3],O_RDONLY);
    if (file_des < 0)
    {
    fprintf(stderr,"Error: Unable to read dictionary file\n");
    return 0;
    }
    if (fstat(file_des,&buffer) < 0)
    {
    fprintf(stderr,"Error: Unable to determine file size\n");
    return 0;
    }
    unsigned int len;
    len = (unsigned int)buffer.st_size;
    data_array = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,file_des,0);
    page_size = getpagesize();
    pthread_create(&sender_thread, 0, SendData, (void *)argv);
    pthread_create(&ack_thread, 0, ReceiveAck, (void *)argv);
    pthread_join(ack_thread, 0);
    pthread_join(sender_thread, 0);
    free(datagram_status);
    if (set_flag == 'y')
    {
        free(retransmit_status);
        free(retransmit_status1);
    }
    munmap(data_array,buffer.st_size);
//Receiving the data from receiver
    fp = fopen("output_sender.bin", "wb");
    int sockfd, newsockfd, portno;
    socklen_t clilen,server_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi("7777");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    clilen = sizeof(cli_addr);
    server_len= sizeof(serv_addr);
    int buff_alloc_flag = 0;
    int tot_count = 0;
    int once = 0;
    char *file_array;
    char end_flag = 'n';
    while(1)
    {
        memset(&datagram_recv , 0 , sizeof(datagram_recv));
        serv_addr.sin_port = htons(portno);
        if (end_flag == 'n')
        {
            n = recvfrom(sockfd, &datagram_recv, sizeof(datagram_recv) ,0,(struct sockaddr *) &serv_addr, &server_len);
            if (buff_alloc_flag == 0)
            {
                float check = (float)datagram_recv.file_size/(float)DATAGRAM_SIZE;
                tot_count = ceil(check);
                datagram_status=(int*)malloc(tot_count * sizeof(int));
                memset(datagram_status , 0 , tot_count * sizeof(int));
                file_array=(char*)malloc(datagram_recv.file_size);
                buff_alloc_flag = 1;
            }
            if (datagram_status[datagram_recv.seq_num] != 1)
            {
                datagram_status[datagram_recv.seq_num] = 1;
                datagram_total++;
                printf("Datagram count %d\n", datagram_total);
                if (datagram_recv.file_size - ((datagram_recv.seq_num) * DATAGRAM_SIZE) < DATAGRAM_SIZE)
                {
                    memcpy( &file_array[datagram_recv.seq_num * DATAGRAM_SIZE] , datagram_recv.data , datagram_recv.file_size - ((datagram_recv.seq_num) * DATAGRAM_SIZE));
                }
                else
                {
                    memcpy( &file_array[datagram_recv.seq_num * DATAGRAM_SIZE] , datagram_recv.data , DATAGRAM_SIZE);
                }

            }
        }
        serv_addr.sin_port = htons(8888);
        if (end_flag == 'y')
        {
            datagram_recv.seq_num = 99999999;
            int i;
            for (i = 0; i < 20; ++i)
            {
                n = sendto(sockfd, &datagram_recv.seq_num, sizeof(int),0,(struct sockaddr *) &serv_addr, (socklen_t)server_len);
            }
            break;
        }
        else
        {
            n = sendto(sockfd, &datagram_recv.seq_num, sizeof(int),0,(struct sockaddr *) &serv_addr, (socklen_t)server_len);
        }
        if (datagram_total == tot_count && once == 0)
        {
            once = 1;
            fwrite(file_array, datagram_recv.file_size, 1,fp);
            fflush(fp);
            fclose(fp);
            end_flag = 'y';
        }
    }
    close(sockfd);
    return 0;
}
