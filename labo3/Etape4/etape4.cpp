
#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

void *threads_SECOND(void *Sptr);
void handlerThreadSIGINT(int sig);
void destructeur(void* nom);
pthread_t tidSecondTAB[5],tidMASTER;
pthread_mutex_t mutexParam;
pthread_mutex_t mutexCpt;
pthread_cond_t condCpt;
int cpt;
pthread_key_t cle;


typedef struct
{ 
	char nom[20];
	int nbSecondes;

} DONNEE;

DONNEE data[] = { "MATAGNE",15,
				  "WILVERS",10,
				  "WAGNER",17,
				  "QUETTIER",8,
				   "",0 };

DONNEE Param;

int main()
{
	int res;

	pthread_mutex_init(&mutexParam,NULL);
	pthread_mutex_init(&mutexCpt,NULL);
	pthread_cond_init(&condCpt,NULL);

	sigset_t mask;
	
	struct sigaction a;
	sigemptyset(&a.sa_mask);
	a.sa_handler = handlerThreadSIGINT;
	a.sa_flags = 0; 
	sigaction(SIGINT, &a, NULL);

	sigemptyset(&mask); 
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK,&mask,NULL);

	printf("\nJe suis le thread_PRINCIPAL(%lu) ALIAS le main()\n\n",pthread_self());
	pthread_key_create(&cle,destructeur);

	for(int i=0;i<5;i++)
	{
		pthread_mutex_lock(&mutexParam);
		memcpy(&Param,&data[i],sizeof(DONNEE)) ;
		//strcpy(Param.nom,data[i].nom);
		//Param.nbSecondes=data[i].nbSecondes;
		if(strlen(Param.nom) >0)
		{
			//printf("param= %s\n",Param.nom);
			res=pthread_create(&tidSecondTAB[i],NULL,threads_SECOND,&Param);
			if(res==0){printf("\nthreadScond%d(%lu) cree avec succe\n",i+1,tidSecondTAB[i]); cpt++;}
		}
	}

	pthread_mutex_lock(&mutexCpt);
	printf("\ncpt=%d",cpt);
	while(cpt>0)
		pthread_cond_wait(&condCpt,&mutexCpt);
		printf("\n\ncpt=%d ducoup tout les threads_SECOND sont fini",cpt);
	pthread_mutex_unlock(&mutexCpt);

	printf("\n");
	pthread_key_delete(cle);
	pthread_exit(NULL);
	return 0;
}

void *threads_SECOND(void *Sptr)
{
	sigset_t mask;
	sigemptyset(&mask); 
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_UNBLOCK,&mask,NULL);

	DONNEE *tmp= (DONNEE*)Sptr;
	struct timespec temps={tmp->nbSecondes,0};
	char * nom= (char*) malloc(strlen(tmp->nom)+1);
	strcpy(nom,tmp->nom);
	// Stocker la chaîne de caractères dans la zone spécifique du thread associée à la clé
	 pthread_setspecific(cle, nom);
	printf("\nJe suis le threads_SECOND %lu lance\n",pthread_self());
	printf("Le nom passe en paramettre est %s\n",tmp->nom);
	pthread_mutex_unlock(&mutexParam);
	nanosleep(&temps,NULL);
	//kill(getpid(),SIGINT);
	printf("\nJe suis le threads_SECOND %lu se termine\n",pthread_self());
	pthread_mutex_lock(&mutexCpt);
	cpt--;
	pthread_mutex_unlock(&mutexCpt);
	pthread_cond_signal(&condCpt);
	pthread_exit(NULL);
}

void handlerThreadSIGINT(int sig)
{
	 char* nom = (char*)pthread_getspecific(cle);
    printf("Thread %lu s'occupe de \"%s\"\n", (unsigned long)pthread_self(), nom);

	printf("\nJe suis le thread_SECOND(%ld) qui a recu le signale n°%d\n\n",pthread_self(),sig);
}

void destructeur(void* nom)
{
	free(nom);
}