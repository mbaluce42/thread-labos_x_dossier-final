#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>     
      
#define BUFFER_SIZE 12
#define SHM_NAME "/shm_procon"
#define EMPTY_SEM "/empty"
#define FULL_SEM "/full"
#define MUTEX_SEM "/mutex"

typedef struct {
    char buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_data_t;

typedef struct
{
	shared_data_t* shared_data;
	sem_t* empty;
	sem_t* full;
	sem_t* mutex;
	char start_char='a'; //a or A
	int upper; //0=min or 1=maj

} allSetting;




pthread_t thread_producteur_1,thread_producteur_2,thread_consommateur;


void *producteur(void* settings/*char start_char, sem_t* empty, sem_t* full, sem_t* mutex, shared_data_t* shared_data*/);
void *consommateur(void* settings/*sem_t* empty, sem_t* full, sem_t* mutex, shared_data_t* shared_data*/);
void afficher_buffer(shared_data_t* shared_data);

int main() {
	allSetting setting;
    // Créer la mémoire partagée
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0660);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    /*shared_data_t* shared_data*/setting.shared_data = (shared_data_t*)mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (setting.shared_data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(setting.shared_data, '&', sizeof(shared_data_t));  // initialiser le buffer à '&'
    
    // Créer les sémaphores
    /*sem_t* empty*/setting.empty = sem_open(EMPTY_SEM, O_CREAT | O_RDWR, 0660, BUFFER_SIZE);
    if (setting.empty == SEM_FAILED) {
        perror("sem_open empty");
        exit(1);
    }
    /*sem_t* full*/setting.full = sem_open(FULL_SEM, O_CREAT | O_RDWR, 0660, 0);
    if (setting.full == SEM_FAILED) {
        perror("sem_open full");
        exit(1);
    }
    /*sem_t* mutex*/setting.mutex = sem_open(MUTEX_SEM, O_CREAT | O_RDWR, 0660, 1);
    if (setting.mutex == SEM_FAILED) {
        perror("sem_open mutex");
        exit(1);
    }
    
    // Créer les threads producteurs et le thread consommateur
    pthread_t thread_producteur_1, thread_producteur_2, thread_consommateur;
    if (pthread_create(&thread_producteur_1, NULL, producteur,&setting) /*(void*)('a'), (void*)empty, (void*)full, (void*)mutex, (void*)shared_data) */!= 0) {
        perror("pthread_create producteur 1");
        exit(1);
    }
    if (pthread_create(&thread_producteur_2, NULL, producteur, &setting)/*(void*)('A'), empty, full, mutex, shared_data)*/ != 0) {
        perror("pthread_create producteur 2");
        exit(1);
    }
    if (pthread_create(&thread_consommateur, NULL, /*(void* (*)(void*))*/consommateur, &setting)/*empty, full, mutex, shared_data)*/ != 0) {
        perror("pthread_create consommateur");
        exit(1);
    }
    
    // Attendre la fin des threads
    if (pthread_join(thread_producteur_1, NULL) != 0)
    {
    	perror("pthread_join consommateur");
    }

    if (pthread_join(thread_producteur_2, NULL) != 0)
    {
    	perror("pthread_join consommateur");
    }

    if (pthread_join(thread_consommateur, NULL) != 0)
    {
    	perror("pthread_join consommateur");
    }

    sem_unlink(EMPTY_SEM);
    sem_unlink(MUTEX_SEM);
    sem_unlink(FULL_SEM);
    shmctl(shm_fd,IPC_RMID,NULL);

}


void* producteur(void* settings)/*char start_char, sem_t* empty, sem_t* full, sem_t* mutex, shared_data_t* shared_data*/ {
    allSetting* tmp=(allSetting*)settings;
    tmp->start_char='a';
    char next_char = tmp->start_char;
       	while (1) {
        usleep(rand() % 1000000);  // attendre un temps aléatoire
        
        sem_wait(tmp->empty);  // attendre qu'il y ait de la place dans le buffer
        sem_wait(tmp->mutex);  // verrouiller l'accès au buffer
        
        // ajouter le prochain caractère dans le buffer
        tmp->shared_data->buffer[tmp->shared_data->in] = next_char;
        tmp->shared_data->in = (tmp->shared_data->in + 1) % BUFFER_SIZE;
        
        printf("Producteur: ajouté %c\n", next_char);
        
        next_char++;
        if (next_char > 'z') {
            next_char = 'a';
        }
        
        sem_post(tmp->mutex);  // déverrouiller l'accès au buffer
        sem_post(tmp->full);   // indiquer qu'il y a un élément de plus dans le buffer
    }
}

void* consommateur(void* settings/*sem_t* empty, sem_t* full, sem_t* mutex, shared_data_t* shared_data*/) {
    allSetting* tmp=(allSetting*)settings;
    while (1) {
        usleep(rand() % 1000000);  // attendre un temps aléatoire
        
        sem_wait(tmp->full);   // attendre qu'il y ait un élément dans le buffer
        sem_wait(tmp->mutex);  // verrouiller l'accès au buffer
        
        // consommer le prochain caractère dans le buffer
        char c = tmp->shared_data->buffer[tmp->shared_data->out];
        tmp->shared_data->buffer[tmp->shared_data->out] = '&';
        tmp->shared_data->out = (tmp->shared_data->out + 1) % BUFFER_SIZE;
        
        printf("Consommateur: consommé %c\n", c);
        
        sem_post(tmp->mutex);  // déverrouiller l'accès au buffer
        sem_post(tmp->empty);  // indiquer qu'il y a de la place de libérée dans le buffer
    }
}

void afficher_buffer(shared_data_t* shared_data) {
    printf("Buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (shared_data->buffer[i] == '&') {
            printf("_ ");
        } else {
            printf("%c ", shared_data->buffer[i]);
        }
    }
    printf("\n");
}