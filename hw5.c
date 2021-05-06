#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#define BUFSIZE 1000
#define gMax 20

int NUMPROD = 1;    // number of producers
int NUMCONS = 1;    // number of consumers

int gBuf[BUFSIZE];  // global buffer
atomic_int gNum = 1;       // global counter      

atomic_int gIn = 0;        // input cursor in gBuf
atomic_int gOut = 0;       // output cursor in gBuf

pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t reader;
sem_t writer;

/*
void insert(int x)
{
    gBuf[gIn] = x;
    ++gIn;
}

int extract()
{    
    int x = gBuf[gOut];
    ++gOut;
    return x;
}


int incgNum() {
    int num = ++gNum;
    return num;
}

int getgNum() {
    int num = gNum;
    return num;
}
*/

// It outputs the message to console
void say(int me, char* msg, int x)
{
    pthread_mutex_lock(&writeMutex);
    printf("Thread number %d ", me);
    printf(msg);
    printf("%d\n", x);
    pthread_mutex_unlock(&writeMutex);
}

void* producer(void* arg) {
    int me = pthread_self();
    int num;
    //for(int i=1; i<=gMax; i++)
    while(gIn < gMax)
    {        
        sem_wait(&writer);
        if(gIn - gOut > 2)
        {
            sleep(1);            
        }
        if(gIn - gOut < 1)
        {
            sem_wait(&reader);
        }
        pthread_mutex_lock(&writeMutex);
            gBuf[gIn] = gNum;
            num = gNum;
        pthread_mutex_unlock(&writeMutex);
        
        say(me, "produced: ", num);

        pthread_mutex_lock(&writeMutex);
            ++gIn;
            ++gNum;
        pthread_mutex_unlock(&writeMutex);
        if(gIn - gOut > 0)
        {
            sem_post(&reader);
        }
        sem_post(&writer);
        
    }
    return NULL;
}

void* consumer(void* arg) {
    int me = pthread_self();
    int num;    
    for(int i=0; i<gMax; i++)
    {
        sem_wait(&reader);     
        pthread_mutex_lock(&readMutex);
        if(gIn - gOut > 1)
        {
            sem_wait(&writer);
        }
        if(gIn - gOut < 1)
        {
            sleep(1);
        }
        pthread_mutex_unlock(&readMutex);
        
        num = gBuf[gOut];

        pthread_mutex_lock(&readMutex);
        ++gOut;
        if(gIn - gOut == 1)
        {
            sem_post(&writer);
        }
        pthread_mutex_unlock(&readMutex);

        sem_post(&reader);
        say(me, "consumed: ", num);
    }
    return NULL;
}

// It simply checks the input on console, so please ignore it.
void checkInput(int argc, char* argv[]) {
    if (argc == 1) {
        NUMPROD = 1;
        NUMCONS = 1;
        return;
    }

    if (argc != 3) {
        printf("Specify <producers> <consumers> \n");
        printf("Eg: 2 3 \n");
        exit(0);
    }

    NUMPROD = atoi(argv[1]);
    if (NUMPROD < 1 || NUMPROD > 10) {
        printf("Number of producers must lie in the range 1..10 \n");
        exit(0);
    }

    NUMCONS = atoi(argv[2]);
    if (NUMCONS < 1 || NUMCONS > 10) {
        printf("Number of consumers must lie in the range 1..10 \n");
        exit(0);
    }
}


int main(int argc, char* argv[]) 
{
    // Checks whether the input is valid or not
    // It also sets the specified number of producers and consumers
    checkInput(argc, argv);

    // Creates producers and consumers threads.
    pthread_t prod[NUMPROD];
    pthread_t cons[NUMCONS];

    // Initialize mutexes and semaphores
    pthread_mutex_init(&readMutex, NULL);
    pthread_mutex_init(&writeMutex, NULL);
    sem_init(&writer, 0, 3);
    sem_init(&reader, 0, 3);

    for(int i = 0; i < NUMPROD; ++i) 
    {
        pthread_create(&prod[i], 0, producer, 0);  
    }

    for(int i = 5; i < NUMCONS+5; ++i)
    {
        pthread_create(&cons[i], 0, consumer, 0);
    }

    for(int i = 0; i < NUMPROD; ++i)
    {
        pthread_join(prod[i], NULL);
    }

    for(int i = 0; i < NUMCONS; ++i)
    {
        pthread_join(cons[i], NULL); 
    }

    /*
    pthread_mutexattr_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);
    */

    printf("All done! Hit any key to finish \n");  
    getchar();  
    return 0;
}