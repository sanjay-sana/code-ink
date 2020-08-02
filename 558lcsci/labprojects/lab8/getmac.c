#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <net/if.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

int main( int argc, char *argv[] )
{
    int s;
    struct ifreq buffer;
    char mymac[20];

    s = socket(PF_INET, SOCK_DGRAM, 0);

    memset(&buffer, 0x00, sizeof(buffer));

    strcpy(buffer.ifr_name, "eth0");

    ioctl(s, SIOCGIFHWADDR, &buffer);

    close(s);

    sprintf(mymac, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", (unsigned char)buffer.ifr_hwaddr.sa_data[0], (unsigned char)buffer.ifr_hwaddr.sa_data[1], (unsigned char)buffer.ifr_hwaddr.sa_data[2], (unsigned char)buffer.ifr_hwaddr.sa_data[3], (unsigned char)buffer.ifr_hwaddr.sa_data[4], (unsigned char)buffer.ifr_hwaddr.sa_data[5]);
    printf("mymac = %s\n", mymac);
    printf("\n");

    return 0;
}
