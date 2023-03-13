#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void *threads_SLAVE(void *Sptr);
void *thread_MASTER(void *Sptr);
void handlerThreadSIGINT(int sig);
pthread_t tidSlaveTAB[4],tidMASTER;

struct threadsetting
{
	char filename[50];
	char wordFind[50];
	int tab;
}typedef threadsetting;


int main()
{
	int res;
	sigset_t mask;
	
	struct sigaction a;
	sigemptyset(&a.sa_mask); 
	a.sa_handler = handlerThreadSIGINT; 
	sigaction(SIGINT, &a, NULL);
	//sigprocmask(SIG_BLOCK,&a.sa_mask,NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK,&mask,NULL);


	printf("\nJe suis le thread_PRINCIPAL(%ld) ALIAS le main()\n\n",pthread_self());
	
	res=pthread_create(&tidMASTER,NULL,thread_MASTER,NULL);
	if(res==0){printf("thread_MASTER(%ld) cree avec succe\n\n",tidMASTER);}
	
	for(int i=0;i<4;i++)
	{
		res=pthread_create(&tidSlaveTAB[i],NULL,threads_SLAVE,NULL);
		if(res==0){printf("thread%d(%ld) cree avec succe\n",i+1,tidSlaveTAB[i]);}
	}

	printf("\n");
	fflush(stdout);
	pause();
	pthread_exit(NULL);
	
	return 0;
}
void *thread_MASTER(void *Sptr)
{
	sigset_t b;
	sigemptyset(&b);
	sigaddset(&b, SIGINT);
	sigprocmask(SIG_UNBLOCK,&b,NULL);

	printf("Je suis le thread_MASTER(%ld) entrain d'attentre mon signal\n",pthread_self());
	
	while(1)
	{
		pause();
	}
	pthread_exit(NULL);

	
}
void *threads_SLAVE(void *Sptr)
{

	printf("Je suis le thread_SLAVE(%ld) entrain d'attentre mon signal\n",pthread_self());
	pause();
	pthread_exit(NULL);
}


void handlerThreadSIGINT(int sig)
{
	printf("\nJe suis le thread %ld qui a recu le signale n°%d\n\n",pthread_self(),sig);
}