#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Function to get the MAC address of the next hop from the ARP cache table

char* get_arp_function(char *argument_ip_add)
{
	FILE *fp_new;
    char *line_buffer;
    char *file_name;
	int found_flag=0;
  file_name=(char *)malloc(15*sizeof(char));
  strcpy(file_name,"/proc/net/arp");
  fp_new=fopen(file_name,"r");
  if(fp_new==NULL)
 {
  fprintf(stderr,"ARP cache file cannot be opened\n");
 }
 line_buffer=(char *)malloc(70*sizeof(char));
int line_number=1; 
while(fgets(line_buffer,70,(FILE*)fp_new)!=NULL)
	
	{
		if(line_number==1)
		{
			memset(line_buffer,0,(70*sizeof(char)));
			line_number++;
		}
		else
		{
		  
		  
		  
		  char *ip_add;
		  ip_add=(char *)malloc(14*sizeof(char));
		  ip_add=strtok(line_buffer," ");
	      int comp=strcmp(ip_add,argument_ip_add);
		  
          if(comp==0)		  
		  {
			  
		  strtok(NULL," ");
		  strtok(NULL," ");
		  char *mac_add;
		  mac_add=(char *)malloc(20*sizeof(char));
		  mac_add=strtok(NULL," ");
		  printf("The mac address obtained is %s\n",mac_add);
		  found_flag=1;
		  return mac_add;
		  
		  break;
		  }
		  
		  memset(line_buffer,0,(70*sizeof(char)));

		  memset(ip_add,0,(14*sizeof(char)));
	     line_number++;
		 
	 }
		
       
		  
		}
      if(found_flag==0)
      {
		  char *no_match_string;
	      no_match_string=(char *)malloc(20*sizeof(char));
		  strcpy(no_match_string,"No_match");
		  return no_match_string;
	  }		  
fclose(fp_new);
 }






// Building a routing table 




int main()
{
	 FILE *fp;
	 fp=fopen("route_table.txt","w");
	 //Writing the column identifier
	 fputs("destination_net     next_hop_ip     next_hop_mac\n",fp);
	 
	 //Writing 1st entry
	 fputs("10.10.1.0           10.10.1.1       ",fp);
	 char *next_mac=NULL;
	 next_mac=(char *)malloc(20*sizeof(char));
	 char *ip_argument=NULL;
	 ip_argument = (char *)malloc(14*sizeof(char));
	 strcpy(ip_argument,"10.10.1.1");
	
	 strcpy(next_mac,get_arp_function(ip_argument));
	 printf("The first returned MAC address is %s\n",next_mac);
         fputs(next_mac,fp);
	 fputs("\n",fp);
	 
	 //writing 2nd entry
	 memset(ip_argument,0,(14*sizeof(char)));
	 memset(next_mac,0,(20*sizeof(char)));
	 fputs("10.10.3.0           10.10.3.2       ",fp);
	 strcpy(ip_argument,"10.10.3.2");
	 strcpy(next_mac,get_arp_function(ip_argument));
	 printf("The second returned MAC address is %s\n",next_mac);
         fputs(next_mac,fp);
	 //fputs("\n",fp);
	 
	 //Cleaning
	 memset(ip_argument,0,(14*sizeof(char)));
         memset(next_mac,0,(20*sizeof(char)));   
         free(ip_argument);
	 free(next_mac);
	 fclose(fp);
	 
	 //Displaying the routing table formed
	 printf("The new routing table formed for rtr3 is as follows :\n");
         char *buffer;
	 buffer=(char *)malloc(90*sizeof(char));
	 FILE *fp_second;
	 fp_second=fopen("route_table.txt","r");
	 while(fgets(buffer,90,(FILE*)fp_second)!=NULL)
	 {
		 printf("%s",buffer);
		 memset(buffer,0,(90*sizeof(char)));
	 }
	 printf("\n");
     free(buffer);
	 fclose(fp_second);
	 
	 return(0);
	 
}
