#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h" //test

#define CLE_SEC 0.7 //s
#define EVEN_SEC 100 //ms
#define SAUT_DKJnr_SEC 1.4//s
#define SAUT_FINAL_DKJnr_SEC 0.23//s
#define RIRE_DK 0.7//s

#define DEFAUT_ENNEMIS_SEC 4 //s
#define DIMINUE_ENNIMIS_SEC 0.25 //s 
#define SIGALRM_SEC 15//sec //difficulte augmente avec le temps
#define MIN_DELAI_SEC 2.5 //s

#define CORBEAU_SEC 0.7 //s
#define CROCO_SEC 0.7 //s

struct timespec SEC_1{0,(long)(0.5*1e9)};


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
void move_DOWN();
void Try_Key_DKjr();
void effaceRire();
bool corbeauPresent(int);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;
pthread_t threadCorbeau;
pthread_t threadCroco;


pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = false;
float delaiEnnemis = DEFAUT_ENNEMIS_SEC;//ms
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;
bool tryDKjr=false;

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



// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	sigset_t mask;
	int nbEchecs=0,res,tmp;

	struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGQUIT;
	sigaction(SIGQUIT,&s,NULL);

	sigemptyset(&mask); 
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_BLOCK,&mask,NULL);

	ouvrirFenetreGraphique();

	res=pthread_create(&threadCle,NULL,FctThreadCle,NULL);
	if(res==0){printf("\nThreadCle(%lu) cree avec succe\n",threadCle); }
	
	tmp=EVEN_SEC;
	res=pthread_create(&threadEvenements,NULL,FctThreadEvenements,&tmp);
	if(res==0){printf("\nThreadEvenements(%lu) cree avec succe\n",threadEvenements); }
	
	res=pthread_create(&threadDK,NULL,FctThreadDK,NULL);
	if(res==0){printf("\nThreadDK(%lu) cree avec succe\n",threadDK); }
	
	res=pthread_create(&threadScore,NULL,FctThreadScore,NULL);
	if(res==0){printf("\nThreadScore(%lu) cree avec succe\n",threadScore); }
	
	res=pthread_create(&threadEnnemis,NULL,FctThreadEnnemis,NULL);
	if(res==0){printf("\nThreadEnnemis(%lu) cree avec succe\n",threadEnnemis); }

	while(nbEchecs<3)
	{
		res=pthread_create(&threadDKJr,NULL,FctThreadDKJr,NULL);
		if(res==0){printf("\nThreadDKJr(%lu) cree avec succe\n",threadDKJr); }
			
		res=pthread_join(threadDKJr,NULL);
		if(res==0){printf("\nthreadDKJr(%lu) fini, arrete avec succes",threadDKJr);}		
		nbEchecs++;
		afficherEchec(nbEchecs);
		nanosleep(&SEC_1,NULL);	
	}
	pthread_cancel(threadEnnemis);
	pthread_key_delete(keySpec);
	pthread_cancel(threadCle);
	pthread_cancel(threadScore);
	pthread_cancel(threadCorbeau);
	pthread_cancel(threadCroco);
	res=pthread_join(threadCle,NULL);
	if(res==0){printf("\nthreadCle(%lu) fini, arrete avec succes\n",threadCle);}

	res=pthread_join(threadScore,NULL);
	if(res==0){printf("\nthreadScore(%lu) fini, arrete avec succes",threadScore);}
	
	res=pthread_join(threadEnnemis,NULL);
	if(res==0){printf("\nthreadEnnemis(%lu) fini, arrete avec succes",threadEnnemis);}

	res=pthread_join(threadEvenements,NULL);
	if(res==0){printf("\nthreadEvenements(%lu) fini, arrete avec succes",threadEvenements);}

	return 0;
}

// -------------------------------------
void effaceRire()
{
	effacerCarres(3,8,2,2);
}
//--------------------------------------
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
	float tmp= CLE_SEC;

	//struct timespec temps={(time_t)(*tmp),(long)((*tmp - temps.tv_sec) * 1e9)};
	struct timespec temps;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanosec
	
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
	    	pthread_mutex_lock(&mutexEvenement);
	    	evenement=evt;
	    	pthread_mutex_unlock(&mutexEvenement);
	    	//printf("\n\n KILLLLLLLL\n");
	    	pthread_kill(threadDKJr,SIGQUIT);//va permettre de reveiller le DKJr qui est sur pause

	   nanosleep(&temps,NULL);
	   }
	pthread_exit(NULL);

}

void* FctThreadDKJr(void * Setting)
{
	sigset_t mask;

	sigemptyset(&mask); 
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_UNBLOCK,&mask,NULL);

	struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGINT;
	sigaction(SIGINT,&s,NULL);

	s.sa_handler=HandlerSIGHUP;
	sigaction(SIGHUP,&s,NULL);

	s.sa_handler=HandlerSIGCHLD;
	sigaction(SIGCHLD,&s,NULL);

	float *tmp= (float*)Setting; //sec
	//struct timespec temps={(time_t)(*tmp),(long)((*tmp - temps.tv_sec) * 1e9)};

	bool on = true; 
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, 1, DKJR); 
	afficherDKJr(11, 9, 1); 
	etatDKJr = LIBRE_BAS; 
	positionDKJr = 1;
	pthread_mutex_unlock(&mutexGrilleJeu);
	while (on)
	{
		pause();

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
				if(evenement== SDLK_DOWN)
				{
					move_DOWN();
				}
			break;
			case DOUBLE_LIANE_BAS:
				switch(evenement)
				{
					 case SDLK_UP:
					 	move_UP();
					 	break;
					 case SDLK_DOWN:
					 	move_DOWN();
					 	break;
				}
			break;
			case LIBRE_HAUT:
				switch(evenement)
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
					case SDLK_DOWN:
						move_DOWN();
						break;
				}
			break;
			case LIANE_HAUT:
				if(evenement==SDLK_DOWN)
				{
					move_DOWN();
				}			
			break;

		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(NULL);
}

void* FctThreadDK(void *)
{
	struct timespec temps;
	float tmp= RIRE_DK;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanosec
	
    int i=1;
    
	afficherCage(2);//1
	afficherCage(1);//2
    afficherCage(3);
    afficherCage(4);
	while (true)
	{
		pthread_mutex_lock(&mutexDK);
		while (!MAJDK) 
		{
			printf("\n\navant wait\n\n");
			pthread_cond_wait(&condDK, &mutexDK);
			printf("\n\napres wait\n\n");
			if(i==1){effacerCarres(2,9,2,2); i++;}
			else if(i==2){effacerCarres(2,7,2,2);i++;}
			else if(i==3){effacerCarres(4,7,2,2);;i++;}
			else if(i==4)
			{
				effacerCarres(4,9,4,3);
				afficherRireDK();
				nanosleep(&temps,NULL);
				effaceRire();
				afficherCage(1); afficherCage(2); afficherCage(3); afficherCage(4);
				i=1;
			}
		}
		MAJDK=false;
		pthread_mutex_unlock(&mutexDK);
	}
    //pthread_mutex_unlock(&mutexDK);
}

void* FctThreadScore(void* Setting)
{
	while (1)
	{
		pthread_mutex_lock(&mutexScore);
		while (!MAJScore)
		{
			pthread_cond_wait(&condScore, &mutexScore);
			afficherScore(score);
		}
		MAJScore = false;
		pthread_mutex_unlock(&mutexScore);

	}
	
	
}

void* FctThreadEnnemis(void* Setting)
{
	struct sigaction al;
	al.sa_flags=0;
	sigemptyset(&al.sa_mask);
	al.sa_handler=HandlerSIGALRM;
	sigaction(SIGALRM,&al,NULL);

	struct timespec temps;
	float tmp=delaiEnnemis;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanose
	pthread_key_create(&keySpec,DestructeurVS);
	srand(time(NULL));
	int ennemi,res;
	alarm(SIGALRM_SEC);//init alarm
	while(1)
	{
		ennemi=rand()%2+1;
		if(ennemi==1)
		{
			
			pthread_create(&threadCorbeau, NULL, FctThreadCorbeau, NULL);
			if(res==0){printf("\nthreadCorbeau(%lu) cree avec succe\n",threadCorbeau);}
			
		}
		else
		{
			pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);	
			if(res==0){printf("\nthreadCroco,(%lu) cree avec succe\n",threadCroco);}
			sleep(2);

		}
		nanosleep(&temps,NULL);
	}

	pthread_exit(NULL);
}

void* FctThreadCorbeau(void* Setting)
{
	struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGUSR1;
	sigaction(SIGUSR1,&s,NULL);



	struct timespec temps;
	float tmp=CORBEAU_SEC;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanose
	
	int* positionCorbeau = (int*) malloc(sizeof(int));
	*positionCorbeau=0;

	for(int i=0;i<8;i++)
	{
		pthread_setspecific(keySpec, positionCorbeau);
		
		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(2,(*positionCorbeau),CORBEAU,pthread_self());
		pthread_mutex_unlock(&mutexGrilleJeu);
		if(i==0||i%2==0){afficherCorbeau(((*positionCorbeau)*2)+8,2);}
		else{afficherCorbeau(((*positionCorbeau)*2)+8,1);}		
		pthread_mutex_lock(&mutexGrilleJeu);
		if(grilleJeu[2][(*positionCorbeau)+1].type==DKJR)
		{
			if(i==0 && tryDKjr==true&& etatDKJr==LIANE_BAS){tryDKjr=false; goto here; }
			printf("\n\n(threadCorbeau)CORBEAU va heurter DKJr\n\n ");
			setGrilleJeu(2,(*positionCorbeau));
			pthread_kill(grilleJeu[2][positionDKJr].tid,SIGINT);
			nanosleep(&SEC_1,NULL);//pause pour traiter le signal
			effacerCarres(9,((*positionCorbeau)*2)+8,2);
			pthread_mutex_unlock(&mutexGrilleJeu);
			if(etatDKJr!=LIBRE_BAS){pthread_mutex_unlock(&mutexGrilleJeu);}
			pthread_exit(NULL);
		}
		else
		{
			here:
			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps,NULL);
			pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(2,*positionCorbeau);
			pthread_mutex_unlock(&mutexGrilleJeu);
			effacerCarres(9,((*positionCorbeau)*2)+8,2); 
			(*positionCorbeau)++;


		}
	}

	pthread_exit(NULL);

}

void* FctThreadCroco(void* Setting)
{
	struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGUSR2;
	sigaction(SIGUSR2,&s,NULL);



	struct timespec temps;
	float tmp=CROCO_SEC;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanose


	S_CROCO *croco=(S_CROCO *)malloc(sizeof(S_CROCO));
	croco->position = 2;
	croco->haut = true;
	

	for(int i=2;i<8;i++)
	{
		pthread_setspecific(keySpec, croco);

		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(1, croco->position,CROCO,pthread_self());
		pthread_mutex_unlock(&mutexGrilleJeu);
		afficherCroco(croco->position *2 +7,2-(i%2));
		pthread_mutex_lock(&mutexGrilleJeu);
		if(grilleJeu[1][(croco->position) +1].type==DKJR)
		{
			printf("\n\n(CROCO va heurter DKJr \n\n");
			setGrilleJeu(1, croco->position);
			pthread_kill(threadDKJr,SIGHUP);
			nanosleep(&SEC_1,NULL);
			effacerCarres(8, (croco->position )* 2 + 7, 1, 1);
			pthread_mutex_unlock(&mutexGrilleJeu);pthread_mutex_unlock(&mutexGrilleJeu);
			pthread_exit(NULL);
		}
		else
		{
			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps, NULL);
			pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(1, croco->position,VIDE);pthread_mutex_unlock(&mutexGrilleJeu);
			effacerCarres(8, (croco->position)* 2 + 7, 1, 1); (croco->position)++;
		}
	}
	afficherCroco(9, 3);
	nanosleep(&temps, NULL);
	effacerCarres(9,23, 1, 1);
	croco->position -=1;
	croco->haut = false;
	
	for(int i=7;i>0;i--)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(1, croco->position,CROCO,pthread_self());
		pthread_mutex_unlock(&mutexGrilleJeu);
		afficherCroco(croco->position *2 +8,5-(i%2));
		pthread_mutex_lock(&mutexGrilleJeu);
		if(grilleJeu[3][(croco->position)-1].type==DKJR)
		{
			if(i==1&& tryDKjr==true){tryDKjr=false; goto ici;}
			printf("\n\n(CROCO va heurter DKJr \n\n");
			setGrilleJeu(3, croco->position);
			pthread_kill(threadDKJr,SIGCHLD);
			nanosleep(&SEC_1,NULL);
			effacerCarres(12, (croco->position)* 2 + 8, 1, 1);
			pthread_mutex_unlock(&mutexGrilleJeu);pthread_mutex_unlock(&mutexGrilleJeu);
			pthread_exit(NULL);
		}
		else
		{
			ici:
			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps, NULL);
			pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(3, croco->position,VIDE);pthread_mutex_unlock(&mutexGrilleJeu);
			effacerCarres(12, (croco->position)* 2 + 8, 1, 1); (croco->position)--;
		}
	}
	pthread_exit(NULL);
}

void HandlerSIGQUIT(int sig)
{
	printf("thread(%lu) a recu le signal n°%d(SIGQUIT)\n",pthread_self(), sig );	
}

void HandlerSIGALRM(int sig)
{
	delaiEnnemis-= DIMINUE_ENNIMIS_SEC;
	if(delaiEnnemis<= MIN_DELAI_SEC)
	{ 
		printf("\n delai MIN atteint\n");
	}
	else{alarm(SIGALRM_SEC);}
}

void HandlerSIGUSR1(int sig)
{
	printf("\nthreadCorbeau(%lu) a recu le signal n°%d(SIGUSR1)\n",pthread_self(),sig);
	int* tmp = (int*)pthread_getspecific(keySpec);//pos corbeau
	setGrilleJeu(2,(*tmp));
	effacerCarres(9,((*tmp)*2)+8,2);
	pthread_exit(NULL);
}

void HandlerSIGUSR2(int sig)
{
	printf("\nthreadCROCO(%lu) a recu le signal n°%d(SIGUSR2)\n",pthread_self(),sig);
	pthread_exit(NULL);
}

void HandlerSIGINT(int sig)
{
	printf("\nthreadDKJr(%lu) a recu le signal n°%d(SIGINT)\n",pthread_self(),sig);
	setGrilleJeu(2,positionDKJr);
	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
	pthread_mutex_unlock(&mutexEvenement);
	pthread_exit(NULL);//threadDKJr
}

void HandlerSIGHUP(int sig)
{
	printf("\nthreadDKJr(%lu) a recu le signal n°%d(SIGHUP)\n",pthread_self(),sig);
	setGrilleJeu(1,positionDKJr);
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
	pthread_mutex_unlock(&mutexEvenement);
	pthread_exit(NULL);//threadDKJr

}

void HandlerSIGCHLD(int sig)
{
	printf("\nthreadDKJr(%lu) a recu le signal n°%d(SIGHUP)\n",pthread_self(),sig);
	setGrilleJeu(3,positionDKJr);
	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
	pthread_mutex_unlock(&mutexEvenement);
	pthread_exit(NULL);//threadDKJr

}

void DestructeurVS(void *p)
{
	free(p);
}

void move_LEFT()
{
	if (positionDKJr > 1 && etatDKJr==LIBRE_BAS)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr--;
		setGrilleJeu(3, positionDKJr, DKJR,pthread_self());
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr>3)
	{
		setGrilleJeu(1, positionDKJr);
		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr--;
		setGrilleJeu(1, positionDKJr, DKJR,pthread_self());
		afficherDKJr(7, (positionDKJr * 2) + 7, ((6-positionDKJr)%4)+1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr==3)
	{
		setGrilleJeu(1, positionDKJr);
		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		Try_Key_DKjr();
	}

	printf("\n\npositionDKJr=%d\n\n",positionDKJr);

}

void move_RIGHT()
{
	if (positionDKJr < 7 && etatDKJr==LIBRE_BAS)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr++;
		setGrilleJeu(3, positionDKJr, DKJR,pthread_self());
		afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr<7)
	{
		setGrilleJeu(1,positionDKJr);
		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr++;

		setGrilleJeu(1,positionDKJr, DKJR,pthread_self());
		if (positionDKJr== 7){afficherDKJr(7,(positionDKJr * 2) + 7, 6);}
		else{afficherDKJr(7,(positionDKJr * 2) + 7, ((6-positionDKJr ) % 4) + 1);}
	}
	printf("positionDKJr=%d\n\n",positionDKJr);

}

void move_UP()
{
	float sautDKjr_sec= SAUT_DKJnr_SEC;
	struct timespec temps;
	temps.tv_sec = (time_t)(sautDKjr_sec); // partie entière de la durée en sec
	temps.tv_nsec = (long)((sautDKjr_sec - temps.tv_sec) * 1e9); // partie décimale convertie en nanosec
	
	if( (positionDKJr== 2 || positionDKJr==3 || positionDKJr==4 || positionDKJr==6 ) && etatDKJr==LIBRE_BAS)//&& etatDKJr==LIBRE_BAS
	{
		if (!corbeauPresent(positionDKJr))
		{
			setGrilleJeu(3,positionDKJr);//met a vide avant de sauter
			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(2, positionDKJr,DKJR,pthread_self());//dkjr up
			afficherDKJr(10, (positionDKJr * 2) + 7,  8);

			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps,NULL);
			pthread_mutex_lock(&mutexGrilleJeu);

			setGrilleJeu(2,positionDKJr);//fin du up
			effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(3, positionDKJr,DKJR,pthread_self());
			afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
		}
		else//corbeau est en haut 
		{
			printf("\n (thread_DKJr)!! CORBEAU EN HAUT, CRASH !!\n");
			pthread_kill(grilleJeu[2][positionDKJr].tid,SIGUSR1);
			nanosleep(&SEC_1,NULL);//pause pour traiter le signal
			setGrilleJeu(3,(positionDKJr*2)+7);//efface
			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
			pthread_mutex_unlock(&mutexGrilleJeu); pthread_mutex_unlock(&mutexEvenement);
			pthread_exit(NULL);//threadDKJr
		}
	}
	else if( (positionDKJr==1 || positionDKJr==5) && etatDKJr==LIBRE_BAS)
	{
		if (!corbeauPresent(positionDKJr))
		{
			setGrilleJeu(3,positionDKJr);//met a vide avant de sauter
			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(2,positionDKJr,DKJR,pthread_self());
			afficherDKJr(10, (positionDKJr * 2) + 7, 7);

			etatDKJr=LIANE_BAS;
		}
		else//corbeau est en haut 
		{
			printf("\n (thread_DKJr)!! CORBEAU EN HAUT, CRASH !!\n");
			pthread_kill(grilleJeu[2][positionDKJr].tid,SIGUSR1);
			nanosleep(&SEC_1,NULL);//pause pour traiter le signal
			setGrilleJeu(3,(positionDKJr*2)+7);//efface
			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
			pthread_mutex_unlock(&mutexGrilleJeu); pthread_mutex_unlock(&mutexEvenement);
			pthread_exit(NULL);//threadDKJr
		}
	}

	else if(positionDKJr== 7) 
	{
		if (etatDKJr==LIBRE_BAS)
		{
			if (!corbeauPresent(positionDKJr))
			{
				pthread_mutex_unlock(&mutexGrilleJeu);
				setGrilleJeu(3,positionDKJr);//met a vide avant de sauter
				effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
				setGrilleJeu(2, positionDKJr,DKJR,pthread_self());
				afficherDKJr(10,(positionDKJr * 2) + 7, 5);
				etatDKJr=DOUBLE_LIANE_BAS;
			}
			else//corbeau est en haut 
			{
				printf("\n (thread_DKJr)!! CORBEAU EN HAUT, CRASH !!\n");
				pthread_kill(grilleJeu[2][positionDKJr].tid,SIGUSR1);
				nanosleep(&SEC_1,NULL);//pause pour traiter le signal
				setGrilleJeu(3,(positionDKJr*2)+7);//efface
				effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
				pthread_mutex_unlock(&mutexGrilleJeu); pthread_mutex_unlock(&mutexEvenement);
				pthread_exit(NULL);//threadDKJr
			}
		}
		
		
		else if(etatDKJr==DOUBLE_LIANE_BAS)
		{
			setGrilleJeu(2,positionDKJr);//met a vide avant de up
			effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(1, positionDKJr,DKJR,pthread_self());
			afficherDKJr(7,(positionDKJr * 2) + 7, 6);
			etatDKJr=LIBRE_HAUT;
		}
	}
	else if(etatDKJr==LIBRE_HAUT)
	{
		if(positionDKJr==6 )
		{
			setGrilleJeu(1,positionDKJr);//met a vide avant de sauter
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(0, positionDKJr,DKJR,pthread_self());
			afficherDKJr(6,(positionDKJr * 2) + 7, 7);
			etatDKJr=LIANE_HAUT;
		}
		else if(positionDKJr==3 || positionDKJr==4)
		{
			setGrilleJeu(1,positionDKJr);//met a vide avant de sauter
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(0, positionDKJr,DKJR,pthread_self());
			afficherDKJr(6,(positionDKJr * 2) + 7, 8);
			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps,NULL);
			pthread_mutex_lock(&mutexGrilleJeu);

			setGrilleJeu(0,positionDKJr);
			effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(1, positionDKJr,DKJR,pthread_self());
			afficherDKJr(7,(positionDKJr * 2) + 7, ((6-positionDKJr ) % 4) + 1);
		}
	}
	printf("\n\npositionDKJr=%d\n\n",positionDKJr);
	
}

void move_DOWN()
{
		if(etatDKJr== LIANE_BAS )
		{
			setGrilleJeu(2,positionDKJr);//met a vide avant de down
			effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
			setGrilleJeu(3, positionDKJr,DKJR,pthread_self());	
			afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
			etatDKJr=LIBRE_BAS;
		}

		else if(etatDKJr== DOUBLE_LIANE_BAS)
		{
			setGrilleJeu(2,positionDKJr);//met a vide avant de down
			effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
			setGrilleJeu(3, positionDKJr,DKJR,pthread_self());
			afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) +1);
			etatDKJr=LIBRE_BAS;
		}
		else if(etatDKJr== LIBRE_HAUT && positionDKJr==7)
		{
			setGrilleJeu(1,positionDKJr);//met a vide avant de down
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(2, positionDKJr,DKJR,pthread_self());
			afficherDKJr(10,(positionDKJr * 2) + 7, 5);
			etatDKJr=DOUBLE_LIANE_BAS;
		}
		else if(etatDKJr==LIANE_HAUT)
		{
			setGrilleJeu(0,positionDKJr);//met a vide avant de down
			effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
			setGrilleJeu(1, positionDKJr,DKJR,pthread_self());
			afficherDKJr(7,(positionDKJr * 2) + 7,1);
			etatDKJr=LIBRE_HAUT;
		}
		printf("\n\npositionDKJr=%d\n\n",positionDKJr);	
}

void Try_Key_DKjr()
{
	struct timespec temps;
	float sautFinal= SAUT_FINAL_DKJnr_SEC;
	temps.tv_sec = (time_t)(sautFinal); // partie entière de la durée en sec
	temps.tv_nsec = (long)((sautFinal - temps.tv_sec) * 1e9); // partie décimale convertie en nanosec

	if(grilleJeu[0][1].type== VIDE)
	{
		afficherDKJr(5,(positionDKJr*2)+7,9); //9
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
	
		effacerCarres(5, (positionDKJr*2)+7-1, 3,3);//3
		positionDKJr--;
		
		afficherDKJr(8,(positionDKJr*2)+7,12);//1 7

		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);

		effacerCarres(6,(positionDKJr * 2)+7,2, 2);
		afficherDKJr(11,(positionDKJr *2) +7, 13);
		
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
	
		positionDKJr--;
		
		effacerCarres(11, (positionDKJr*2)+5, 2, 2);

		pthread_mutex_unlock(&mutexGrilleJeu);pthread_mutex_unlock(&mutexEvenement);tryDKjr=true;
		pthread_exit(NULL);
	}
	else
	{
		afficherDKJr(5,(positionDKJr*2)+7,9);
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
        effacerCarres(5, (positionDKJr*2)+6, 3,3);
		positionDKJr--;
        afficherDKJr(3, (positionDKJr * 2)+6,10);
        pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);  
		pthread_mutex_lock(&mutexGrilleJeu);               
	    effacerCarres(3, (positionDKJr*2)+7,2,1);//haut dkJR
		afficherCage(4);
		effacerCarres(5, (positionDKJr*2)+6+1,2,2);//bas dkjr
		afficherCage(4);
		positionDKJr--;
		
        pthread_mutex_lock(&mutexDK);
        MAJDK=true;
		pthread_mutex_unlock(&mutexDK);
		pthread_cond_signal(&condDK);//reveil du threadDK

		pthread_mutex_lock(&mutexScore);
        MAJScore = true;
		score+=10;
        pthread_mutex_unlock(&mutexScore);
		pthread_cond_signal(&condScore);

        afficherDKJr(7, (positionDKJr * 2) + 6,11);
    	pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
        effacerCarres(6, 10, 2, 3);

        setGrilleJeu(1, positionDKJr, DKJR);
        afficherDKJr(11, 9, 1);                  
        etatDKJr = LIBRE_BAS;tryDKjr=true;
	}
}

bool corbeauPresent(int posDKJr)
{	
	if(grilleJeu[2][posDKJr].type==CORBEAU)
	{
		return true;
	}
	else
	{
		return false;
	}
}