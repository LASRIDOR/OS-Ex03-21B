#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

int threadsCounter = 0;
int itemsMadeCounter = 0;
int doneItemsCounter = 0;
pthread_mutex_t lockPrintAndList;
pthread_mutex_t producerLock;
pthread_mutex_t consumerLock;
pthread_mutex_t randLock;
pthread_mutex_t threadCreatedLock;
pthread_cond_t finishThreads;
pthread_cond_t finishItemsWait;

void createThreadsAndWait(pthread_t* consumers, pthread_t* producers);
void* produce(void* thread_number);
void* consume(void* thread_number);
void waitForAllThreads();
void waitForItemsCreated();
void setThreadCounter();
void finishProducer(int threadNumber);
void destroy();
void finishConsumer(int threadNumber);
void freeAllocations();
void initialize();
int randNumber();

