
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

typedef struct
{ 
	char nom[20];
	int nbSecondes;

} DONNEE;


DONNEE data[] = { {"MATAGNE", 15},
                  {"WILVERS", 10},
                  {"WAGNER", 17},
                  {"QUETTIER", 8},
                  {"", 0} };


DONNEE Param;


int main()
{
	int res;

	printf("\nJe suis le thread_PRINCIPAL(%lu) ALIAS le main()\n\n",pthread_self());
	
	for(int i=0;i<5;i++)
	{
		memcpy(&Param,&data[i],sizeof(DONNEE)) ;
		if(Param.nom !=NULL)
		{
			res=pthread_create(&tidSecondTAB[i],NULL,threads_SECOND,&Param);
			if(res==0){printf("threadScond%d(%lu) cree avec succe\n\n",i+1,tidSecondTAB[i]);}

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
	struct timespec temps;
	DONNEE *tmp= (DONNEE*)Sptr;

	printf("Je suis le threads_SECOND %d.%lu lancé (PID = %d et TID = %lu)l\n",getpid(),pthread_self(),getpid(),pthread_self());
	printf("Le nom passe en paramettre est %s\n",tmp->nom);

	temps.tv_sec=tmp->nbSecondes; temps.tv_nsec=0;
	nanosleep(&temps,NULL);
	printf("Je suis le threads_SECOND %d.%lu se termine\n\n",getpid(),pthread_self());
	pthread_exit(NULL);
}