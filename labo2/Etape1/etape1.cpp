#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void *threads_slave(void *Sptr);
void handlerThreadSIGINT(int sig);
pthread_t tidTAB[4];

struct threadsetting
{
	char filename[50];
	char wordFind[50];
	int tab;
}typedef threadsetting;


int main()
{
	int res;
	
	struct sigaction a;
	sigemptyset(&a.sa_mask); 
	a.sa_handler = handlerThreadSIGINT; 
	sigaction(SIGINT, &a, NULL);
	

	printf("\nJe suis le thread_PRINCIPAL(%ld) ALIAS le main()\n\n",pthread_self());
	for(int i=0;i<4;i++)
	{
		res=pthread_create(&tidTAB[i],NULL,threads_slave,NULL);
		if(res==0){printf("thread%d(%ld) cree avec succe\n",i+1,tidTAB[i]);}
	}

	printf("\n");
	pause();
	pthread_exit(NULL);
	
	return 0;
}
void *threads_slave(void *Sptr)
{

	printf("Je suis le thread_slave(%ld) entrain d'attentre mon signal\n",pthread_self());
	pause();
	pthread_exit(NULL);
}


void handlerThreadSIGINT(int sig)
{
	printf("\nJe suis le thread %ld qui a recu le signale n°%d\n\n",pthread_self(),sig);
}