void *send_random_packets()
{	
	int sock_sniff;
	sock_sniff = createSocket(threadArg->sniffEthPort);

	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer;
	buffer = (unsigned char *)malloc(frame_len);

	struct sockaddr_ll device={0};
	memset (&device, 0, sizeof (device));
	if ((device->sll_ifindex = if_nametoindex ("eth4")) == 0)
	{
		perror ("if_nametoindex() failed to obtain interface index ");
		exit(1);
	}	
	device.sll_family =   AF_PACKET;
	device.sll_addr[0]  = MY0_SRC_MAC0;
	device.sll_addr[1]  = MY0_SRC_MAC1;
	device.sll_addr[2]  = MY0_SRC_MAC2;
	device.sll_addr[3]  = MY0_SRC_MAC3;
	device.sll_addr[4]  = MY0_SRC_MAC4;
	device.sll_addr[5]  = MY0_SRC_MAC5;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol=htons(0x1234);

	saddr_size = sizeof saddr;
	int i=0;
	header_format1 *pckt=(header_format1*)(buffer);
	pckt->packet_type == BROADCAST_PACKET_TYPE;
	pckt->padding1 = 3;
	while(1)
	{
		sleep(20);
		printf("broadcast pckt value of packet_type is %d\n", pckt->packet_type);
		printf("broadcast pckt value of count is %d\n", pckt->padding1);
		sendto(sock_sniff, buffer, frame_len, 0, (struct sockaddr*)device, sizeof(*device));
	}
	close(sock_sniff);
}