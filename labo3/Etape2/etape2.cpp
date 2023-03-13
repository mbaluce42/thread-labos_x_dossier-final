
#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

void *threads_SECOND(void *Sptr);
pthread_t tidSecondTAB[5],tidMASTER;
pthread_mutex_t mutexParam;

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

	printf("\nJe suis le thread_PRINCIPAL(%lu) ALIAS le main()\n\n",pthread_self());
	
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
			if(res==0){printf("\nthreadScond%d(%lu) cree avec succe\n",i+1,tidSecondTAB[i]);}
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		res=pthread_join(tidSecondTAB[i],NULL);
		if(res==0){printf("\nthread%d(%lu) supprimé,arreté avec succes\n",i+1,tidSecondTAB[i]);}
	}

	printf("\n");
	fflush(stdout);
	pthread_exit(NULL);
	return 0;
}

void *threads_SECOND(void *Sptr)
{
	DONNEE *tmp= (DONNEE*)Sptr;
	struct timespec temps={tmp->nbSecondes,0};
	printf("\nJe suis le threads_SECOND %lu lance\n",pthread_self());
	printf("Le nom passe en paramettre est %s\n",tmp->nom);
	nanosleep(&temps,NULL);
	pthread_mutex_unlock(&mutexParam);
	printf("\nJe suis le threads_SECOND %lu se termine\n",pthread_self());
	
	pthread_exit(NULL);
}