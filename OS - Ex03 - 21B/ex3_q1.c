#include "ex3_q1.h"
#include "ex3_q1_given.h"

list_node* list_head;
list_node* list_tail;

int main()
{
	pthread_t consumers[N_CONS];
	pthread_t producers[N_PROD];
	
	initialize();

	createThreadsAndWait(consumers, producers);

	print_list();
	
	destroy();
	freeAllocations();
}

void destroy()
{
	pthread_mutex_destroy(&lockPrintAndList);
	pthread_mutex_destroy(&producerLock);
	pthread_mutex_destroy(&consumerLock);
	pthread_mutex_destroy(&randLock);
	pthread_mutex_destroy(&threadCreatedLock);
	pthread_cond_destroy(&finishThreads);
	pthread_cond_destroy(&finishItemsWait);
}

void freeAllocations()
{
	list_node* current = list_head;
	list_node* prev;
  
	while(current)
	{
		prev = current;
		current = current->next;
		free(prev->item);
		free(prev);
	}
}

void initialize()
{
	//init locks and conds
	pthread_mutex_init(&lockPrintAndList, NULL);
	pthread_mutex_init(&producerLock, NULL);
	pthread_mutex_init(&consumerLock, NULL);
	pthread_mutex_init(&randLock, NULL);
	pthread_mutex_init(&threadCreatedLock, NULL);
	pthread_cond_init(&finishThreads, NULL);
	pthread_cond_init(&finishItemsWait, NULL);
}

void createThreadsAndWait(pthread_t* consumers, pthread_t* producers)
{
	int i = 0;
	for(i = 0; i < N_PROD; i++)
	{
		pthread_create(&producers[i], NULL, produce, (void*)(long)i + 1); 
	}
	printf(ALL_PROD_CREATED);
	
	for(i = 0; i < N_CONS; i++)
	{
		pthread_create(&consumers[i], NULL, consume, (void*)(long)i + 1); 
	}
	printf(ALL_CONS_CREATED);
	setThreadCounter();

	for(i = 0; i < N_PROD; i++)
	{
		pthread_join(producers[i], NULL); 
	}
	pthread_mutex_lock(&lockPrintAndList);
	printf(PROD_TERMINATED);
	pthread_mutex_unlock(&lockPrintAndList);
	
	for(i = 0; i < N_CONS; i++)
	{
		pthread_join(consumers[i], NULL); 
	}
	printf(CONS_TERMINATED);	
}

void* produce(void* thread_number)
{
	int threadNumber = (long)(thread_number);
	waitForAllThreads();
	int firstRand, secondRand;
    
	while(itemsMadeCounter < TOTAL_ITEMS)
	{
		firstRand = randNumber();		// rand 2 numbers
		secondRand = randNumber();
		
		pthread_mutex_lock(&producerLock);
		if(itemsMadeCounter < TOTAL_ITEMS)		//check again and create if item if neeeded 
		{
			item* newItem = (item*)malloc(sizeof(item));	// allocate and set new item
			newItem->prod = firstRand * secondRand;
			newItem->status = (STATUS) NOT_DONE;
			
			pthread_mutex_lock(&lockPrintAndList);
			write_adding_item(threadNumber, newItem);  
			add_to_list(newItem);
			pthread_mutex_unlock(&lockPrintAndList);

			itemsMadeCounter++;
			if(itemsMadeCounter > doneItemsCounter)
				pthread_cond_broadcast(&finishItemsWait);
		}
		pthread_mutex_unlock(&producerLock);
	}
	
	finishProducer(threadNumber);
	pthread_exit(NULL);
}

void* consume(void* thread_number)
{
	int threadNumber = (long)(thread_number);
	waitForItemsCreated();
	
	while(doneItemsCounter < TOTAL_ITEMS)
	{
		pthread_mutex_lock(&consumerLock);
		while(doneItemsCounter == itemsMadeCounter && doneItemsCounter < TOTAL_ITEMS)
		{
			pthread_cond_wait(&finishItemsWait, &consumerLock);
		}
		
		if(doneItemsCounter < itemsMadeCounter)
		{
			item* itemReceived = get_undone_from_list();		
				
			pthread_mutex_lock(&lockPrintAndList);
			write_getting_item(threadNumber, itemReceived);
			set_two_factors(itemReceived);
			pthread_mutex_unlock(&lockPrintAndList);

			itemReceived->status = (STATUS)DONE;
			doneItemsCounter++;
		}
		
		pthread_mutex_unlock(&consumerLock);
	}
	
	finishConsumer(threadNumber);
	pthread_exit(NULL);
}

void finishProducer(int threadNumber)
{
	pthread_mutex_lock(&lockPrintAndList);
	write_producer_is_done(threadNumber);
	pthread_mutex_unlock(&lockPrintAndList);
}

void finishConsumer(int threadNumber)
{
	pthread_mutex_lock(&lockPrintAndList);
	write_consumer_is_done(threadNumber);
	pthread_mutex_unlock(&lockPrintAndList);
}

void waitForAllThreads()
{
	pthread_mutex_lock(&producerLock);
	while(threadsCounter < N_PROD + N_CONS)
	{
		pthread_cond_wait(&finishThreads, &producerLock);
	}
	pthread_mutex_unlock(&producerLock);
}

void waitForItemsCreated()
{
	pthread_mutex_lock(&consumerLock);
	while(itemsMadeCounter < ITEM_START_CNT)
	{
		pthread_cond_wait(&finishItemsWait, &consumerLock);
	}
	pthread_mutex_unlock(&consumerLock);
}

void setThreadCounter()
{
	threadsCounter = N_CONS + N_PROD;
	pthread_cond_broadcast(&finishThreads);
}

int randNumber()
{
	int randNum;
	do{
		pthread_mutex_lock(&randLock);
		randNum = get_random_in_range();
		pthread_mutex_unlock(&randLock);
	}while(!is_prime(randNum));
	return randNum;
}