#pragma once
#define _CRT_SECURE_NO_WARNINGS 1 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 

// --- Exemplo 3: Multiplas Threads com parâmetro
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 5 

void* PrintHello(void* threadid)
{
	int tid;
	tid = (int)threadid;
	printf("Hello World! It's me, thread #%d!\n", tid);
	pthread_exit(NULL);

	return NULL;
}

int exemplo_3(int argc, char* argv[])
{
	pthread_t threads[NUM_THREADS];
	int rc, t;

	for (t = 0; t < NUM_THREADS; t++)
	{
		printf("In main: creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, PrintHello, (void*)t);
		if (rc)
		{
			printf("ERROR code is %d\n", rc);
			exit(-1);
		}
	}

	system("pause");

	pthread_exit(NULL);
	return 0;
}
// --- Fim Exemplo 3 ---