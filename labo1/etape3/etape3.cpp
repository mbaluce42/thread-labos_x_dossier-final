#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void *fctglob(void *Sptr);
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
	int *pRecup;
	threadsetting s[4];
	for(int i=0;i<4;i++)
	{
		strcpy(s[i].filename,"printf.txt");
		strcpy(s[i].wordFind,"printf");
		s[i].tab=i;
		res=pthread_create(&tidTAB[i],NULL,fctglob,&s[i]);
		if(res==0){printf("thread%d cree avec succe\n",i+1);}
	}
	
	/*res=pthread_create(&tidTAB[0],NULL,fctglob,&s[1]);
	if(res==0){printf("thread2 cree avec succe\n");}

	res=pthread_create(&tidTAB[0],NULL,fctglob,&s[2]);
	if(res==0)
	{printf("thread3 cree avec succe\n");}

	res=pthread_create(&tidTAB[0],NULL,fctglob,&s[3]);
	if(res==0)
	{printf("thread4 cree avec succe\n");}*/

	res=pthread_join(tidTAB[0],(void**)&pRecup);
	if(res==0){printf("\nthread1 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",*pRecup);
	free(pRecup);
	

	res=pthread_join(tidTAB[1],(void**)&pRecup);
	if(res==0){printf("\nthread2 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",*pRecup);
	free(pRecup);

	res=pthread_join(tidTAB[2],(void**)&pRecup);
	if(res==0){printf("\nthread3 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",*pRecup);
	free(pRecup);

	res=pthread_join(tidTAB[3],(void**)&pRecup);
	if(res==0){printf("\nthread4 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",*pRecup);

	free(pRecup);

	
	return 0;
}
void *fctglob(void *Sptr)
{
	threadsetting *tmp= (threadsetting*)Sptr;
	int move=0,res;
	char word[50];
	int fd;
	int *cpt= (int*)malloc(sizeof(int));
	*cpt=0;
	while(1)
	{
		fd=open("printf.txt",O_RDWR);
		if(fd==-1){perror("Erreur de open(): ");break;}
		if(lseek(fd,move,SEEK_SET)==-1){break;};//si ==-1 alors fin de fichier
		if((res=read(fd,&word,strlen(tmp->wordFind)))==-1){perror("Erreur de read(): ");exit(-1);}
		else if(res==0){break;}
		else
		{
			close(fd);
			if(strcmp(word,tmp->wordFind)==0){(*cpt)++;}
			for(int j=0;j<tmp->tab;j++){printf("\t");}
			printf("*\n");
			move++;
			usleep(5000);
		}
	}

	pthread_exit(cpt);
}