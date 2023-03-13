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
void handlerThreadSIGUSR1(int sig);
pthread_t tidSlaveTAB[4],tidMASTER;

int main()
{
	int res;
	sigset_t mask;
	
	struct sigaction a,b;
	sigemptyset(&a.sa_mask); 
	a.sa_handler = handlerThreadSIGINT;
	a.sa_flags = 0; 
	sigaction(SIGINT, &a, NULL);

	sigemptyset(&a.sa_mask); 
	a.sa_handler = handlerThreadSIGUSR1;
	a.sa_flags = 0; 
	sigaction(SIGUSR1, &a, NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
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

	for (int i = 0; i < 4; ++i)
	{
		res=pthread_join(tidSlaveTAB[i],NULL);
		if(res==0){printf("\nthread%d(%ld) supprimé,arreté avec succes\n",i+1,tidSlaveTAB[i]);}
	}

	printf("\n");
	fflush(stdout);
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
	sigset_t b;
	sigemptyset(&b);
	sigaddset(&b, SIGUSR1);
	sigprocmask(SIG_UNBLOCK,&b,NULL);

	printf("Je suis le thread_SLAVE(%ld) entrain d'attentre mon signal\n",pthread_self());
	pause();
	pthread_exit(NULL);
}

void handlerThreadSIGINT(int sig)
{
	fflush(stdout);
	printf("\nJe suis le thread_MASTER(%ld) qui a recu le signale n°%d\n\n",pthread_self(),sig);
	kill(getpid(),SIGUSR1);
}

void handlerThreadSIGUSR1(int sig)
{
	fflush(stdout);
	printf("Je suis le thread_SLAVE(%ld) entrain d'attentre mon signal\n",pthread_self());
	pthread_exit(NULL);
}
