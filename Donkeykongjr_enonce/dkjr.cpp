#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define CLE_SEC 0.7 //s
#define EVEN_SEC 100 //ms
#define SAUT_DKJnr_SEC 1.4//s

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

void move_LEFT();
void move_RIGHT();
void move_UP();

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

/*typedef struct
{
	int nbSecondes;
	int nbNanoSecondes;

}CLE_setting;*/

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
		sigset_t mask;

		struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGQUIT;
	sigaction(SIGQUIT,&s,NULL);

	sigemptyset(&mask); 
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_BLOCK,&mask,NULL);


	int res,tmp;
	float cle_sec = CLE_SEC;

	ouvrirFenetreGraphique();

	afficherCage(2);
	afficherCage(3);
	afficherCage(4);

	afficherRireDK();

	res=pthread_create(&threadCle,NULL,FctThreadCle,&cle_sec);
	if(res==0){printf("\nThreadCle(%lu) cree avec succe\n",threadCle); }


	/*afficherCroco(11, 2);
	afficherCroco(17, 1);
	afficherCroco(0, 3);
	afficherCroco(12, 5);
	afficherCroco(18, 4);*/

	/*afficherDKJr(11, 9, 1);
	afficherDKJr(6, 19, 7);
	afficherDKJr(0, 0, 9);*/

	res=pthread_create(&threadDKJr,NULL,FctThreadDKJr,NULL);
	if(res==0){printf("\nThreadDKJr(%lu) cree avec succe\n",threadDKJr); }
	
	afficherCorbeau(10, 2);
	afficherCorbeau(16, 1);
	
	effacerCarres(9, 10, 2, 1);

	afficherEchec(1);
	afficherScore(1999);


	tmp=EVEN_SEC;
	res=pthread_create(&threadEvenements,NULL,FctThreadEvenements,&tmp);
	if(res==0){printf("\nThreadEvenements(%lu) cree avec succe\n",threadEvenements); }

	res=pthread_join(threadCle,NULL);
	if(res==0){printf("\nthreadCle(%lu) fini, arrete avec succes\n",threadCle);}
	res=pthread_join(threadEvenements,NULL);
	if(res==0){printf("\nthreadEvenements(%lu) fini, arrete avec succes",threadEvenements);}


	return 0;

	
}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}
//-----------------------------------------

void* FctThreadCle(void *Setting)
{
	int i,j,fin=0;
	float *tmp= (float*)Setting; //sec

	//struct timespec temps={(time_t)(*tmp),(long)((*tmp - temps.tv_sec) * 1e9)};
	struct timespec temps;
	temps.tv_sec = (time_t)(*tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((*tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanosec
	
		while(1)
		{
			for(i=4;i>0 ;i--)
			{
				afficherCle(i);
				pthread_mutex_lock(&mutexGrilleJeu);
				if(i==1){setGrilleJeu(0,1,CLE,pthread_self());}
				else{setGrilleJeu(0,1,VIDE,pthread_self());}
				pthread_mutex_unlock(&mutexGrilleJeu);
				nanosleep(&temps,NULL);
				effacerCarres(3,12,2,3);
			}

			for(j=i+2;i==0 && j<=4;j++)
				{
					afficherCle(j);
					pthread_mutex_lock(&mutexGrilleJeu);
					setGrilleJeu(0,1,VIDE,pthread_self());
					pthread_mutex_unlock(&mutexGrilleJeu);
					nanosleep(&temps,NULL);
					effacerCarres(3,12,2,3);
				}
		}
	
	pthread_exit(NULL);

}

void* FctThreadEvenements(void *Setting)
{
	
	int evt;
	int *t= (int *)Setting; //ms
	struct timespec temps;
	temps.tv_sec=0; temps.tv_nsec= (time_t)((*t)* 1e6);//convert en nanosec

	while (1)
	{
	   pthread_mutex_lock(&mutexEvenement);
	   evenement=AUCUN_EVENEMENT;
	   pthread_mutex_unlock(&mutexEvenement);

	   evt = lireEvenement();

	   switch (evt)
	   {
	   		case SDL_QUIT:
					exit(0);
				case SDLK_UP:
					printf("KEY_UP\n");
					break;
				case SDLK_DOWN:
					printf("KEY_DOWN\n");
					break;
				case SDLK_LEFT:
					printf("KEY_LEFT\n");
					break;
				case SDLK_RIGHT:
					printf("KEY_RIGHT\n");
					break;
	   }
	   if(evt==SDLK_UP || evt==SDLK_DOWN || evt==SDLK_LEFT || evt==SDLK_RIGHT ) 	
	   {
	    	pthread_mutex_lock(&mutexEvenement);
	    	evenement=evt;
	    	pthread_mutex_unlock(&mutexEvenement);
	    	//printf("\n\n KILLLLLLLL\n");
	    	pthread_kill(threadDKJr,SIGQUIT);//va permettre de reveiller le DKJr qui est sur pause
	   }
	   //clock_gettime(CLOCK_MONOTONIC, &debut); // obtenir le temps de début 
	   nanosleep(&temps,NULL);
	   /*clock_gettime(CLOCK_MONOTONIC, &fin); // obtenir le temps de fin 
			double duree = (fin.tv_sec - debut.tv_sec) + (fin.tv_nsec - debut.tv_nsec) / 1e9; // calculer la durée en secondes 
			printf("La durée du nanosleep est de %f secondes.\n", duree); // afficher la durée*/
	}
	pthread_exit(NULL);

}

void* FctThreadDKJr(void * Setting)
{
	sigset_t mask;

	sigemptyset(&mask); 
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_UNBLOCK,&mask,NULL);

	bool on = true; 
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, 1, DKJR); 
	afficherDKJr(11, 9, 1); 
	etatDKJr = LIBRE_BAS; 
	positionDKJr = 1;
	pthread_mutex_unlock(&mutexGrilleJeu);
	while (on)
	{
		printf("avant kill \n");

		pause();
		printf("apres kill \n");
		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);
		switch (etatDKJr)
		{
			case LIBRE_BAS:
			switch (evenement)
			{
				case SDLK_LEFT:
				move_LEFT();
				break;

				case SDLK_RIGHT:
				move_RIGHT();
				break;

				case SDLK_UP:
				move_UP();
				break;
			}
			case LIANE_BAS:
			
			break;
			case DOUBLE_LIANE_BAS:
			
			break;
			case LIBRE_HAUT:
			
			break;
			case LIANE_HAUT:
			 break;

		}
		pthread_mutex_unlock(&mutexEvenement);
		pthread_mutex_unlock(&mutexGrilleJeu);
	}
	pthread_exit(NULL);
}


void HandlerSIGQUIT(int sig)
{

	printf("thread(%lu) a recu le signal n°%d\n",pthread_self(), sig );	
}


void move_LEFT()
{
	if (positionDKJr > 1)
	{
		//void afficherDKJr(int ligne, int colonne, int num);
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr--;
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
	}

}

void move_RIGHT()
{
	if (positionDKJr < 7)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr++;
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
	}

}

void move_UP()
{
	if(positionDKJr== 2 || positionDKJr==3 || positionDKJr==4 || positionDKJr==6 )
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(10, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);

		sleep(1);
				
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
	}
	else if(positionDKJr==1 || positionDKJr==5)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(10, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 7);

		sleep(1);
				
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);

	}
	else
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(10, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 3);

		sleep(1);
				
		effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);

	}

}
