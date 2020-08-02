#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "my402list.h"

typedef struct input_arguments {
	float lambda;
	float mu;
	float r;
	unsigned int bucketSize;
	unsigned int tokensPerPacket;
	unsigned int numPackets;
} inputArg;

typedef struct packetDesc {
	// time_t arrivalTime;
	unsigned int num_tokens;
	// time_t serviceTime;
} packetData;

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

struct timeval startTime, endTime;

inputArg *inputVar;
pthread_t packet_id, token_id, server_id, monitor;
My402List* linkedList_q1;
My402List* linkedList_q2;
unsigned int token_count = 0;
unsigned int packet_count = 0;
short int stop_flag = 1;

sigset_t set;

void *PacketArrival(void *inputVar)
{
	fprintf(stdout, "Packet Arrival\n");
	time_t interArrival_packet;
	time_t usedTime1 = 0;
	inputArg *input_values;
	// linkedList_q1 = (My402List*)malloc(sizeof(My402List));
	// packet = (packetData*)malloc(sizeof(packetData));
	input_values = (inputArg*)inputVar;
	interArrival_packet = ((1/input_values->lambda)*1000000);
	while (stop_flag)
	{
		My402ListElem *currElement;
		packetData *addPacket;
		addPacket = (packetData*)malloc(sizeof(packetData));
		fprintf(stdout, "Packet1\n");
		gettimeofday(&startTime, NULL);
		fprintf(stdout, "sleep time = %ld\n", (interArrival_packet - usedTime1));
		// if ((interArrival_packet - usedTime1) > 0)
		// {
		// 	usleep(interArrival_packet - usedTime1);
		// }
		// else
		// {
		// 	usleep(0);
		// }
		sleep(1);
		// fprintf(stdout, "interArrival_time = %ld\n", interArrival_packet);
		addPacket->num_tokens = 3;
		pthread_mutex_lock(&m1);
		// pthread_mutex_lock(&m3);
		fprintf(stdout, "Mutex packet:\n");
		My402ListAppend(linkedList_q1, (void*)addPacket);
		fprintf(stdout, "linkedList_q1->num_members = %d\n", linkedList_q1->num_members);
		fprintf(stdout, "token_count = %d\n", token_count);
		fprintf(stdout, "List empty: %d\n", My402ListEmpty(linkedList_q1));
		// if (My402ListEmpty(linkedList_q1) == 0 && token_count >= input_values->tokensPerPacket)
		// {
			if (My402ListEmpty(linkedList_q1) == 0 && token_count >= input_values->tokensPerPacket)
			{
				fprintf(stdout, "Packet2\n");
				// fprintf(stdout, "num elements = %d\n", linkedList_q1->num_members);
				packetData *newpacket;
				currElement = My402ListFirst(linkedList_q1);
				newpacket = (packetData*)currElement->obj;
				My402ListUnlink(linkedList_q1, currElement);
				token_count = token_count - input_values->tokensPerPacket;
				My402ListAppend(linkedList_q2, (void*)newpacket);
				fprintf(stdout, "Snum elements q1= %d\n", linkedList_q1->num_members);
				fprintf(stdout, "Snum elements q2= %d\n", linkedList_q2->num_members);
				if (linkedList_q2->num_members == 1)
				{
					pthread_cond_signal(&condVar);
				}
			}
		// }
		pthread_mutex_unlock(&m1);
		gettimeofday(&endTime, NULL);
		usedTime1 = ((endTime.tv_sec * 1000000) + endTime.tv_usec) - ((startTime.tv_sec * 1000000) + startTime.tv_usec);
		// pthread_mutex_unlock(&m3);
	}
	return 0;
}

void *TokenArrival(void *inputVar)
{
	fprintf(stdout, "Token Arrival\n");
	time_t interArrival_time;
	time_t usedTime2 = 0;
	inputArg *input_values;
	// linkedList_q1 = (My402List*)malloc(sizeof(My402List));
	// packet = (packetData*)malloc(sizeof(packetData));
	input_values = (inputArg*)inputVar;
	interArrival_time = ((1/input_values->r)*1000000);
	while (stop_flag)
	{
		fprintf(stdout, "Token1\n");
		gettimeofday(&startTime, NULL);
		fprintf(stdout, "sleep time = %ld\n", (interArrival_time - usedTime2));
		// if ((interArrival_time - usedTime2) > 0)
		// {
		// 	usleep(interArrival_time - usedTime2);
		// }
		// else
		// {
		// 	usleep(0);
		// }
		sleep(1);
		// fprintf(stdout, "interArrival_time = %ld\n", interArrival_time);
		pthread_mutex_lock(&m1);
		// pthread_mutex_lock(&m2);
		// pthread_mutex_lock(&m3);
		fprintf(stdout, "Mutex token:\n");
		token_count++;
		// fprintf(stdout, "linkedList_q1->num_members = %d\n", linkedList_q1->num_members);
		fprintf(stdout, "token_count = %d\n", token_count);
		// if (My402ListEmpty(linkedList_q1) == 0 && token_count >= input_values->tokensPerPacket)
		// {
			My402ListElem *currElement;
			packetData *packet;
			if (My402ListEmpty(linkedList_q1) == 0 && token_count >= input_values->tokensPerPacket)
			{
				fprintf(stdout, "Token2\n");
				currElement = My402ListFirst(linkedList_q1);
				packet = (packetData*)currElement->obj;
				// fprintf(stdout, "num before q1= %d\n", linkedList_q1->num_members);
				My402ListUnlink(linkedList_q1, currElement);
				token_count = token_count - input_values->tokensPerPacket;
				My402ListAppend(linkedList_q2, (void*)packet);
				fprintf(stdout, "Tnum elements q1= %d\n", linkedList_q1->num_members);
				fprintf(stdout, "Tnum elements q2= %d\n", linkedList_q2->num_members);
				if (linkedList_q2->num_members == 1)
				{
					pthread_cond_signal(&condVar);
				}
			}
		// }
		pthread_mutex_unlock(&m1);
		// pthread_mutex_unlock(&m2);
		// pthread_mutex_unlock(&m3);
		gettimeofday(&endTime, NULL);
		usedTime2 = ((endTime.tv_sec * 1000000) + endTime.tv_usec) - ((startTime.tv_sec * 1000000) + startTime.tv_usec);
	}
	return 0;
}

void *Server(void *inputVar)
{
	fprintf(stdout, "Server function\n");
	time_t service_rate;
	time_t usedTime3 = 0;
	inputArg *input_values;
	input_values = (inputArg*)inputVar;
	service_rate = ((1/input_values->mu)*1000000);
	while (stop_flag)
	{
		fprintf(stdout, "Server1\n");
		gettimeofday(&startTime, NULL);
		fprintf(stdout, "sleep time = %ld\n", (service_rate - usedTime3));
		// if ((service_rate - usedTime3) > 0)
		// {
		// 	usleep(service_rate - usedTime3);
		// }
		// else
		// {
		// 	usleep(0);
		// }
		sleep(1);
		pthread_mutex_lock(&m1);
		while(My402ListEmpty(linkedList_q2) == TRUE)
		{
			fprintf(stdout, "Wait\n");
			pthread_cond_wait(&condVar, &m1);
		}
		fprintf(stdout, "Hello\n");
		if (My402ListEmpty(linkedList_q2) == 0)
		{
			fprintf(stdout, "Yessss\n");
			My402ListElem *currElem;
			// packetData *currPacket;
			currElem = My402ListFirst(linkedList_q2);
			// currPacket = (packetData*)currElem->obj;
			My402ListUnlink(linkedList_q2, currElem);
			fprintf(stdout, "num elements q2= %d\n", linkedList_q2->num_members);
		}
		pthread_mutex_unlock(&m1);
		gettimeofday(&endTime, NULL);
		usedTime3 = ((endTime.tv_sec * 1000000) + endTime.tv_usec) - ((startTime.tv_sec * 1000000) + startTime.tv_usec);
	}
	return 0;
}

void monitor_func(void * inputVar)
{
	int sig;
	sigwait(&set, &sig);
	pthread_mutex_lock(&m1);
	pthread_cancel(packet_id);
	pthread_cancel(token_id);
	pthread_cancel(server_id);
	pthread_mutex_unlock(&m1);
	return(0);
}

// void handler(int signo)
// {
// 	stop_flag = 1;
// 	fprintf(stdout, "\nTerminated...\n");
// 	exit(1);
// }

int main(int argc, char *argv[])
{
	// sigset(SIGINT,handler);
	sigemptyset(&set);
 	sigaddset(&set, SIGINT);
 	sigprocmask(SIG_BLOCK, &set, 0);
	inputVar = (inputArg*)malloc(sizeof(inputArg));
	inputVar->lambda = 2;
	inputVar->mu = 0.35;
	inputVar->r = 1.5;
	inputVar->bucketSize = 10;
	inputVar->tokensPerPacket = 3;
	inputVar->numPackets = 20;
	fprintf(stdout, "argc = %d\n", argc);
	fprintf(stdout, "Emulation parameters:\n");
	fprintf(stdout, "\tnumber to arrive = %d\n", inputVar->numPackets);
	// for (int i = 0; i < argc; ++i)
	// {
	// 	if (strcmp(argv[i], "-lambda") == 0)
	// 	{
	// 		fprintf(stdout, "\tlambda = %f\n", inputVar.lambda);
	// 	}
	// }
	fprintf(stdout, "\tlambda = %f\n", inputVar->lambda);
	fprintf(stdout, "\tmu = %f\n", inputVar->mu);
	fprintf(stdout, "\tr = %f\n", inputVar->r);
	fprintf(stdout, "\tB = %d\n", inputVar->bucketSize);
	fprintf(stdout, "\tP = %d\n", inputVar->tokensPerPacket);
	// fprintf(stdout, "\ttsfile = %f\n", inputVar.lambda);
	fprintf(stdout, "00000000.000ms: emulation begins\n");
	// gettimeofday(&startTime, NULL);
	// fprintf(stdout, "%ld\n", (startTime.tv_sec * 1000000) + startTime.tv_usec);
	linkedList_q1 = (My402List*)malloc(sizeof(My402List));
	linkedList_q2 = (My402List*)malloc(sizeof(My402List));
	My402ListInit(linkedList_q1);
	My402ListInit(linkedList_q2);
	pthread_create(&monitor, 0, monitor_func, (void *)inputVar);
	pthread_create(&packet_id, 0, PacketArrival, (void *)inputVar);
	pthread_create(&token_id, 0, TokenArrival, (void *)inputVar);
	pthread_create(&server_id, 0, Server, (void *)inputVar);
	pthread_join(packet_id, 0);
	pthread_join(token_id, 0);
	pthread_join(server_id, 0);
	fprintf(stdout, "emulation ends\n");
	return 0;
}
