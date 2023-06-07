// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <Windows.h>
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
#define bufer_primos_pt long long*
#define bignumber_t long long
#define round_t int

typedef struct {
	bignumber_t number;
	round_t round;
	id_t id_Thread_que_resolveu;
} pkg_number_to_veriry_t, *pkg_number_to_veriry_pt, numeros_primos_t, *numeros_primos_pt;

pkg_number_to_veriry_pt create_pkg_number_to_veriry(bignumber_t number)
{
	pkg_number_to_veriry_pt pkg = malloc(sizeof(pkg_number_to_veriry_t));
	pkg->number = number;
	pkg->round = 0;
	pkg->id_Thread_que_resolveu = -1;
	return pkg;
}

typedef struct {
	int* current_index;
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer;
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
}buffer_t, *buffer_pt;

buffer_pt create_buffer(int max_size_buffer)
{
	int* index = malloc(sizeof(int));
	*index = 0;

	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_t* cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond, NULL);

	buffer_pt b = malloc(sizeof(buffer_t));
	b->current_index = index;
	b->buffer = malloc(sizeof(pkg_number_to_veriry_t) * max_size_buffer);
	b->max_size_buffer = max_size_buffer;
	b->mutex = mutex;
	b->cond = cond;
	return b;
}

typedef struct {
	id_t id;
	buffer_pt buffer_out;
}pkg_thread_geradora_t, *pkg_thread_geradora_pt;

typedef struct {
	id_t id;
	buffer_pt buffer_in; // tamanho K
	buffer_pt buffer_out; // tamanho K
	bufer_primos_pt primos; // tamanho X
}pkg_thread_sieve_processamento_t, *pkg_thread_sieve_processamento_pt;

typedef struct {
	id_t id;
	buffer_pt buffer_resultados; // tamanho X/ln(x)
}pkg_thread_resultado_t, * pkg_thread_resultado_pt;

#define TRUE 1
#define FALSE 0

pkg_thread_geradora_pt create_pkg_thread_geradora(id_t id, buffer_pt buffer)
{
	pkg_thread_geradora_pt tg = malloc(sizeof(pkg_thread_geradora_t));
	tg->id = id;
	tg->buffer_out = buffer;
	return tg;
}

void insertInBuffer(buffer_pt buffer, pkg_number_to_veriry_pt number_to_verify)
{
	
}

void* thread_geradora(void* args)
{
	pkg_thread_geradora_pt pkg = (pkg_thread_geradora_pt)args;
	buffer_pt buffer = pkg->buffer_out;
	bignumber_t number = 1;
	pkg_number_to_veriry_pt ntv;
	while (TRUE)
	{
		ntv = create_pkg_number_to_veriry(number);
		pthread_mutex_lock(buffer->mutex);
		printf("numero gerado: %lld\n", ntv->number);
		// envia o número para a primeira thread de processamento
		if (*(buffer->current_index) < buffer->max_size_buffer)
		{
			buffer->buffer[(*(buffer->current_index))++] = ntv;
			pthread_mutex_unlock(buffer->mutex);
		}
		else
		{
			// system("pause");
			// esperar liberar
			// usar cond
			pthread_cond_wait(buffer->cond, buffer->mutex);
			// buffer circular
			buffer->buffer[(*(buffer->current_index))++] = ntv;
			pthread_mutex_unlock(buffer->mutex);
		}
		number++;
	}
	return NULL;
}

pkg_thread_sieve_processamento_pt create_pkg_thread_sieve_processamento(id_t id, buffer_pt buffer_in, buffer_pt buffer_out, int buffer_size_internal)
{
	pkg_thread_sieve_processamento_pt tsp = malloc(sizeof(pkg_thread_sieve_processamento_t));
	tsp->id = id;
	tsp->buffer_in = buffer_in;
	tsp->buffer_out = buffer_out;
	tsp->primos = malloc(sizeof(bufer_primos_pt) * buffer_size_internal);
	return tsp;
}

void* thread_sieve_processamento(void* args)
{
	pkg_thread_sieve_processamento_pt pkg = (pkg_thread_sieve_processamento_pt)args;
	buffer_pt buffer_in = pkg->buffer_in;
	buffer_pt buffer_out = pkg->buffer_out;
	bufer_primos_pt primos = pkg->primos;
	return NULL;
}

pkg_thread_resultado_pt create_pkg_thread_resultado(id_t id, buffer_pt buffer_resultados)
{
	pkg_thread_resultado_pt ts = malloc(sizeof(pkg_thread_resultado_t));
	ts->id = id;
	ts->buffer_resultados = buffer_resultados;
	return (pkg_thread_resultado_pt)NULL;
}

void* thread_resultado(void* args)
{
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t t_geradora, t_resultado;

	int max_size_comunication_buffer = 10;
	int max_size_internal_buffer = 5;
	int numero_a_serem_testados = 131313;


	buffer_pt buffer_out_geradora = create_buffer(max_size_comunication_buffer);

	buffer_pt buffer_out_tsp = create_buffer(max_size_comunication_buffer);

	buffer_pt buffer_resultados_numeros_primos = create_buffer(numero_a_serem_testados);

	pkg_thread_geradora_pt pkg_tg = create_pkg_thread_geradora(1, buffer_out_geradora);
	pkg_thread_sieve_processamento_pt pkg_tsp = create_pkg_thread_sieve_processamento(2, buffer_out_geradora, buffer_out_tsp, max_size_internal_buffer);
	pkg_thread_resultado_pt pkg_tr = create_pkg_thread_resultado(3, buffer_resultados_numeros_primos);

	pthread_create(&t_geradora, NULL, &thread_geradora, (void*) pkg_tg);
	pthread_create(&t_resultado, NULL, &thread_resultado, (void*) pkg_tr);

	pthread_join(t_geradora, NULL);
	pthread_join(t_resultado, NULL);

	/*
		threads de processamento
	*/

	/*
	pthread_cancel(t_geradora);
	pthread_cancel(t_resultado);
	*/

	return 0;
}