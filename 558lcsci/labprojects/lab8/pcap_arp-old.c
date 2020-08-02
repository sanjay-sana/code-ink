/*
    Packet sniffer using libpcap library
*/
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h> // for exit()
#include <string.h> //for memset

#include <sys/ioctl.h>
#include <sys/types.h>    
#include <net/if.h>
#include <netinet/in.h>
 
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_ntoa()
#include <unistd.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
 
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void process_ip_packet(const u_char * , int);
void print_ip_packet(const u_char * , int);
void print_tcp_packet(const u_char *  , int );
void print_udp_packet(const u_char * , int);
void print_icmp_packet(const u_char * , int );
void PrintData (const u_char * , int);
void getMyMacAddress(char *, char*, char*);
 
FILE *logfile;
struct sockaddr_in source,dest;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;

char *devname;

FILE *file_arp = NULL;
char *arp_cache[2];

void updateArpCache()
{
    struct sockaddr_in addr;
    int fd, cnt;
    struct ip_mreq mreq;
    char *message="Hello!";

    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr("10.10.1.1");
    addr.sin_port=htons(12345);
    if (sendto(fd,message,sizeof(message),0,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
       perror("sendto");
       exit(1);
    }

    addr.sin_addr.s_addr=inet_addr("10.10.3.2");
    if (sendto(fd,message,sizeof(message),0,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
       perror("sendto");
       exit(1);
    }

    addr.sin_addr.s_addr=inet_addr("10.1.2.3");
    if (sendto(fd,message,sizeof(message),0,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
       perror("sendto");
       exit(1);
    }

    addr.sin_addr.s_addr=inet_addr("10.1.2.4");
    if (sendto(fd,message,sizeof(message),0,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
       perror("sendto");
       exit(1);
    }
    usleep(11000);
    printf("ARP Updated\n");
}

void buildArpTable()
{
    char buff[2000];
    file_arp = fopen("/proc/net/arp", "r");
    size_t length = fread(buff, 1, sizeof(buff), file_arp);
    printf("Buffer\n%s\n", buff);
    char *line = NULL;
    // printf("fault1\n");
    line = strtok(buff, "\n");
    // printf("fault2\n");
    int i=-1;
    // printf("Line\n");
    // arp_cache = (char*)calloc(2,strlen(line));
    while (line != NULL)
    {
        // printf("while loop %d\n", strlen(line));
        // printf("%s\n", line, strlen(line));
        if(strncmp(line, "10.10.", 6) == 0)
        {
            i++;
            arp_cache[i] = (char *)calloc(sizeof(char), strlen(line)-1);
            memset(arp_cache[i], 0, sizeof(arp_cache[i]));
            // strncpy(arp_cache[i], line, 76);
            memcpy(arp_cache[i], line, strlen(line));
            *(arp_cache[i]+76) = '\0';
            memset(line, 0, strlen(line));
            // printf("Match found\n");
        }
        line = strtok(NULL, "\n");
    }
    printf("ARP Cache\n");
    printf("%s\n", arp_cache[0], strlen(arp_cache[0]));
    printf("%s\n", arp_cache[1], strlen(arp_cache[1]));
}

int main()
{
    pcap_if_t *alldevsp , *device;
    pcap_t *handle; //Handle of the device that shall be sniffed
 
    char errbuf[100], devs[100][100];
    int count = 1 , n;
     
    // //First get the list of available devices
    // printf("Finding available devices ... ");
    // if( pcap_findalldevs( &alldevsp , errbuf) )
    // {
    //     printf("Error finding devices : %s" , errbuf);
    //     exit(1);
    // }
    // printf("Done");
     
    // //Print the available devices
    // printf("\nAvailable Devices are :\n");
    // for(device = alldevsp ; device != NULL ; device = device->next)
    // {
    //     printf("%d. %s - %s\n" , count , device->name , device->description);
    //     if(device->name != NULL)
    //     {
    //         strcpy(devs[count] , device->name);
    //     }
    //     count++;
    // }
    // // printf("devices\n");
    // // printf("%s\n", devs[7]);
    // // printf("%s\n", devs[8]);
    // // printf("%s\n", devs[9]);
    // // printf("***\n");
    // //Ask user which device to sniff
    // printf("Enter the number of the device you want to sniff : ");
    // scanf("%d" , &n);
    // devname = devs[n];
    devname = "eth0";
     
    //Open the device for sniffing
    printf("Opening device %s for sniffing ... " , devname);
    handle = pcap_open_live(devname , 65536 , 1 , 0 , errbuf);
     
    if (handle == NULL) 
    {
        fprintf(stderr, "Couldn't open device %s : %s\n" , devname , errbuf);
        exit(1);
    }
    printf("Done\n");

// Get ARP Cache updated by sending ARP packets.
    updateArpCache();
    buildArpTable();
     
    logfile=fopen("log.txt","w");
    if(logfile==NULL) 
    {
        printf("Unable to create file.");
    }
     
    //Put the device in sniff loop
    pcap_loop(handle , -1 , process_packet , NULL);
     
    return 0;   
}
 
void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
    int size = header->len;
     
    //Get the IP Header part of this packet , excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    ++total;
    switch (iph->protocol) //Check the Protocol and do accordingly...
    {
        case 1:  //ICMP Protocol
            ++icmp;
            // printf("Next is print icmp packet\n");
            print_icmp_packet( buffer , size);
            break;
         
        case 2:  //IGMP Protocol
            ++igmp;
            break;
         
        case 6:  //TCP Protocol
            ++tcp;
            print_tcp_packet(buffer , size);
            break;
         
        case 17: //UDP Protocol
            ++udp;
            print_udp_packet(buffer , size);
            break;
         
        default: //Some Other Protocol like ARP etc.
            ++others;
            break;
    }
    printf("TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r", tcp , udp , icmp , igmp , others , total);
}
 
void print_ethernet_header(const u_char *Buffer, int Size)
{
    struct ethhdr *eth = (struct ethhdr *)Buffer;
     
    fprintf(logfile , "\n");
    fprintf(logfile , "Ethernet Header\n");
    fprintf(logfile , "   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5] );
    fprintf(logfile , "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    fprintf(logfile , "   |-Protocol            : %u \n",(unsigned short)eth->h_proto);
}
 
void print_ip_header(const u_char * Buffer, int Size)
{
    print_ethernet_header(Buffer , Size);
   
    unsigned short iphdrlen;
         
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );
    iphdrlen =iph->ihl*4;
     
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
     
    fprintf(logfile , "\n");
    fprintf(logfile , "IP Header\n");
    fprintf(logfile , "   |-IP Version        : %d\n",(unsigned int)iph->version);
    fprintf(logfile , "   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
    fprintf(logfile , "   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
    fprintf(logfile , "   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
    fprintf(logfile , "   |-Identification    : %d\n",ntohs(iph->id));
    //fprintf(logfile , "   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
    //fprintf(logfile , "   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
    //fprintf(logfile , "   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
    fprintf(logfile , "   |-TTL      : %d\n",(unsigned int)iph->ttl);
    fprintf(logfile , "   |-Protocol : %d\n",(unsigned int)iph->protocol);
    fprintf(logfile , "   |-Checksum : %d\n",ntohs(iph->check));
    fprintf(logfile , "   |-Source IP        : %s\n" , inet_ntoa(source.sin_addr) );
    fprintf(logfile , "   |-Destination IP   : %s\n" , inet_ntoa(dest.sin_addr) );
}
 
void print_tcp_packet(const u_char * Buffer, int Size)
{
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)( Buffer  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
     
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
             
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
     
    fprintf(logfile , "\n\n***********************TCP Packet*************************\n");  
         
    print_ip_header(Buffer,Size);
         
    fprintf(logfile , "\n");
    fprintf(logfile , "TCP Header\n");
    fprintf(logfile , "   |-Source Port      : %u\n",ntohs(tcph->source));
    fprintf(logfile , "   |-Destination Port : %u\n",ntohs(tcph->dest));
    fprintf(logfile , "   |-Sequence Number    : %u\n",ntohl(tcph->seq));
    fprintf(logfile , "   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
    fprintf(logfile , "   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
    //fprintf(logfile , "   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
    //fprintf(logfile , "   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
    fprintf(logfile , "   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
    fprintf(logfile , "   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
    fprintf(logfile , "   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
    fprintf(logfile , "   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
    fprintf(logfile , "   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
    fprintf(logfile , "   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
    fprintf(logfile , "   |-Window         : %d\n",ntohs(tcph->window));
    fprintf(logfile , "   |-Checksum       : %d\n",ntohs(tcph->check));
    fprintf(logfile , "   |-Urgent Pointer : %d\n",tcph->urg_ptr);
    fprintf(logfile , "\n");
    fprintf(logfile , "                        DATA Dump                         ");
    fprintf(logfile , "\n");
         
    fprintf(logfile , "IP Header\n");
    PrintData(Buffer,iphdrlen);
         
    fprintf(logfile , "TCP Header\n");
    PrintData(Buffer+iphdrlen,tcph->doff*4);
         
    fprintf(logfile , "Data Payload\n");    
    PrintData(Buffer + header_size , Size - header_size );
                         
    fprintf(logfile , "\n###########################################################");
}
 
void print_udp_packet(const u_char *Buffer , int Size)
{
     
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)(Buffer +  sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;
     
    struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen  + sizeof(struct ethhdr));
     
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof udph;
     
    fprintf(logfile , "\n\n***********************UDP Packet*************************\n");
     
    print_ip_header(Buffer,Size);           
     
    fprintf(logfile , "\nUDP Header\n");
    fprintf(logfile , "   |-Source Port      : %d\n" , ntohs(udph->source));
    fprintf(logfile , "   |-Destination Port : %d\n" , ntohs(udph->dest));
    fprintf(logfile , "   |-UDP Length       : %d\n" , ntohs(udph->len));
    fprintf(logfile , "   |-UDP Checksum     : %d\n" , ntohs(udph->check));
     
    fprintf(logfile , "\n");
    fprintf(logfile , "IP Header\n");
    PrintData(Buffer , iphdrlen);
         
    fprintf(logfile , "UDP Header\n");
    PrintData(Buffer+iphdrlen , sizeof udph);
         
    fprintf(logfile , "Data Payload\n");    
     
    //Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size , Size - header_size);
     
    fprintf(logfile , "\n###########################################################");
}

void getMyMacAddress(char *devname, char* my_mac, char* my_recv_ip)
{
    int s;
    struct ifreq buffer;
    s = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&buffer, 0x00, sizeof(buffer));
    strcpy(buffer.ifr_name, devname);
    ioctl(s, SIOCGIFHWADDR, &buffer);
    sprintf(my_mac, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", (unsigned char)buffer.ifr_hwaddr.sa_data[0], (unsigned char)buffer.ifr_hwaddr.sa_data[1], (unsigned char)buffer.ifr_hwaddr.sa_data[2], (unsigned char)buffer.ifr_hwaddr.sa_data[3], (unsigned char)buffer.ifr_hwaddr.sa_data[4], (unsigned char)buffer.ifr_hwaddr.sa_data[5]);
    memset(&buffer, 0x00, sizeof(buffer));
    strcpy(buffer.ifr_name, devname);
    ioctl(s, SIOCGIFADDR, &buffer);
    sprintf(my_recv_ip, "%s", inet_ntoa(((struct sockaddr_in *)&buffer.ifr_addr)->sin_addr));
    close(s);
}

void print_icmp_packet(const u_char * Buffer , int Size)
{
    // printf("log file print started\n");
    unsigned short iphdrlen;
    struct ethhdr *eth = (struct ethhdr *)Buffer;
     
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;
     
    struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen  + sizeof(struct ethhdr));
     
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof icmph;
    char my_mac[20];
    char my_recv_ip[20];
    getMyMacAddress(devname, my_mac, my_recv_ip);
    char dest_mac[20];
    sprintf(dest_mac, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
    printf("RTR3 eth0 IP: %s, length = %d\n", my_recv_ip, strlen(my_recv_ip));
    printf("RTR3 MAC = %s, length = %d\n", my_mac, strlen(my_mac));
    printf("DEST MAC = %s, length = %d\n", dest_mac, strlen(dest_mac));

    if (strcmp(my_mac, dest_mac) == 0)
    {
        printf("Proceed further\n");
        /* 
        Get next hop IP address by checking the routing table
        Use next hop IP to fetch its corresponsing MAC
        Replace the buffer dsrc/dest MAC/IP with the new ones
        Forward the packet to the next hop
        */
    }
    else
    {
        printf("Dont deal with it\n");
    }

    fprintf(logfile , "\n\n***********************ICMP Packet*************************\n"); 
     
    print_ip_header(Buffer , Size);
             
    fprintf(logfile , "\n");
         
    fprintf(logfile , "ICMP Header\n");
    fprintf(logfile , "   |-Type : %d",(unsigned int)(icmph->type));
             
    if((unsigned int)(icmph->type) == 11)
    {
        fprintf(logfile , "  (TTL Expired)\n");
    }
    else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
    {
        fprintf(logfile , "  (ICMP Echo Reply)\n");
    }
     
    fprintf(logfile , "   |-Code : %d\n",(unsigned int)(icmph->code));
    fprintf(logfile , "   |-Checksum : %d\n",ntohs(icmph->checksum));
    //fprintf(logfile , "   |-ID       : %d\n",ntohs(icmph->id));
    //fprintf(logfile , "   |-Sequence : %d\n",ntohs(icmph->sequence));
    fprintf(logfile , "\n");
 
    fprintf(logfile , "IP Header\n");
    PrintData(Buffer,iphdrlen);
         
    fprintf(logfile , "UDP Header\n");
    PrintData(Buffer + iphdrlen , sizeof icmph);
         
    fprintf(logfile , "Data Payload\n");    
     
    //Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size , (Size - header_size) );
     
    fprintf(logfile , "\n###########################################################");
    // printf("logfile print packet end\n");
}
 
void PrintData (const u_char * data , int Size)
{
    int i , j;
    for(i=0 ; i < Size ; i++)
    {
        if( i!=0 && i%16==0)   //if one line of hex printing is complete...
        {
            fprintf(logfile , "         ");
            for(j=i-16 ; j<i ; j++)
            {
                if(data[j]>=32 && data[j]<=128)
                    fprintf(logfile , "%c",(unsigned char)data[j]); //if its a number or alphabet
                 
                else fprintf(logfile , "."); //otherwise print a dot
            }
            fprintf(logfile , "\n");
        } 
         
        if(i%16==0) fprintf(logfile , "   ");
            fprintf(logfile , " %02X",(unsigned int)data[i]);
                 
        if( i==Size-1)  //print the last spaces
        {
            for(j=0;j<15-i%16;j++) 
            {
              fprintf(logfile , "   "); //extra spaces
            }
             
            fprintf(logfile , "         ");
             
            for(j=i-i%16 ; j<=i ; j++)
            {
                if(data[j]>=32 && data[j]<=128) 
                {
                  fprintf(logfile , "%c",(unsigned char)data[j]);
                }
                else
                {
                  fprintf(logfile , ".");
                }
            }
             
            fprintf(logfile ,  "\n" );
        }
    }
}
