// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/*
	seu código deve ter NO MÍNIMO, uma thread geradora de pacotes de 'N' números a serem detectados
		Geradores de números
	'M' threads de detecção e comunicação entre elas para transportar esses pacotes de números enviados pela thread geradora
		Threads de processamento
	e uma thread de recepção de resultados que deve imprimir OS RESULTADOS EM ORDEM
		Threads de processamento deve imprimir os resultados em ordem
	Uma Thread de impressão
	As Threads de processamento devem ter um buffer de tamnho a ser informado pelo valor da K

	sieve = peneira

	N: quantidade de primos que a devem ter na thread RESULTADO
	M: quantidade de threads de processamento
	K: tamanho dos buffers de comunicação
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
#define primos_t int*
#define bignumber_t long long

typedef struct {
	int* index;
	int* buffer;
	int max_size_buffer;
	pthread_mutex_t* mutex;
}buffer_t, *buffer_pt;

buffer_pt create_buffer(int* index, int* buffer, int max_size_buffer, pthread_mutex_t* mutex)
{
	buffer_pt b = malloc(sizeof(buffer_t));
	b->index = index;
	b->buffer = buffer;
	b->max_size_buffer = max_size_buffer;
	b->mutex = mutex;
	return buffer;
}

typedef struct {
	id_t id;
	buffer_pt buffer_out;
}pkg_thread_geradora_t, *pkg_thread_geradora_pt;

typedef struct {
	id_t id;
	buffer_pt buffer_in; // tamanho K
	buffer_pt buffer_out; // tamanho K
	primos_t primos; // tamanho X
}pkg_thread_sieve_processamento_t, *pkg_thread_sieve_processamento_pt;

typedef struct {
	id_t id;
	int *qtd_primos; // tamanho X/ln(x)
}pkg_thread_resultado_t, * pkg_thread_resultado_pt;

#define TRUE 1
#define FALSE 0

pkg_thread_geradora_pt create_pkg_thread_geradora(id_t id)
{
	pkg_thread_geradora_pt tg = malloc(sizeof(pkg_thread_geradora_t));
	tg->id = id;
	return tg;
}

void* thread_geradora(void* args)
{
	pkg_thread_geradora_pt pkg = (pkg_thread_geradora_pt)args;
	buffer_pt buffer = pkg->buffer_out;
	bignumber_t number = 0;
	while (TRUE)
	{
		// envia o número para a primeira thread de processamento
		pthread_mutex_lock(buffer->mutex);
		int* i = buffer->index;
		buffer->buffer[*i] = number++;
		(*i)++;
		pthread_mutex_unlock(buffer->mutex);
	}
	return NULL;
}

pkg_thread_sieve_processamento_pt create_pkg_thread_sieve_processamento(id_t id, buffer_pt buffer_in, buffer_pt buffer_out, primos_t primos)
{
	pkg_thread_sieve_processamento_pt tsp = malloc(sizeof(pkg_thread_sieve_processamento_t));
	tsp->id = id;
	tsp->buffer_in = buffer_in;
	tsp->buffer_out = buffer_out;
	tsp->primos = primos;
	return tsp;
}

void* thread_sieve_processamento(void* args)
{
	return NULL;
}

pkg_thread_resultado_pt create_pkg_thread_resultado(id_t id)
{
	pkg_thread_resultado_pt ts = malloc(sizeof(pkg_thread_resultado_t));
	ts->id = id;
	ts->qtd_primos;
	return (pkg_thread_resultado_pt)NULL;
}

void* thread_resultado(void* args)
{
	return NULL;
}

int main(int argc, char *argv[])
{

	pthread_t t_geradora, t_resultado;

	pthread_create(&t_geradora, NULL, &thread_geradora, NULL);
	pthread_create(&t_resultado, NULL, &thread_geradora, NULL);

	/*
		threads de processamento
	*/

	pthread_cancel(t_geradora);
	pthread_cancel(t_resultado);

	return 0;
}