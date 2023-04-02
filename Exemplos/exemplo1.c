#pragma once
#define _CRT_SECURE_NO_WARNINGS 1 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 

// Exemplo 1: Hello World! com PThreads
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void* thread(void* vargp)
{
	printf("Hello World da thread criada pela thread principal!\n");
	pthread_exit((void*)NULL);
	return NULL;
}

int exemplo_1()
{
	pthread_t tid;  // Estrutura que define uma thread
	printf("Hello World da thread principal!\n");

	// Cria uma thread com os atributos definidos em tid, opções padrão NULL,
	// thread é função que contém o código da thread e não há parametros de entrada (ou seja, NULL)
	pthread_create(&tid, NULL, thread, NULL);

	// Espera a thread "tid" terminar e não captura seu valor de retorno retorno (NULL)
	pthread_join(tid, NULL);

	system("pause");

	// Retorno NULL da thread principal. Desnecessário.
	pthread_exit((void*)NULL);

	return 0;
}

// --- Fim Exemplo 1 ---