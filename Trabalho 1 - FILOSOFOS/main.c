
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
#include <math.h>

#define TRUE 1
#define FALSE 0
#define CONSEGUIU 0

#define LIVRE 0
#define OCUPADO 1

#define NAO_ESTA_NA_LISTA -1
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

typedef int bool;

int floor_int(double num)
{
	return ((int)floor((double)num / 2));
}

typedef struct {
	int id;
	sem_t* sem_pode_comer;
} hashi_t;

hashi_t* create_hashi(int id, sem_t* sem)
{
	hashi_t* h = (hashi_t*)malloc(sizeof(hashi_t));
	if (h != NULL)
	{
		h->id = id;
		h->sem_pode_comer = sem;
	}
	return h;
}

typedef struct {
	int id;
} filosofo_t;

filosofo_t* create_filoso(int id)
{
	filosofo_t* f = (filosofo_t*)malloc(sizeof(filosofo_t));
	if (f!=NULL)
	{
		f->id = id;
	}
	return f;
}

typedef struct filosofos_comendo_t {
	filosofo_t** filosofos;
	pthread_mutex_t* mutex;
	int* qtd_max_filosofos_comendo; // definida pela formula: qtd_max_filosofos_comendo = N - floor(N/2)
	int* posicoes_livres; // {0, 1, 0, 0, 1} => 0 = LIVRE e 1 = OCUPADO
	int qtd_filosofos_inseridos;
	int (*adicionar_filosofo) (filosofo_t*, struct filosofos_comendo_t*);
	void (*remover_filosofo) (int, struct filosofos_comendo_t*);
} filosofos_comendo_t;

int adicionar_filosofo_comendo(filosofo_t* filosofo, filosofos_comendo_t* filosofos_comendo)
{
	// acha posição livre, para inserir o filosofo
	int index_filosofo;
	for (index_filosofo = 0; index_filosofo < *(filosofos_comendo->qtd_max_filosofos_comendo); index_filosofo++)
	{
		if (filosofos_comendo->posicoes_livres[index_filosofo] == LIVRE)
		{
			break;
		}
	}
	pthread_mutex_lock(filosofos_comendo->mutex);
	filosofos_comendo->filosofos[index_filosofo] = filosofo;
	filosofos_comendo->posicoes_livres[index_filosofo] = OCUPADO;
	(filosofos_comendo->qtd_filosofos_inseridos)++;
	pthread_mutex_unlock(filosofos_comendo->mutex);
	return index_filosofo;
}

void remover_filosofo_comendo(int index_filosofo, filosofos_comendo_t* filosofos_comendo)
{
	pthread_mutex_lock(filosofos_comendo->mutex);
	filosofos_comendo->filosofos[index_filosofo] = NULL;
	filosofos_comendo->posicoes_livres[index_filosofo] = LIVRE;
	(filosofos_comendo->qtd_filosofos_inseridos)--;
	pthread_mutex_unlock(filosofos_comendo->mutex);
}

int* create_lst_posicoes_livres(int num_posicoes_livres)
{
	int* npl = (int*)malloc(sizeof(int) * num_posicoes_livres);
	if (npl != NULL)
	{
		for (int i = 0; i < num_posicoes_livres; i++)
		{
			npl[i] = LIVRE;
		}
	}
	return npl;
}

filosofos_comendo_t* create_filosofos_comendo(int* qtd_max_filososfos_comendo, pthread_mutex_t* mutex_filosofos_comendo)
{
	filosofos_comendo_t* filosofos_comendo = (filosofos_comendo_t*)malloc(sizeof(filosofos_comendo_t));
	if (filosofos_comendo != NULL)
	{
		filosofos_comendo->filosofos = (filosofo_t**)malloc(sizeof(filosofo_t*) * (*qtd_max_filososfos_comendo));
		filosofos_comendo->mutex = mutex_filosofos_comendo;
		filosofos_comendo->qtd_max_filosofos_comendo = qtd_max_filososfos_comendo;
		filosofos_comendo->posicoes_livres = create_lst_posicoes_livres(*(qtd_max_filososfos_comendo));
		filosofos_comendo->qtd_filosofos_inseridos = 0;
		filosofos_comendo->adicionar_filosofo = adicionar_filosofo_comendo;
		filosofos_comendo->remover_filosofo = remover_filosofo_comendo;
	}
	return filosofos_comendo;
}

typedef struct {
	bool alguem_comeu;
	pthread_mutex_t* mutex;
	sem_t* sem;
} comeu_o_estipulado_t;

comeu_o_estipulado_t* create_comeu_o_estipulado(pthread_mutex_t* mutex_estipulado, sem_t* sem_alguem_comeu_o_estipulado)
{
	comeu_o_estipulado_t* estipulado = (comeu_o_estipulado_t*)malloc(sizeof(comeu_o_estipulado_t));
	if (estipulado != NULL)
	{
		estipulado->alguem_comeu = FALSE;
		estipulado->mutex = mutex_estipulado;
		estipulado->sem = sem_alguem_comeu_o_estipulado;
	}
	return estipulado;
}

typedef struct {
	filosofo_t* filosofo;
	
	hashi_t* hashi_1;
	hashi_t* hashi_2;

	filosofos_comendo_t* filosofos_comendo;
	int index_filosofo_comendo;

	int* qtd_que_precisa_comer;
	comeu_o_estipulado_t* alguem_comeu_estipulado;
} pkg_filosofo_t;

pkg_filosofo_t* create_pkg_filosofo(filosofo_t* f,
	hashi_t* h1, hashi_t* h2,
	filosofos_comendo_t* filosofos_comendo,
	int* qtd_que_precisa_comer, comeu_o_estipulado_t* comeram_o_estipulado
)
{
	pkg_filosofo_t* pkg = (pkg_filosofo_t*)malloc(sizeof(pkg_filosofo_t));
	pkg->filosofo = f;
	pkg->hashi_1 = h1;
	pkg->hashi_2 = h2;
	pkg->filosofos_comendo = filosofos_comendo;
	pkg->index_filosofo_comendo = NAO_ESTA_NA_LISTA;
	pkg->qtd_que_precisa_comer = qtd_que_precisa_comer;
	pkg->alguem_comeu_estipulado = comeram_o_estipulado;
	return pkg;
}

void estou_comendo_com(filosofo_t* f, filosofos_comendo_t* fc)
{
	int i, k;
	pthread_mutex_lock(fc->mutex);
	if (fc->qtd_filosofos_inseridos >= 3)
	{
		printf("Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
		// for pelas posições livres
		for (i = 0, k = 0; i < (*(fc->qtd_max_filosofos_comendo)) - 1; i++, k++)
		{
			if (fc->posicoes_livres[i] == OCUPADO) {
				if (k < fc->qtd_filosofos_inseridos)
				{
					printf("%d, ", fc->filosofos[i]->id);
				}
				else
				{
					printf("%d.\n", fc->filosofos[i]->id);
				}
			}
		}
	}
	else if(fc->qtd_filosofos_inseridos == 2)
	{
		printf("Filosofo %d esta comendo ao mesmo tempo que o filosofo ", f->id);
		for (i = 0, k = 0; i < (*(fc->qtd_max_filosofos_comendo)) - 1; i++, k++)
		{
			if (fc->posicoes_livres[i] == OCUPADO) {
				if (k < fc->qtd_filosofos_inseridos)
				{
					printf("%d.\n", fc->filosofos[i]->id);
					break;
				}
			}
		}
		// for pelas posições livres, com um break ao achar
	}
	else if (fc->qtd_filosofos_inseridos == 1)
	{
		printf("Filosofo %d esta comendo sozinho.", f->id);
	}
	pthread_mutex_lock(fc->mutex);
}

void* filosofo(void* arg)
{
	int vezes_que_comeu = 0;
	pkg_filosofo_t* pf = (pkg_filosofo_t*)arg;
	while (vezes_que_comeu < *(pf->qtd_que_precisa_comer)) {
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
			pf->index_filosofo_comendo = pf->filosofos_comendo->adicionar_filosofo(pf->filosofo, pf->filosofos_comendo);
			vezes_que_comeu++;
			// travar mutex do filosofos_comendo
			//  IDEIA DE UM IMPRESSOR, QUE RECEBE UMA NOTIFICAÇÂO DE QUE O FILOSOFO ESTÁ COMENDO E IMPRIME TODOS QUE ESTÃO COMENDO COM ELE
			estou_comendo_com(pf->filosofo, pf->filosofos_comendo);
			pf->filosofos_comendo->remover_filosofo(pf->index_filosofo_comendo, pf->filosofos_comendo);
			pf->index_filosofo_comendo = NAO_ESTA_NA_LISTA;

			// informa pelo semaforo do hashi_1 que está livre pra comer
			sem_post(pf->hashi_1->sem_pode_comer);
			// informa pelo semaforo do hashi_2 que está livre pra comer
			sem_post(pf->hashi_2->sem_pode_comer);
		}
	}

	// informa de algum jeito aos outros que ele chegou no limite
	if (sem_trywait(pf->alguem_comeu_estipulado->sem) == CONSEGUIU)
	{
		pthread_mutex_lock(pf->alguem_comeu_estipulado->mutex);
		pf->alguem_comeu_estipulado->alguem_comeu = TRUE;
		pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
	}

	return (void*) vezes_que_comeu;
}

int main()
{
	int n_filosofos = 5, vezes_a_comer = 10;
	
	int qtd_max_filososfos_comendo = n_filosofos - floor_int(n_filosofos / 2.0);
	int index_filosofos_comendo = 0;
	
	sem_t sem_alguem_comeu_o_estipulado;
	sem_init(&sem_alguem_comeu_o_estipulado, 0, 1); // Começa liberado para um acesso porque o primeiro que acessar já mudará a flag que todos irão ler

	pthread_mutex_t mutex_estipulado;
	pthread_mutex_init(&mutex_estipulado, NULL);

	comeu_o_estipulado_t* estipulado = create_comeu_o_estipulado(&mutex_estipulado, &sem_alguem_comeu_o_estipulado);
	
	// lista de filosofos que estão comendo

	pthread_mutex_t mutex_filosofos_comendo;
	pthread_mutex_init(&mutex_filosofos_comendo, NULL);

	filosofos_comendo_t* filosofos_comendo = create_filosofos_comendo(&qtd_max_filososfos_comendo, &mutex_filosofos_comendo);

	// lista de thread de filosofos
	filosofo_t** filosofos = (filosofo_t**)malloc(sizeof(filosofo_t*) * n_filosofos);
	int i;
	if (filosofos != NULL)
	{
		for (i = 0; i < n_filosofos; i++)
		{
			filosofos[i] = create_filoso(i);
		}
	}

	// lista de semaforos para os hashis
	sem_t* semaphores_hashis = (sem_t*)malloc(sizeof(sem_t) * n_filosofos);
	if (semaphores_hashis != NULL)
	{
		for (i = 0; i < n_filosofos; i++)
		{
			sem_init(&semaphores_hashis[i], 0, 1);
		}
	}

	// lista de hashis
	hashi_t** hashis = (hashi_t**)malloc(sizeof(hashi_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++)
	{
		hashis[i] = create_hashi(i, &semaphores_hashis[i]);
	}

	// lista de pkgs_filosofos
	pkg_filosofo_t** pkgs = (pkg_filosofo_t**)malloc(sizeof(pkg_filosofo_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++)
	{
		pkgs[i] = create_pkg_filosofo(filosofos[i], hashis[i], hashis[i + 1], filosofos_comendo, &vezes_a_comer, estipulado);
	}

	// criação das threads
	pthread_t* threads_filosofos = (pthread_t*)malloc(sizeof(pthread_t*) * n_filosofos);
	if (threads_filosofos != NULL)
	{
		for (i = 0; i < n_filosofos; i++) {
			pthread_create(&threads_filosofos[i], NULL, &filosofo, (void*) pkgs[i]);
		}
		for (i = 0; i < n_filosofos; i++) {
			pthread_join(threads_filosofos[i], NULL);
		}
	}


	return 0;
}