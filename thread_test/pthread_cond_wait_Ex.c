#include <pthread.h> 
#include <stdio.h> 
#include <unistd.h> 
#include "libb.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void response(int val)
{
	if (val)
		pthread_cond_signal(&cond); //signals
	else
		printf("condition not fulfilled\n");
}

void foo(void *arg) 
{
	pthread_mutex_lock(&lock);
	printf("Waiting on condition\n");
	bar();
	pthread_cond_wait(&cond, &lock);
	pthread_mutex_unlock(&lock);
}


int main(void)
{
	pthread_t call_thread;
	int a = 10;
	pthread_create(&call_thread, NULL, foo, &a); 
	foo();
	bar();
	return 0; 
}
