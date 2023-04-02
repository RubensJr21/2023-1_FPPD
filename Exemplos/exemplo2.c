#pragma once
#define _CRT_SECURE_NO_WARNINGS 1 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 

//// Exemplo 2: Escopo de variáveis
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int global;
void* thr_func(void* arg);

int exemplo_2(void)
{
	pthread_t tid;
	global = 20;
	printf("Thread principal: %d\n", global);
	pthread_create(&tid, NULL, thr_func, NULL);
	pthread_join(tid, NULL);
	printf("Thread principal: %d\n", global);
	system("pause");
	return 0;
}

void* thr_func(void* arg)
{
	global = 40;
	printf("Novo thread: %d\n", global);
	return NULL;
}
//// --- Fim Exemplo 2 ---