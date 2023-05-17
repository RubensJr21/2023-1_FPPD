// C�digo necess�rio para o Visual Studio n�o acusar fun��es inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/*
	seu c�digo deve ter NO M�NIMO, uma thread geradora de pacotes de 'N' n�meros a serem detectados
		Geradores de n�meros
	'M' threads de detec��o e comunica��o entre elas para transportar esses pacotes de n�meros enviados pela thread geradora
		Threads de processamento
	e uma thread de recep��o de resultados que deve imprimir OS RESULTADOS EM ORDEM
		Threads de processamento deve imprimir os resultados em ordem
	Uma Thread de impress�o
	As Threads de processamento devem ter um buffer de tamnho a ser informado pelo valor da K

	sieve = peneira

	N: quantidade de primos que a devem ter na thread RESULTADO
	M: quantidade de threads de processamento
	K: tamanho dos buffers de comunica��o
	X: tamanho do buffer de primos das threads de processamento
		buffer interno (que guarda os numeros primos)

	typedef struct {
		id_t id;
		int buffer_in[K];
		int buffer_out[K];
		int primos[X];
	}sieve_processamento;

	./primes.exe <N> <M> <K> <X>
	
	
*/

#define id_t int

typedef struct {
	id_t id;
}thread_geradora_t, *thread_geradora_pt;

typedef struct {
	id_t id;
	int *buffer_in; // tamanho K
	int *buffer_out; // tamanho K
	int *primos; // tamanho X
}thread_sieve_processamento_t, *thread_sieve_processamento_pt;

typedef struct {
	id_t id;
	int *qtd_primos; // tamanho X/ln(x)
}thread_resultado_t, * thread_resultado_pt;

#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[])
{
	return 0;
}