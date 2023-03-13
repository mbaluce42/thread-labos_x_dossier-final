#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define caractere 6
int cpt=0;
void *fct(void *);
pthread_t tid;

int main()
{
	int res;
	int *pbrecup;

	res=pthread_create(&tid,NULL,fct,NULL);
	if(res==0)
	{
		printf("thread cree avec succe\n");	
		
		res=pthread_join(tid,NULL);
		if(res==0){printf("thread supprimé,arreté avec succes\n");}
		printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",cpt);
	}
	
	return 0;
}
void *fct(void *)
{
	//struct timespec_t temps; 
	int i=0,res;
	int fd;
	char mot[caractere+1];
	mot[caractere]='\0';

	while(1)
	{
		fd=open("printf.txt",O_RDWR);
		if(fd==-1){perror("Erreur de open(): ");break;}
		
		if(lseek(fd,i,SEEK_SET)==-1){break;};//si ==-1 alors fin de fichier
		if((res=read(fd,mot,caractere))==-1){perror("Erreur de read(): ");exit(-1);}
		else if(res==0){break;}
		else
		{
			close(fd);
			if(strcmp(mot,"printf")==0){cpt++;}
			printf("*");
			fflush(stdout);
			i++;
			usleep(2000);
		}

	}

	/*int *pa= (int*)malloc(sizeof(int));
	*pa=243;*/
	pthread_exit(NULL);
	/*temps.tv_sec = 5;temps.tv_nsec = 0; 
 	nanosleep(&temps, NULL); */


}