#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define caractere 6

int cpt[5]{0,0,0,0};
void *fct1(void *);
void *fct2(void *);
void *fct3(void *);
void *fct4(void *);
pthread_t tid1,tid2,tid3,tid4;

int main()
{
	int res;
	int *pbrecup;

	res=pthread_create(&tid1,NULL,fct1,NULL);
	if(res==0){printf("thread1 cree avec succe\n");}

	res=pthread_create(&tid2,NULL,fct2,NULL);
	if(res==0){printf("thread2 cree avec succe\n");}

	res=pthread_create(&tid3,NULL,fct3,NULL);
	if(res==0)
	{printf("thread3 cree avec succe\n");}

	res=pthread_create(&tid4,NULL,fct4,NULL);
	if(res==0)
	{printf("thread4 cree avec succe\n");}

	res=pthread_join(tid1,NULL);
	if(res==0){printf("\nthread1 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",cpt[0]);

	res=pthread_join(tid2,NULL);
	if(res==0){printf("\nthread2 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",cpt[1]);

	res=pthread_join(tid3,NULL);
	if(res==0){printf("\nthread3 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",cpt[2]);

	res=pthread_join(tid4,NULL);
	if(res==0){printf("\nthread4 supprimé,arreté avec succes\n");}
	printf("le nbr d'occurance du mot:printf est present: %d dans le fichier \n",cpt[3]);

	

	return 0;
}
void *fct1(void *)
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
			if(strcmp(mot,"printf")==0){cpt[0]++;}
			printf("*\n");
			fflush(stdout);
			i++;
			usleep(5000);
		}
	}
	pthread_exit(NULL);
}

void *fct2(void *)
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
			if(strcmp(mot,"printf")==0){cpt[1]++;}
			printf("\t*\n");
			fflush(stdout);
			i++;
			usleep(5000);
		}
	}
	pthread_exit(NULL);
}

void *fct3(void *)
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
			if(strcmp(mot,"printf")==0){cpt[2]++;}
			printf("\t\t*\n");
			fflush(stdout);
			i++;
			usleep(5000);
		}
	}
	pthread_exit(NULL);
}

void *fct4(void *)
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
			if(strcmp(mot,"printf")==0){cpt[3]++;}
			printf("\t\t\t*\n");
			fflush(stdout);
			i++;
			usleep(5000);
		}
	}
	pthread_exit(NULL);
}