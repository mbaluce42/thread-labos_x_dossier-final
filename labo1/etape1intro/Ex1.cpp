#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>

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
		
		res=pthread_join(tid,(void**)&pbrecup);
		
		if(res==0)
		{
			printf("thread supprimé avec succes\n");
			printf("affichage de l'entier que le thread cree dans ca fct()sans paramettre: %d\n",*pbrecup);	
			free(pbrecup);
		}
		//res=pthread_detach(tid);
		
	}
	
	return 0;
}
void *fct(void *)
{
	int *pa= (int*)malloc(sizeof(int));
	*pa=243;
	pthread_exit(pa);


}