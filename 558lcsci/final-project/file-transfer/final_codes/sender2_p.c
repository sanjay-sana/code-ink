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
#include <time.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>


#include "header.h"
#include "custom.h"
#define FLAG 10
/*
TODO 11/27 ONLY SENDER 2 IS UPDATED
*/
//sender 2 
//TODO 11/27 upadate for sender1 as well
#define H1_TYPE_SENDER2 FT_PACKET_TYPE_R1B
#define H2_TYPE_SENDER2 FT_PACKET_TYPE_R2A
#define H3_TYPE_SENDER2 FT_PACKET_TYPE_N3

#define H1_FROM_NODE_SENDER2 N2
#define H1_TO_NODE_SENDER2 N3

#define H2_FROM_NODE_SENDER2 N2
#define H2_TO_NODE_SENDER2 N3


#define H3_FROM_NODE_SENDER2 N2
#define H3_TO_NODE_SENDER2 N3


// #define FILENAME1 "output.bin1"
// #define FILENAME2 "output.bin2"

/****MUST BE GLOBAL**********/
struct stat buffer;
typedef struct input_arguments {
        char data[data_len];
} datagram;
struct timeval start, end;
time_t now;

struct sockaddr_ll saddr,raddr;
/***************************/

/**************SENDER 1*******************/
packet_format my_framesent1;
unsigned char *data_array1;
int packet_count1=0;
int ack_count1=0;
int *retransmit1_status;
int *retransmit1_status1;
char set_flag1 = 'n';
int *datagram_status_send1;/*also used by receivers*/

pthread_mutex_t datagram_mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ack_mutex1 = PTHREAD_MUTEX_INITIALIZER;

char filename1[20];
char filename2[20];
/**************SENDER 1*******************/

/**************SENDER 2*******************/
packet_format my_framesent2;
unsigned char *data_array2;
int packet_count2=0;
int ack_count2=0;
int *retransmit2_status;
int *retransmit2_status1;
char set_flag2 = 'n';
int *datagram_status_send2;/*also used by receivers*/

pthread_mutex_t datagram_mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ack_mutex2 = PTHREAD_MUTEX_INITIALIZER;

/**************SENDER 2*******************/


/**************RECEIVER 1*************/
int *datagram_status_recv1;/*also used by receivers*/
int datagram_total_recv1=0;
FILE *fp1;
/***********************************/
/**************RECEIVER 1*************/
int *datagram_status_recv2;/*also used by receivers*/
int datagram_total_recv2=0;
FILE *fp2;
/***********************************/


pthread_t sender_thread1, ack_thread1,recv_thread1;

pthread_t sender_thread2, ack_thread2,recv_thread2;

/*************parse the cmd line to fill the below****************/
char*interface_name;
char*action;
char node_names[2][50];
int num_nodes;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void read_keys(){}
unsigned char iv[AES_BLOCK_SIZE];
unsigned char *key;

/*****************************************************************************************/
unsigned char *dec_out;
unsigned char* AESencrypt(unsigned char* aes_input, unsigned char aes_key[],int size, unsigned char *enc_out)
{
    unsigned char iv_enc[AES_BLOCK_SIZE];
    int keylength=128;
    
    AES_KEY enc_key;
      memset(iv_enc,0x00,AES_BLOCK_SIZE);

    const size_t encslength = ((strlen(aes_input) + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
 //   unsigned char *enc_out=(unsigned char*)malloc(frame_len*sizeof(unsigned char));
   memset(enc_out, 0, sizeof(enc_out));


    AES_set_encrypt_key(aes_key, keylength, &enc_key);
    AES_cbc_encrypt(aes_input,enc_out, size, &enc_key, iv_enc, AES_ENCRYPT);
  
    return enc_out;
   // return NULL;
}

unsigned char* AESdecrypt(unsigned char* aes_input,unsigned char aes_key[],int size){
    unsigned char iv_dec[AES_BLOCK_SIZE];
    int i;
    memset(iv_dec,0x00,AES_BLOCK_SIZE);
    int keylength=128;
    AES_KEY dec_key;
    memset(dec_out, 0, sizeof(frame_len));
    AES_set_decrypt_key(aes_key, keylength, &dec_key);
    AES_cbc_encrypt(aes_input, dec_out, size, &dec_key, iv_dec, AES_DECRYPT);
    return dec_out;

}

/*****************************************************************************************/
unsigned char *keyCalculate(int key){
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

void update_header(short int sender, short int receiver, int len, int no,packet_format * frame)
{
    frame->hdr.src_addr = sender;
    frame->hdr.dest_addr = receiver;
    frame->hdr.seq_no = no;
}

void attach_header(short int packet_type_arg,short int src_addr_arg,short int dest_addr_arg,short int padding2_arg,int seq_no_arg,int padding1_arg,unsigned char* packet_ptr)
{
        memcpy(packet_ptr,(unsigned char*)&packet_type_arg,2*sizeof(unsigned char));
        memcpy(packet_ptr+2,(unsigned char*)&src_addr_arg,2*sizeof(unsigned char));
        memcpy(packet_ptr+4,(unsigned char*)&dest_addr_arg,2*sizeof(unsigned char));
        memcpy(packet_ptr+6,(unsigned char*)&padding2_arg,2*sizeof(unsigned char));
        memcpy(packet_ptr+8,(unsigned char*)&seq_no_arg,4*sizeof(unsigned char));
        memcpy(packet_ptr+12,(unsigned char*)&padding1_arg,4*sizeof(unsigned char));
}

void parse_command_line_args(int count,char*argv[])
{
    action = argv[1];

}
/*********************************************************************************************************************************************/
/*********************************************************************************************************************************************/

void *SendData1(void *inputargv)
{
    char **argvar = (char**)inputargv;
    int sockfd, portno, n;
    struct sockaddr_ll saddr;
    //portno = atoi(argvar[2]);
    /********create your raw socket's file descriptor******************/
   
    int ifindex, bytes;
    struct ifreq ptr;
    short int proto = htons(0x1234);
    pthread_t sender,receiver;

    /***************************SOCKET CREATION***************************/ 
    if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
        handle_error("socket");
    printf ("main:  Socket created\n");
    /*********************FETCHING & UPDATING sockaddr_ll***********************/
    memset(&ptr,0, sizeof(ptr));
    strcpy(ptr.ifr_name,"eth4");
    if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
        printf ("main:  ioctl error\n");
        handle_error ("ioctl");
    }
    ifindex = ptr.ifr_ifindex;

    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sll_family = PF_PACKET;   
    saddr.sll_ifindex = ifindex;
    saddr.sll_halen = ETH_ALEN;


    /**************************BINDING SOCKET************************/
    if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
        printf ("main:  Bind error");
        handle_error("bind");
    }

/*****************************************************************/
    int packet_num = 0;

    packet_count1 = buffer.st_size / (data_len); //around 7 lakh
    if (buffer.st_size % (data_len) > 0)
    {
        packet_count1++;
    }
    datagram_status_send1=(int*)calloc(packet_count1, sizeof(int));
    memset(datagram_status_send1 , 0 , packet_count1*sizeof(int));
   
/*****************************************************************************************/
/*********/
    unsigned char* output_hdr1=(unsigned char*)malloc((HDR_SIZE)*sizeof(unsigned char));
    unsigned char* output_hdr2=(unsigned char*)malloc((HDR_SIZE)*sizeof(unsigned char));

    unsigned char* output_hdr2_encrypted=(unsigned char*)malloc((HDR_SIZE)*sizeof(unsigned char));


    unsigned char* output_data=(unsigned char*)malloc(data_len*sizeof(unsigned char));

    unsigned char* output_encrypted=(unsigned char*)malloc((frame_len)*sizeof(unsigned char));
    
    //FIX
    unsigned char *aes_key=(unsigned char*)malloc(AES_BLOCK_SIZE*sizeof(unsigned char));
    unsigned char *enc_out=(unsigned char*)malloc(file_size*sizeof(unsigned char));

    aes_key=keyCalculate(5);
/************/
    do
    {
        memset(&my_framesent1 , 0 , frame_len);

        if (buffer.st_size - (packet_num * (data_len)) > (data_len)) //last packet condition
        {
        
            memcpy( my_framesent1.data_inside , &data_array1[packet_num * (data_len)] , (data_len));
        }
        else
        {
            memcpy( my_framesent1.data_inside , &data_array1[packet_num * (data_len)] , buffer.st_size - (packet_num * (data_len)));
        }


        //hdr ,malloc 

        unsigned char* output_data_encrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));
        memset(output_data_encrypted,0,data_len);
        
        memset(output_hdr1,0,HDR_SIZE);
        memset(output_encrypted,0,frame_len);
        memset(output_data,0,data_len);
/*****************************************use this everywhere else FIX , update the recvr wiht new data len etc*************************/
                 memset(output_hdr2,0,HDR_SIZE);

        //create header 1 - FIX
        attach_header(FT_PACKET_TYPE_N2,N2,N1,0,packet_num,0,output_hdr1);

        //create header 2 - FIX
        attach_header(FT_PACKET_TYPE_N2,N2,N1,0,packet_num,0,output_hdr2);
        //DO NOT ENCRYPT THE OUTER HEADER and copy it
        memcpy(output_encrypted,output_hdr1,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

        //encrypt the next header
        output_hdr2_encrypted=AESencrypt(output_hdr2,aes_key,HDR_SIZE, enc_out);

        //now copy the next encrypted header
        memcpy(output_encrypted+HDR_SIZE,output_hdr2_encrypted,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

        //data
        memcpy(output_data,(unsigned char*)&my_framesent1.data_inside,data_len);

        //enc data
        output_data_encrypted=AESencrypt(output_data,aes_key,data_len, enc_out);

        memcpy(output_encrypted+HDR_SIZE+HDR_SIZE,output_data_encrypted,data_len);
/******************************************************************************************************/
        n = sendto(sockfd, output_encrypted, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));
        
        packet_num++;

        if(packet_num%2==0) //sleep once 1000 packets are sent
        usleep(10000);

} while(packet_num < packet_count1);


/********************************RETRANSMISSION  code starts********************************************/
   pthread_mutex_lock(&ack_mutex1);
    if (ack_count1 < packet_count1)
    {
        pthread_mutex_unlock(&ack_mutex1);
        
        int retransmit_count=0;
        retransmit1_status = (int*)calloc(packet_count1, sizeof(int));
        int i;
        for (i = 0; i < packet_count1; ++i)
        {
            pthread_mutex_lock(&datagram_mutex1);
            if (datagram_status_send1[i] == 0)
            {
                pthread_mutex_unlock(&datagram_mutex1);
                retransmit1_status[retransmit_count] = i;
                retransmit_count++;

                if (buffer.st_size - (i * data_len) > data_len)
                {
                    memcpy( my_framesent1.data_inside , &data_array1[i * data_len] , data_len);
                }
                else
                {

                    memcpy( my_framesent1.data_inside , &data_array1[i * data_len] , buffer.st_size - (i * data_len));
                }
                //datagram_sent.file_size = buffer.st_size;
                /* encrypt
                */

                unsigned char* output_data_encrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));
                memset(output_data_encrypted,0,data_len);
                
                memset(output_hdr1,0,HDR_SIZE);
                memset(output_encrypted,0,frame_len);
                memset(output_data,0,data_len);
/*****************************************use this everywhere else FIX , update the recvr wiht new data len etc*************************/
                memset(output_hdr2,0,HDR_SIZE);

        //create header 1 - FIX
        attach_header(H1_TYPE_SENDER2 ,N2,N1,0,i,0,output_hdr1);

        //create header 2 - FIX
        attach_header(FT_PACKET_TYPE_N2,N2,N1,0,i,0,output_hdr2);

        //DO NOT ENCRYPT THE OUTER HEADER and copy it
        memcpy(output_encrypted,output_hdr1,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

        //encrypt the next header
        output_hdr2_encrypted=AESencrypt(output_hdr2,aes_key,HDR_SIZE, enc_out);

        //now copy the next encrypted header
        memcpy(output_encrypted+HDR_SIZE,output_hdr2_encrypted,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

        //data
        memcpy(output_data,(unsigned char*)&my_framesent1.data_inside,data_len);

        //enc data
        output_data_encrypted=AESencrypt(output_data,aes_key,data_len, enc_out);

        memcpy(output_encrypted+HDR_SIZE+HDR_SIZE,output_data_encrypted,data_len);

/******************************************************************************************************/
                n = sendto(sockfd, output_encrypted, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));
                
            }
            else
            {
                pthread_mutex_unlock(&datagram_mutex1);
            }
        }
            //printf("retransmit status count %d\n",retransmit_count);            
            int x;
            for(x=0;x<retransmit_count;x++)
            {
                //printf(" %d ",retransmit1_status[x]);
            }
            //printf("\n");
        
       retransmit1_status1 = (int*)calloc(retransmit_count, sizeof(int));
        set_flag1 = 'y';
        int iterator = retransmit_count;
       while(1)
        {
            usleep(1000);
            int k=0;
            pthread_mutex_lock(&ack_mutex1);
            if (ack_count1 < packet_count1)
            {
                pthread_mutex_unlock(&ack_mutex1);
                int i;
                for (i = 0; i < iterator; ++i)
                {
                    pthread_mutex_lock(&datagram_mutex1);
                    if (datagram_status_send1[retransmit1_status[i]] == 0)
                    {
                        pthread_mutex_unlock(&datagram_mutex1);
                        retransmit1_status1[k] = retransmit1_status[i];
                        unsigned char* output_data_encrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));
                        memset(output_data_encrypted,0,data_len);
                        
                        memset(output_hdr1,0,HDR_SIZE);
                        memset(output_encrypted,0,frame_len);
                        memset(output_data,0,data_len);
        
                        // attach_header(21,N1,N3,0,retransmit1_status1[k],0,output_hdr1);
                        
                        if (buffer.st_size - (retransmit1_status[i] * data_len) > data_len)
                        {

                            memcpy( my_framesent1.data_inside , &data_array1[retransmit1_status[i] * data_len] , data_len);
                        }
                        else
                        {
                            memcpy( my_framesent1.data_inside , &data_array1[retransmit1_status[i] * data_len] , buffer.st_size - (retransmit1_status[i] * data_len));
                        }
/*****************************************use this everywhere else FIX , update the recvr wiht new data len etc*************************/
                        memset(output_hdr2,0,HDR_SIZE);

                        //create header 1 - FIX
                        attach_header(FT_PACKET_TYPE_N2,N2,N1,0,retransmit1_status1[k],0,output_hdr1);

                        //create header 2 - FIX
                        attach_header(FT_PACKET_TYPE_N2,N2,N1,0,retransmit1_status1[k],0,output_hdr2);

                        //DO NOT ENCRYPT THE OUTER HEADER and copy it
                        memcpy(output_encrypted,output_hdr1,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

                        //encrypt the next header
                        output_hdr2_encrypted=AESencrypt(output_hdr2,aes_key,HDR_SIZE, enc_out);

                        //now copy the next encrypted header
                        memcpy(output_encrypted+HDR_SIZE,output_hdr2_encrypted,HDR_SIZE);//FIX this-copying the hdr to encrypted hdr

                        //data
                        memcpy(output_data,(unsigned char*)&my_framesent1.data_inside,data_len);

                        //enc data
                        output_data_encrypted=AESencrypt(output_data,aes_key,data_len, enc_out);

                        memcpy(output_encrypted+HDR_SIZE+HDR_SIZE,output_data_encrypted,data_len);
/******************************************************************************************************/

                        n = sendto(sockfd, output_encrypted, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));

                        k++;
                    }
                    else
                    {
                        pthread_mutex_unlock(&datagram_mutex1);
                    }
                }
                free(retransmit1_status);
                retransmit1_status = (int*)calloc(k, sizeof(int));
                memcpy( retransmit1_status , retransmit1_status1 , k * sizeof(int));
                free(retransmit1_status1);
                retransmit1_status1 = (int*)calloc(k, sizeof(int));
                iterator = k;
            }
            else
            {
                pthread_mutex_unlock(&ack_mutex1);
                break;
            }
        }
    }
    pthread_mutex_unlock(&ack_mutex1);
    close(sockfd);
    pthread_exit(0);
    return 0;
}


void *ReceiveAck1(void *inputargv)
{
   int sockfd, portno, n;
    unsigned char*temp_buffer;
    struct sockaddr_ll saddr,raddr;
    socklen_t size = (socklen_t)(sizeof(struct sockaddr_ll));
    int i;

/********create your raw socket's file descriptor******************/
    int ifindex, bytes;
    struct ifreq ptr;
    short int proto = htons(0x1234);
    pthread_t sender,receiver;

    /***************************SOCKET CREATION***************************/ 
    if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
        handle_error("socket");
    printf ("main:  Socket created\n");
    /*********************FETCHING & UPDATING sockaddr_ll***********************/
    memset(&ptr,0, sizeof(ptr));
    strcpy(ptr.ifr_name,"eth4");
    if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
        printf ("main:  ioctl error\n");
        handle_error ("ioctl");
    }
    ifindex = ptr.ifr_ifindex;

    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sll_family = PF_PACKET;   
    saddr.sll_ifindex = ifindex;
    saddr.sll_halen = ETH_ALEN;
    saddr.sll_protocol = htons(ETH_P_ALL);


    /**************************BINDING SOCKET************************/
    if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
        printf ("main:  Bind error");
        handle_error("bind");
    }



    packet_format*temp_frame,*temp_ack_frame;
    header_format1*temp_hdr;
    temp_buffer = (unsigned char*)malloc(frame_len) ;
    short int addr;
    int recv_packet_seq_no=0;

    while(1)
    {   
            memset(temp_buffer,0,frame_len);
            if ((n = recvfrom(sockfd, temp_buffer, frame_len ,0,(struct sockaddr *) &raddr, &size)) == -1)
            {
            perror("recvfrom");
            exit(1);
            }
    
            recv_packet_seq_no=0;
            temp_hdr=(header_format1*)temp_buffer;
            if(((temp_hdr->seq_no<packet_count1&&temp_hdr->seq_no>-1) || temp_hdr->seq_no==99999999) && (temp_hdr->src_addr==N1)&&(temp_hdr->packet_type==ACK_PACKET))
            {
                if (temp_hdr->seq_no == 99999999)
                {
                 //printf("over");    
                    break;
                }
                pthread_mutex_lock(&datagram_mutex1);
                pthread_mutex_lock(&ack_mutex1);
                if (datagram_status_send1[temp_hdr->seq_no] != 1)
                {
                    datagram_status_send1[temp_hdr->seq_no]=1;
                    ack_count1++;
                   //printf(" ACKS RECVD%d ack count %d ",temp_hdr->seq_no,ack_count1);

                }

                pthread_mutex_unlock(&ack_mutex1);
                pthread_mutex_unlock(&datagram_mutex1);
            }
    }

    close(sockfd);
    pthread_cancel(ack_thread1);
    pthread_exit(0);
    return 0;
}

/*********************************************************************************************************************************************/
/*********************************************************************************************************************************************/


void *SendData2(void *inputargv)
{
    char **argvar = (char**)inputargv;
    int sockfd, portno, n;
    struct sockaddr_ll saddr;
   
    int ifindex, bytes;
    struct ifreq ptr;
    short int proto = htons(0x1234);
    pthread_t sender,receiver;

    /***************************SOCKET CREATION***************************/ 
    if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
        handle_error("socket");
    printf ("main:  Socket created\n");
    /*********************FETCHING & UPDATING sockaddr_ll***********************/
    memset(&ptr,0, sizeof(ptr));
    strcpy(ptr.ifr_name,"eth4");
    if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
        printf ("main:  ioctl error\n");
        handle_error ("ioctl");
    }
    ifindex = ptr.ifr_ifindex;

    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sll_family = PF_PACKET;   
    saddr.sll_ifindex = ifindex;
    saddr.sll_halen = ETH_ALEN;


    /**************************BINDING SOCKET************************/
    if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0)
    {
        printf ("main:  Bind error");
        handle_error("bind");
    }

/*****************************************************************/
    int packet_num = 0;

    packet_count2 = buffer.st_size / (data_len); //around 7 lakh
    if (buffer.st_size % (data_len) > 0)
    {
        packet_count2++;
    }
    datagram_status_send2=(int*)calloc(packet_count2, sizeof(int));
    memset(datagram_status_send2 , 0 , packet_count2*sizeof(int));
   
/****************************************************************my_framesent*************************/
/*********/

    unsigned char* output_hdr2_encrypted=(unsigned char*)malloc((HDR_SIZE)*sizeof(unsigned char));
    unsigned char* output_hdr3_encrypted=(unsigned char*)malloc((HDR_SIZE)*sizeof(unsigned char));


    unsigned char* output_data=(unsigned char*)malloc(data_len*sizeof(unsigned char));

    unsigned char* output_encrypted=(unsigned char*)malloc((frame_len)*sizeof(unsigned char));
    unsigned char* output_data_encrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));
    
    //FIX
    unsigned char *aes_key=(unsigned char*)malloc(AES_BLOCK_SIZE*sizeof(unsigned char));  
    unsigned char *enc_out=(unsigned char*)malloc(frame_len*sizeof(unsigned char));

    header_format1 *out_hdr1 = (header_format1 *)output_encrypted;
    header_format1 *out_hdr2 = (header_format1 *)malloc(sizeof(header_format1));
    header_format1 *out_hdr3 = (header_format1 *)malloc(sizeof(header_format1));


    out_hdr1->packet_type = H1_TYPE_SENDER2;
    out_hdr1->src_addr = H1_FROM_NODE_SENDER2;
    out_hdr1->dest_addr = H1_TO_NODE_SENDER2;

    out_hdr2->packet_type = H2_TYPE_SENDER2;
    out_hdr2->src_addr = H2_FROM_NODE_SENDER2;
    out_hdr2->dest_addr = H2_TO_NODE_SENDER2;


    out_hdr3->packet_type = H3_TYPE_SENDER2;
    out_hdr3->src_addr = H3_FROM_NODE_SENDER2;
    out_hdr3->dest_addr = H3_TO_NODE_SENDER2;

    printf("\n Before decyrtion of headers\n");
    printf("\n Header 1\n");
    printf("\npacket type %d\n",H1_TYPE_SENDER2);

    printf("\n Header 1\n");
    printf("\npacket type %d\n",H2_TYPE_SENDER2);

    printf("\n Header 1\n");
    printf("\npacket type %d\n",H3_TYPE_SENDER2);
    
    aes_key=keyCalculate(5);
/************/
    unsigned char*encrypted_file_memory=(unsigned char*)malloc(sizeof(unsigned char*)*file_size);
    unsigned char*packet_memory=(unsigned char*)malloc(sizeof(unsigned char*)*frame_len*packet_count2);
    unsigned char*file_memory=(unsigned char*)malloc(sizeof(unsigned char*)*file_size);

    memset(encrypted_file_memory,0,file_size);
    memset(file_memory,0,file_size);

    int x=0;


    EVP_CIPHER_CTX ctx_enc,ctx_dec;
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};
    
    int outlen1=data_len;
    int outlen2=HDR_SIZE;
    for(x=0;x<packet_count2;x++)
    {

                out_hdr1->seq_no=x;
        out_hdr2->seq_no=x;
        out_hdr3->seq_no=x;

      //  AESencrypt((unsigned char*)&data_array2[x*data_len],aes_key,data_len, enc_out)
      //  memcpy(encrypted_file_memory+x*data_len,EVP_EncryptUpdate(&ctx_enc, out, &outlen1, in, sizeof(in)),data_len);

        EVP_CIPHER_CTX ctx_enc,ctx_dec;
        unsigned char key[16] = {0};
            unsigned char iv[16] = {0};



        memcpy(packet_memory+x*frame_len,(unsigned char*)out_hdr1,HDR_SIZE);



        EVP_EncryptInit(&ctx_enc, EVP_aes_128_cbc(), key, iv);

        EVP_EncryptUpdate(&ctx_enc, packet_memory+x*frame_len+HDR_SIZE, &outlen2, (unsigned char*)out_hdr2, HDR_SIZE);



        EVP_EncryptInit(&ctx_enc, EVP_aes_128_cbc(), key, iv);

        EVP_EncryptUpdate(&ctx_enc, packet_memory+x*frame_len+HDR_SIZE+HDR_SIZE, &outlen2,(unsigned char*) out_hdr3, HDR_SIZE);


            EVP_EncryptInit(&ctx_enc, EVP_aes_128_cbc(), key, iv);

           EVP_EncryptUpdate(&ctx_enc, packet_memory+x*frame_len+HDR_SIZE+HDR_SIZE+HDR_SIZE, &outlen1, (unsigned char*)&data_array2[x*data_len], data_len);

           printf("\n data before encryption: hello \n");
    }



    time(&now);
    //printf("%s", ctime(&now));
/*
    for(x=0;x<packet_count2;x++)
    {


    EVP_CIPHER_CTX ctx_enc,ctx_dec;
    unsigned char key[16] = {0};
    unsigned char iv[16] = {0};

    }
*/
    
    retransmit2_status = (int*)calloc(packet_count2, sizeof(int));
    retransmit2_status1 = (int*)calloc(packet_count2, sizeof(int));
    do
    {


        printf("\ndata after decryption %s\n",packet_memory+packet_num*frame_len);

        n = sendto(sockfd, packet_memory+packet_num*frame_len, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));


        packet_num++;
      if(packet_num%200==0)
            usleep(50);

} while(packet_num < packet_count2);
    time(&now);
   // printf("Time after first transmission %s", ctime(&now));

usleep(1000);
/**************************************************************************************************/
   pthread_mutex_lock(&ack_mutex2);
   printf("Total count after first transmission = %d\n", packet_count2);
   printf("Ack count after first transmission= %d\n", ack_count2);
    if (ack_count2 < packet_count2)
    {
        pthread_mutex_unlock(&ack_mutex2);
        
        int retransmit_count=0;
        int i;
        for (i = 0; i < packet_count2; ++i)
        {
            pthread_mutex_lock(&datagram_mutex2);
            if (datagram_status_send2[i] == 0)
            {
                pthread_mutex_unlock(&datagram_mutex2);
                retransmit2_status[retransmit_count] = i;
                retransmit_count++;
                n = sendto(sockfd, packet_memory+i*frame_len, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));
                //usleep(1);
            if(i%5==0)
            usleep(2);

                
            }
            else
            {
                pthread_mutex_unlock(&datagram_mutex2);
            }
        }
        
        set_flag2 = 'y';
        int iterator = retransmit_count;
            time(&now);
   // printf("Time after first retransmission %s", ctime(&now));

       while(1)
        {
            int k=0;
            pthread_mutex_lock(&ack_mutex2);
            if (ack_count2 < packet_count2)
            {
                pthread_mutex_unlock(&ack_mutex2);
                int i;
                for (i = 0; i < iterator; ++i)
                {
                    pthread_mutex_lock(&datagram_mutex2);
                    if (datagram_status_send2[retransmit2_status[i]] == 0)
                    {
                        pthread_mutex_unlock(&datagram_mutex2);
                        retransmit2_status1[k] = retransmit2_status[i];
                        n = sendto(sockfd, packet_memory+retransmit2_status[i]*frame_len, frame_len,0,(struct sockaddr *)&saddr, sizeof(saddr));

                        k++;
                        usleep(1);
                    }
                    else
                    {
                        pthread_mutex_unlock(&datagram_mutex2);
                    }
                }
                memset(retransmit2_status, 0, packet_count2 * sizeof(int));
                memcpy( retransmit2_status , retransmit2_status1 , k * sizeof(int));
                iterator = k;
            }
            else
            {
                pthread_mutex_unlock(&ack_mutex2);
                break;
            }
        }
    }
    pthread_mutex_unlock(&ack_mutex2);
    close(sockfd);
    pthread_exit(0);
    return 0;
}


void *ReceiveAck2(void *inputargv)
{
   int sockfd, portno, n;
    unsigned char*temp_buffer;
    struct sockaddr_ll saddr,raddr;
    socklen_t size = (socklen_t)(sizeof(struct sockaddr_ll));
    int i;

/********create your raw socket's file descriptor******************/
    int ifindex, bytes;
    struct ifreq ptr;
    short int proto = htons(0x1234);
    pthread_t sender,receiver;

    /***************************SOCKET CREATION***************************/ 
    if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
        handle_error("socket");
    printf ("main:  Socket created\n");
    /*********************FETCHING & UPDATING sockaddr_ll***********************/
    memset(&ptr,0, sizeof(ptr));
    strcpy(ptr.ifr_name,"eth4");
    if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
        printf ("main:  ioctl error\n");
        handle_error ("ioctl");
    }
    ifindex = ptr.ifr_ifindex;

    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sll_family = PF_PACKET;   
    saddr.sll_ifindex = ifindex;
    saddr.sll_halen = ETH_ALEN;
    saddr.sll_protocol = htons(ETH_P_ALL);


    /**************************BINDING SOCKET************************/
    if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
        printf ("main:  Bind error");
        handle_error("bind");
    }



    packet_format*temp_frame,*temp_ack_frame;
    header_format1*temp_hdr;
    temp_buffer = (unsigned char*)malloc(frame_len) ;
    
    short int addr;
    
    int recv_packet_seq_no=0;
    int k=0,temp_seq_no;
    int*temp_seq_no_ptr;
    unsigned char*u_temp_seq_no=(unsigned char*)malloc(frame_len);
    while(1)
    {   

            memset(temp_buffer,0,frame_len);
            if ((n = recvfrom(sockfd, temp_buffer, frame_len ,0,(struct sockaddr *) &raddr, &size)) == -1)
            {
            perror("recvfrom");
            exit(1);
            }
    
            recv_packet_seq_no=0;
            temp_hdr=(header_format1*)temp_buffer;
            //printf("recv ack2%d\n",temp_hdr->seq_no);

            // if(((temp_hdr->seq_no<packet_count2&&temp_hdr->seq_no>-1) || temp_hdr->seq_no==99999999) && (temp_hdr->src_addr==N3)&&(temp_hdr->packet_type==ACK_R1_OUT))
            if(((temp_hdr->seq_no<packet_count2&&temp_hdr->seq_no>-1) || temp_hdr->seq_no==99999999 || temp_hdr->seq_no==66666666) && (temp_hdr->src_addr==N3)&&(temp_hdr->packet_type==ACK_PACKET_TYPE_R1B))
            {
                if (temp_hdr->seq_no == 99999999)
                {
                 //printf("over");    
                    break;
                }
                pthread_mutex_lock(&datagram_mutex2);
                pthread_mutex_lock(&ack_mutex2);
                //printf("\ncritical section\n" );
                /*
                if (datagram_status_send2[temp_hdr->seq_no] != 1)
                {
                    datagram_status_send2[temp_hdr->seq_no]=1;
                    ack_count2++;
                   //printf(" ACKS RECVD%d ack count %d ",temp_hdr->seq_no,ack_count2);

                }
                */
                //printf("receiving" );
                for ( k = 0; k < 207; ++k)
                {
                    /* code */
                    memcpy(u_temp_seq_no,k*6+temp_buffer+HDR_SIZE,6);

                    temp_seq_no_ptr=(int*)(u_temp_seq_no);
                    temp_seq_no=(*temp_seq_no_ptr);
                    //printf("\nseq%d\n",temp_seq_no );
                    //memset(u_temp_seq_no,0,frame_len);
                    if (datagram_status_send2[temp_seq_no] != 1)
                    {
                        datagram_status_send2[temp_seq_no]=1;
                        ack_count2++;
                       //printf(" ACKS RECVD%d ack count %d ",temp_seq_no,ack_count2);


                    }

                }
                pthread_mutex_unlock(&ack_mutex2);
                pthread_mutex_unlock(&datagram_mutex2);
            }
    }

    close(sockfd);
    pthread_cancel(sender_thread2);
    pthread_exit(0);
    return 0;
}


/*********************************************************************************************************************************************/
/*********************************************************************************************************************************************/

void *receiver1(void *inputargv)
{
    char *file_name1 = (char *)inputargv;

    printf("entered into function");
            fp1 = fopen(file_name1, "wb");
        int sockfd, newsockfd, portno;
        socklen_t clilen,server_len;
        struct sockaddr_in serv_addr, cli_addr;
        socklen_t size = (socklen_t)(sizeof(struct sockaddr_ll));

        //parse_command_line_args(argc,argv);
        /********create your raw socket's file descriptor******************/
        int ifindex, bytes;
        struct ifreq ptr;
        short int proto = htons(0x1234);
        pthread_t sender,receiver;

        /***************************SOCKET CREATION***************************/ 
        if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
            handle_error("socket");
        printf ("main:  Socket created\n");
        /*********************FETCHING & UPDATING sockaddr_ll***********************/
        memset(&ptr,0, sizeof(ptr));
        strcpy(ptr.ifr_name,"eth4");
        if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
            printf ("main:  ioctl error\n");
            handle_error ("ioctl");
        }
        ifindex = ptr.ifr_ifindex;

        memset((void*)&saddr, 0, sizeof(saddr));
        saddr.sll_family = PF_PACKET;   
        saddr.sll_ifindex = ifindex;
        saddr.sll_halen = ETH_ALEN;
        saddr.sll_protocol = htons(ETH_P_ALL);

        /**************************BINDING SOCKET************************/
        if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
            printf ("main:  Bind error");
            handle_error("bind");
        }


        /*****************************************************************/

        int buff_alloc_flag = 0;
        int tot_count = 0;
        int once = 0;
        char *file_array;
        char end_flag = 'n';
        int c=0;
        int i;
        int debug_count=0;
        int n;
        unsigned char *temp_buffer = (unsigned char*)malloc(frame_len*sizeof(unsigned char)) ;    
        unsigned char *aes_key=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        packet_format*temp_frame,*temp_ack_frame;
        header_format1*temp_hdr;
        short int source_address,dest_address;
        int recv_packet_seq_no;
        
        dec_out=(unsigned char*)malloc((frame_len-HDR_SIZE)*sizeof(unsigned char));

        unsigned char* output_hdr=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        unsigned char* output_data=(unsigned char*)malloc(data_len*sizeof(unsigned char));
        unsigned char* hdr_decrypted=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        unsigned char* data_decrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));

        aes_key=keyCalculate(5);
        
        while(1)
        {

    printf("entered into function");
 
            if (end_flag == 'n')
            {

                    //printf("got packet %d",debug_count);
                    debug_count++;

                    //receiving encrypted data
                    memset(temp_buffer,0,(frame_len-HDR_SIZE));
                    n = recvfrom(sockfd, temp_buffer, (frame_len-HDR_SIZE) ,0,(struct sockaddr *) &raddr, &size);
        
                    //splitting data and header and decrypting it
                    memset(output_hdr,0,HDR_SIZE);
                    memset(output_data,0,data_len);
                    memset(data_decrypted,0,HDR_SIZE);
                    
                    memcpy(output_hdr,temp_buffer,HDR_SIZE);
                    //hdr_decrypted=AESdecrypt(output_hdr,aes_key,HDR_SIZE);
                    memcpy(&source_address,output_hdr+2,2);
                    memcpy(&dest_address,output_hdr+4,2);               
                    memcpy(&recv_packet_seq_no,output_hdr+8,4);
    /*******************************************************************************************************************/
                    if (buff_alloc_flag == 0)
                    {
                            float check = (float)(file_size)/(float)(data_len);
                            tot_count = ceil(check);
                            datagram_status_recv1=(int*)malloc(tot_count * sizeof(int));//datagram_status is the bit array
                            memset(datagram_status_recv1 , 0 , tot_count * sizeof(int));
                            file_array=(unsigned char*)malloc(file_size);
                            buff_alloc_flag = 1;
                    }
    /*******************************************************************************************************************/
                    //printf("tot_count%d",recv_packet_seq_no);                        
                    if(recv_packet_seq_no>-1&&recv_packet_seq_no<tot_count&&source_address==N1)
                    {
                        memcpy(output_data,temp_buffer+HDR_SIZE,data_len);
                        data_decrypted=AESdecrypt(output_data,aes_key,data_len);
                        if (datagram_status_recv1[recv_packet_seq_no] != 1)//if its not set and if u have recived it, set the status
                        {
                                datagram_status_recv1[recv_packet_seq_no] = 1;
                                datagram_total_recv1++;
                                if (((file_size - (recv_packet_seq_no * data_len)) < data_len) )
                                {
                                    //printf("HELLO%s",data_decrypted);
                                    header_format1 *temp_var=(header_format1*)output_hdr;
                                    //printf("%dseq_no",temp_var->seq_no);
                                    //printf("%dpacket type",temp_var->packet_type);
                                    memcpy( &file_array[(recv_packet_seq_no) * (data_len)] , data_decrypted , file_size - (recv_packet_seq_no* data_len));

                                }
                                else 
                                {
                                    memcpy( &file_array[(recv_packet_seq_no) * (data_len)] , data_decrypted , (data_len));
                                }
                        
                        }       


                    }
                }
            



            if (end_flag == 'y')
            {
                    header_format1 my_ackframe;
                    memset(&my_ackframe , 0 , sizeof(my_ackframe));
                    my_ackframe.seq_no = 99999999;
                    my_ackframe.packet_type=ACK_PACKET;                    
                    my_ackframe.src_addr=N2;
                    my_ackframe.dest_addr=N1;
                    int i;
                    for (i = 0; i < 20; ++i)
                    {
                        n = sendto(sockfd, &my_ackframe, sizeof(my_ackframe),0,(struct sockaddr *)&saddr, sizeof(saddr));
                    }
                    break;
            }
            
            else // else send acks by sending sequence numbers alone
            {
                if((recv_packet_seq_no>-1 && recv_packet_seq_no< tot_count)&&source_address==N1)
                {

                    header_format1 my_ackframe;
                    memset(&my_ackframe , 0 , sizeof(my_ackframe));
                    my_ackframe.seq_no = recv_packet_seq_no;
                    my_ackframe.packet_type=ACK_PACKET;
                    my_ackframe.src_addr=N2;
                    my_ackframe.dest_addr=N1;
                    //printf("sendin acks");
                    n = sendto(sockfd, &my_ackframe, sizeof(my_ackframe),0,(struct sockaddr *)&saddr, sizeof(saddr));

                    c++;
                    if(c%10==0)
                    usleep(1000);
                }

            }
            if (datagram_total_recv1 == tot_count  && once == 0) //a check !
            {
                once = 1;
                //printf("%s\n",file_array[0] );
                fwrite(file_array, file_size, 1,fp1);
                fflush(fp1);
                fclose(fp1);
                end_flag = 'y';
            }
        }
        close(sockfd);
        munmap(data_array1,buffer.st_size);
        time(&now);

}
/*******************************************************************************************************************************************************/
void *receiver2(void *inputargv)
{
    char *file_name2 = (char *)inputargv;
    //printf("entered into function");
            fp2 = fopen(file_name2, "wb");
        int sockfd, newsockfd, portno;
        socklen_t clilen,server_len;
        struct sockaddr_in serv_addr, cli_addr;
        socklen_t size = (socklen_t)(sizeof(struct sockaddr_ll));

        //parse_command_line_args(argc,argv);
        /********create your raw socket's file descriptor******************/
        int ifindex, bytes;
        struct ifreq ptr;
        short int proto = htons(0x1234);
        pthread_t sender,receiver;

        /***************************SOCKET CREATION***************************/ 
        if( (sockfd = socket(PF_PACKET,SOCK_RAW,htons(0x1234))) < 0 )
            handle_error("socket");
        printf ("main:  Socket created\n");
        /*********************FETCHING & UPDATING sockaddr_ll***********************/
        memset(&ptr,0, sizeof(ptr));
        strcpy(ptr.ifr_name,"eth4");
        if (ioctl(sockfd, SIOCGIFINDEX, &ptr) < 0){
            printf ("main:  ioctl error\n");
            handle_error ("ioctl");
        }
        ifindex = ptr.ifr_ifindex;

        memset((void*)&saddr, 0, sizeof(saddr));
        saddr.sll_family = PF_PACKET;   
        saddr.sll_ifindex = ifindex;
        saddr.sll_halen = ETH_ALEN;
        saddr.sll_protocol = htons(ETH_P_ALL);

        /**************************BINDING SOCKET************************/
        if (bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
            printf ("main:  Bind error");
            handle_error("bind");
        }


        /*****************************************************************/

        int buff_alloc_flag = 0;
        int tot_count = 0;
        int once = 0;
        char *file_array;
        char end_flag = 'n';
        int c=0;
        int i;
        int debug_count=0;
        int n;
        unsigned char *temp_buffer = (unsigned char*)malloc(frame_len*sizeof(unsigned char)) ;    
        unsigned char *aes_key=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        packet_format*temp_frame,*temp_ack_frame;
        header_format1*temp_hdr;
        short int source_address,dest_address;
        int recv_packet_seq_no;
        
        dec_out=(unsigned char*)malloc((frame_len-HDR_SIZE)*sizeof(unsigned char));

        unsigned char* output_hdr=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        unsigned char* output_data=(unsigned char*)malloc(data_len*sizeof(unsigned char));
        unsigned char* hdr_decrypted=(unsigned char*)malloc(HDR_SIZE*sizeof(unsigned char));
        unsigned char* data_decrypted=(unsigned char*)malloc(data_len*sizeof(unsigned char));

        aes_key=keyCalculate(5);
        
        while(1)
        {

    //printf("entered into function");
 
            if (end_flag == 'n')
            {

                    //printf("got packet %d",debug_count);
                    debug_count++;

                    //receiving encrypted data
                    memset(temp_buffer,0,(frame_len-HDR_SIZE));
                    n = recvfrom(sockfd, temp_buffer, (frame_len-HDR_SIZE) ,0,(struct sockaddr *) &raddr, &size);
        
                    //splitting data and header and decrypting it
                    memset(output_hdr,0,HDR_SIZE);
                    memset(output_data,0,data_len);
                    memset(data_decrypted,0,HDR_SIZE);
                    
                    memcpy(output_hdr,temp_buffer,HDR_SIZE);
                    //hdr_decrypted=AESdecrypt(output_hdr,aes_key,HDR_SIZE);
                    memcpy(&source_address,output_hdr+2,2);
                    memcpy(&dest_address,output_hdr+4,2);               
                    memcpy(&recv_packet_seq_no,output_hdr+8,4);
    /*******************************************************************************************************************/
                    if (buff_alloc_flag == 0)
                    {
                            float check = (float)(file_size)/(float)(data_len);
                            tot_count = ceil(check);
                            datagram_status_recv2=(int*)malloc(tot_count * sizeof(int));//datagram_status is the bit array
                            memset(datagram_status_recv2 , 0 , tot_count * sizeof(int));
                            file_array=(unsigned char*)malloc(file_size);
                            buff_alloc_flag = 1;
                    }
    /*******************************************************************************************************************/
                    //printf("tot_count%d",recv_packet_seq_no);                        
                    if(recv_packet_seq_no>-1&&recv_packet_seq_no<tot_count&&source_address==N3)
                    {
                        memcpy(output_data,temp_buffer+HDR_SIZE,data_len);
                        data_decrypted=AESdecrypt(output_data,aes_key,data_len);
                        if (datagram_status_recv2[recv_packet_seq_no] != 1)//if its not set and if u have recived it, set the status
                        {
                                datagram_status_recv2[recv_packet_seq_no] = 1;
                                datagram_total_recv2++;
                                if (((file_size - (recv_packet_seq_no * data_len)) < data_len) )
                                {
                                    //printf("HELLO%s",data_decrypted);
                                    header_format1 *temp_var=(header_format1*)output_hdr;
                                    //printf("%dseq_no",temp_var->seq_no);
                                    //printf("%dpacket type",temp_var->packet_type);
                                    //printf();
                                    memcpy( &file_array[(recv_packet_seq_no) * (data_len)] , data_decrypted , file_size - (recv_packet_seq_no* data_len));

                                }
                                else 
                                {
                                    memcpy( &file_array[(recv_packet_seq_no) * (data_len)] , data_decrypted , (data_len));
                                }
                        
                        }       


                    }
                }
            



            if (end_flag == 'y')
            {
                    header_format1 my_ackframe;
                    memset(&my_ackframe , 0 , sizeof(my_ackframe));
                    my_ackframe.seq_no = 99999999;
                    // my_ackframe.packet_type=ACK_R1_IN;   
                    my_ackframe.packet_type = ACK_PACKET_TYPE_N2;
                    my_ackframe.src_addr = N2;
                    my_ackframe.dest_addr = N3;
                    int i;
                    for (i = 0; i < 20; ++i)
                    {
                        n = sendto(sockfd, &my_ackframe, sizeof(my_ackframe),0,(struct sockaddr *)&saddr, sizeof(saddr));
                    }
                    break;
            }
            
            else // else send acks by sending sequence numbers alone
            {
                if((recv_packet_seq_no>-1 && recv_packet_seq_no< tot_count)&&source_address == N3)
                {

                    header_format1 my_ackframe;
                    memset(&my_ackframe , 0 , sizeof(my_ackframe));
                    my_ackframe.seq_no = recv_packet_seq_no;
                    // my_ackframe.packet_type=ACK_R1_IN;
                    my_ackframe.packet_type = ACK_PACKET_TYPE_N2;
                    my_ackframe.src_addr = N2;
                    my_ackframe.dest_addr = N3;
                    //printf("sendin acks");
                    n = sendto(sockfd, &my_ackframe, sizeof(my_ackframe),0,(struct sockaddr *)&saddr, sizeof(saddr));

                    c++;
                    if(c%10==0)
                    usleep(1000);
                }

            }
            if (datagram_total_recv2 == tot_count  && once == 0) //a check !
            {
                once = 1;
                //printf("%s\n",file_array[0] );
                fwrite(file_array, file_size, 1,fp2);
                fflush(fp2);
                fclose(fp2);
                end_flag = 'y';
            }
        }
        close(sockfd);
        munmap(data_array2,buffer.st_size);
        time(&now);

}
/*******************************************************************************************************************************************************/
int main(int argc, char *argv[])
{

    parse_command_line_args(argc,argv);
    dec_out=(unsigned char*)malloc((frame_len-HDR_SIZE)*sizeof(unsigned char));

    printf("READING KEYS FROM FILE....");
    read_keys();
    memset(iv,0x00,AES_BLOCK_SIZE);
    int keylength=128;
    key=(unsigned char*)malloc(AES_BLOCK_SIZE*sizeof(unsigned char));    
    key=keyCalculate(5);

    if(strcmp(action,"do_send")==0)
    {
        int num_nodes=atoi(argv[2]);
        file_size = atoi(argv[4]);
        printf("file_size = %d\n", file_size);

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
         time(&now);
         printf("Start time %s", ctime(&now));

        len = (unsigned int)buffer.st_size;
        page_size = getpagesize();

        data_array2 = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,file_des,0);
        pthread_create(&sender_thread2, 0, SendData2, (void *)argv);//send
        pthread_create(&ack_thread2, 0, ReceiveAck2, (void *)argv);//acks
        pthread_join(sender_thread2, 0);
        pthread_join(ack_thread2, 0);
        free(datagram_status_send2);
        if (set_flag1 == 'y')
        {
            free(retransmit2_status);
           free(retransmit2_status1);
        }
        munmap(data_array2,buffer.st_size);

        //sender 2
        if(num_nodes==2)
        {

            //sender 1
            data_array1 = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,file_des,0);
            pthread_create(&sender_thread1, 0, SendData1, (void *)argv);//send
            //FIX- change the packet type on the header for acks !
            pthread_create(&ack_thread1, 0, ReceiveAck1, (void *)argv);//acks
            pthread_join(sender_thread1, 0);
            pthread_join(ack_thread1, 0);
            free(datagram_status_send1);
            if (set_flag1 == 'y')
            {
                free(retransmit1_status);
                free(retransmit1_status1);
            }
            munmap(data_array1,buffer.st_size);

        }
        
    }
    else if(strcmp(action,"do_recv")==0)
    {
        int num_sending_nodes = atoi(argv[2]);
        switch(num_sending_nodes)
        {
            case 1:
                strcpy(filename1, argv[3]);
                printf("filename1 = %s\n", filename1);
                file_size = atoi(argv[4]);
                printf("file_size = %d\n", file_size);
                pthread_create(&recv_thread2, 0, receiver2, (void *)filename1);//send
                pthread_join(recv_thread2, 0);
                break;
            case 2:
                strcpy(filename1, argv[3]);
                strcpy(filename2, argv[4]);
                file_size = atoi(argv[5]);
                printf("file_size = %d\n", file_size);
                pthread_create(&recv_thread1, 0, receiver1, (void *)filename1);
                pthread_create(&recv_thread2, 0, receiver2, (void *)filename2);
                pthread_join(recv_thread1, 0);
                pthread_join(recv_thread2, 0);
                break;
        }
        
    }
    return 0;
}