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

#define POS_CORBEAU 0.7 //s //changement position horiz

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
int up=0;

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
		int nbEchecs=0;

		struct sigaction s;
	s.sa_flags=0;
	sigemptyset(&s.sa_mask);
	s.sa_handler=HandlerSIGQUIT;
	sigaction(SIGQUIT,&s,NULL);

	sigemptyset(&mask); 
	sigaddset(&mask, SIGQUIT);
	sigprocmask(SIG_BLOCK,&mask,NULL);


	int res,tmp;

	ouvrirFenetreGraphique();

	/*afficherCage(2);
	afficherCage(3);
	afficherCage(4);*/

	//afficherRireDK();

	res=pthread_create(&threadCle,NULL,FctThreadCle,NULL);
	if(res==0){printf("\nThreadCle(%lu) cree avec succe\n",threadCle); }
	
	tmp=EVEN_SEC;
	res=pthread_create(&threadEvenements,NULL,FctThreadEvenements,&tmp);
	if(res==0){printf("\nThreadEvenements(%lu) cree avec succe\n",threadEvenements); }
	
	res=pthread_create(&threadDK,NULL,FctThreadDK,NULL);
	if(res==0){printf("\nThreadDK(%lu) cree avec succe\n",threadDK); }
	
	res=pthread_create(&threadScore,NULL,FctThreadScore,NULL);
	if(res==0){printf("\nThreadScore(%lu) cree avec succe\n",threadScore); }
	
	while(nbEchecs<3)
	{
		res=pthread_create(&threadDKJr,NULL,FctThreadDKJr,NULL);
		if(res==0){printf("\nThreadDKJr(%lu) cree avec succe\n",threadDKJr); }
		
		res=pthread_join(threadDKJr,NULL);
		if(res==0){printf("\nthreadDKJr(%lu) fini, arrete avec succes",threadDKJr);
		nbEchecs++;
		afficherEchec(nbEchecs);}

	}

	/*afficherCroco(11, 2);
	afficherCroco(17, 1);
	afficherCroco(0, 3);
	afficherCroco(12, 5);
	afficherCroco(18, 4);*/

	/*afficherDKJr(11, 9, 1);
	afficherDKJr(6, 19, 7);
	afficherDKJr(0, 0, 9);*/

	/*afficherCorbeau(10, 2);
	afficherCorbeau(16, 1);
	
	effacerCarres(9, 10, 2, 1);

	afficherEchec(1);*/
	res=pthread_join(threadEvenements,NULL);
	if(res==0){printf("\nthreadEvenements(%lu) fini, arrete avec succes",threadEvenements);}

	res=pthread_join(threadCle,NULL);
	if(res==0){printf("\nthreadCle(%lu) fini, arrete avec succes\n",threadCle);}

	res=pthread_join(threadScore,NULL);
	if(res==0){printf("\nthreadScore(%lu) fini, arrete avec succes",threadScore);}
	
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
	
	srand(time(NULL));
	int ennemi;
	alarm(SIGALRM_SEC);//init alarm
	ennemi=rand()%2+1;

	while(1)
	{
		if(ennemi==1)
		{
			pthread_create(&threadCorbeau, NULL, FctThreadCorbeau, NULL);
		}
		else
		{
			pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);		
		}
		nanosleep(&temps,NULL);
	}
}

void* FctThreadCorbeau(void* Setting)
{
	struct timespec temps;
	float tmp=POS_CORBEAU;
	temps.tv_sec = (time_t)(tmp); // partie entière de la durée en sec
	temps.tv_nsec = (long)((tmp - temps.tv_sec) * 1e9); // partie décimale convertie en nanose


}
void HandlerSIGQUIT(int sig)
{
	printf("thread(%lu) a recu le signal n°%d\n",pthread_self(), sig );	
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
void move_LEFT()
{
	if (positionDKJr > 1 && etatDKJr==LIBRE_BAS)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr--;
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr>3)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr--;
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(7, (positionDKJr * 2) + 7, ((6-positionDKJr)%4)+1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr==3)
	{
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
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
	}
	else if(etatDKJr==LIBRE_HAUT && positionDKJr<7)
	{
		setGrilleJeu(3, positionDKJr);
		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		positionDKJr++;

		setGrilleJeu(3, positionDKJr, DKJR);
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
		setGrilleJeu(3, positionDKJr);
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		setGrilleJeu(3, positionDKJr, DKJR);
		afficherDKJr(10, (positionDKJr * 2) + 7,  8);

		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);

		effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
		afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
	}
	else if( (positionDKJr==1 || positionDKJr==5) && etatDKJr==LIBRE_BAS)
	{
		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
		afficherDKJr(10, (positionDKJr * 2) + 7, 7);

		etatDKJr=LIANE_BAS;
	}

	else if(positionDKJr== 7) 
	{
		if (etatDKJr==LIBRE_BAS)
		{
			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(10,(positionDKJr * 2) + 7, 5);
			etatDKJr=DOUBLE_LIANE_BAS;
		}
		
		
		else if(etatDKJr==DOUBLE_LIANE_BAS)
		{
			effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(7,(positionDKJr * 2) + 7, 6);
			etatDKJr=LIBRE_HAUT;
		}
	}
	else if(etatDKJr==LIBRE_HAUT)
	{
		if(positionDKJr==6 )
		{
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(6,(positionDKJr * 2) + 7, 7);
			etatDKJr=LIANE_HAUT;
		}
		else if(positionDKJr==3 || positionDKJr==4)
		{
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(6,(positionDKJr * 2) + 7, 8);
			pthread_mutex_unlock(&mutexGrilleJeu);
			nanosleep(&temps,NULL);
			pthread_mutex_lock(&mutexGrilleJeu);

			effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(7,(positionDKJr * 2) + 7, ((6-positionDKJr ) % 4) + 1);
		}
		
	}
	printf("\n\npositionDKJr=%d\n\n",positionDKJr);
	
}

void move_DOWN()
{
		if(etatDKJr== LIANE_BAS )
		{
			effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
			afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
			etatDKJr=LIBRE_BAS;
		}

		else if(etatDKJr== DOUBLE_LIANE_BAS)
		{
			effacerCarres(10, (positionDKJr * 2) + 7, 3, 2);
			afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) +1);
			etatDKJr=LIBRE_BAS;
		}
		else if(etatDKJr== LIBRE_HAUT && positionDKJr==7)
		{
			effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
			afficherDKJr(10,(positionDKJr * 2) + 7, 5);
			etatDKJr=DOUBLE_LIANE_BAS;
		}
		else if(etatDKJr==LIANE_HAUT)
		{
			effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
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
	
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

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

		effacerCarres(6, (positionDKJr * 2)+7,2, 2);
		afficherDKJr(11,(positionDKJr *2) +7, 13);
		
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
	
		positionDKJr--;
		
		effacerCarres(11, (positionDKJr*2)+5, 2, 2);

		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);

		pthread_exit(NULL);
	}
	else
	{
    	//setGrilleJeu(1, positionDKJr);
 
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

        //positionDKJr = 1;
        setGrilleJeu(1, positionDKJr, DKJR);
        afficherDKJr(11, 9, 1);                      
        etatDKJr = LIBRE_BAS;
	}
}