
// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

// Código começa aqui

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define TRUE 1
#define FALSE 0
#define CONSEGUIU 0


/*
Problema: Filósofos
N filósofos estão à mesa
Máximo de filósofos comendo ao mesmo tempo (Compartilhamento de garfo e faca)
s/ starvation (alguém ficar sem comer) = uso de semaforo
s/ deadlock (todos ficarem esperando) = uso de semaforo
para quando pelo menos 1 dos filósofos atingirem a quantidade estipulada de vezes que devem comer passada por parâmetro

prints:
	Filosofo X esta comendo ao mesmo tempo que os filosofos Y,Z,W.
		- Começar com Filosofo X esta comendo.
		- Pensar em como implementar a outra parte

entrada:
	./filosofos.exe <N_filosofos> <M_vezes_comeu>]

saída:
	Filosofo 1 comeu 50 vezes.
	Filosofo 2 comeu 70 vezes.
	etc.
	USO DE JOIN AQUI

ANOTAÇÕES:
	Garfos são compartilhados
		- Talvez exista um tipo grafo que tenha seu mutex
	Para fins de lógica, opotou-se usar hashis, já que fazem mais sentido
*/

typedef int boolean;

typedef struct {
	sem_t sem_pode_comer;
} hashi_t;

typedef struct {
	int id;
} filosofo_t;

typedef struct {
	boolean alguem_comeu;
	//hread_cond_t condicao;
	pthread_mutex_t mutex;
	sem_t sem;
} comeu_o_estipulado_t;

typedef struct {
	filosofo_t* filosofo;
	
	hashi_t* hashi_1;
	hashi_t* hashi_2;

	filosofo_t* filosofos_comendo;
	int* qtd_max_filosofos_comendo; // definida pela formula: qtd_max_filosofos_comendo = N - floor(N/2)

	int* qtd_que_precisa_comer;
	comeu_o_estipulado_t* alguem_comeu_estipulado;
} pkg_filosofo_t;

pkg_filosofo_t* create_pkg_filosofo_t(filosofo_t* f, hashi_t* h1, hashi_t* h2, filosofo_t* lst_f, int* qtd_max_filosofos_comendo, int* qtd_que_precisa_comer, comeu_o_estipulado_t* comeram_o_estipulado) {
	pkg_filosofo_t* pkg = (pkg_filosofo_t*)malloc(sizeof(pkg_filosofo_t));
	pkg->filosofo = f;
	pkg->hashi_1 = h1;
	pkg->hashi_2 = h2;
	pkg->filosofos_comendo = lst_f;
	pkg->qtd_max_filosofos_comendo = qtd_max_filosofos_comendo;
	pkg->qtd_que_precisa_comer = qtd_que_precisa_comer;
	pkg->alguem_comeu_estipulado = comeram_o_estipulado;
	return pkg;
}

void* filosofo(void* arg)
{
	int vezes_que_comeu = 0;
	pkg_filosofo_t* pf = (pkg_filosofo_t*)arg;
	while (vezes_que_comeu < pf->qtd_que_precisa_comer) {
		pthread_mutex_lock(pf->alguem_comeu_estipulado->mutex);
		if (pf->alguem_comeu_estipulado->alguem_comeu == TRUE)
		{
			pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
			return (void*)vezes_que_comeu;
		}
		else
		{
			pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
			// esperar no semaforo do hashi_1 pra comer
			sem_wait(pf->hashi_1->sem_pode_comer);
			// esperar no semaforo do hashi_2 pra comer
			sem_wait(pf->hashi_2->sem_pode_comer);
			vezes_que_comeu++;
		}
	}

	// informa de algum jeito aos outros que ele chegou no limite
	if (sem_try_wait(pf->alguem_comeu_estipulado->sem) == CONSEGUIU)
	{
		pthread_mutex_lock(pf->alguem_comeu_estipulado->mutex);
		pf->alguem_comeu_estipulado->alguem_comeu = TRUE;
		pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
	}

	return (void*) vezes_que_comeu;
}

int main()
{
	int n_filosofos = 5, vezes_a_comer = 10000;

	sem_t sem_alguem_comeu;
	sem_init(&sem_alguem_comeu, 0, 1);

	/*pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);

	pthread_cond_signal(cond);*/

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	comeu_o_estipulado_t alguem;
	alguem.alguem_comeu = FALSE;
	//alguem.condicao = cond;
	alguem.mutex = mutex;
	alguem.sem = sem_alguem_comeu;
	
	// lista de thread de filosofos

	// lista de hashis

	return 0;
}