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

#define NAO_ESTA_COMENDO 0
#define ESTA_COMENDO 1

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
	filosofo_t* f = malloc(sizeof(filosofo_t));
	f->id = id;
	return f;
}

void filosofo_esta_comendo(filosofo_t* filosofo, int* filosofos_comendo)
{
	filosofos_comendo[filosofo->id] = ESTA_COMENDO;
}

void filosofo_terminou_de_comer(filosofo_t* filosofo, int* filosofos_comendo)
{
	filosofos_comendo[filosofo->id] = NAO_ESTA_COMENDO;
}

int* create_vetor_estado_filosofos(int qtd_de_filosofos)
{
	int* npl = malloc(sizeof(int) * qtd_de_filosofos);
	for (int i = 0; i < qtd_de_filosofos; i++)
	{
		npl[i] = NAO_ESTA_COMENDO;
	}
	return npl;
}

typedef struct {
	bool alguem_comeu;
	pthread_mutex_t* mutex;
	sem_t* sem;
} comeu_o_estipulado_t;

comeu_o_estipulado_t* create_comeu_o_estipulado(pthread_mutex_t* mutex_estipulado, sem_t* sem_alguem_comeu_o_estipulado)
{
	comeu_o_estipulado_t* estipulado = malloc(sizeof(comeu_o_estipulado_t));
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

	int* filosofos_comendo;
	int numero_de_filosofos;
	void (*filosofo_estah_comendo) (filosofo_t*, int*);
	void (*filosofo_terminou_de_comer) (filosofo_t*, int*);

	int* qtd_que_precisa_comer;
	comeu_o_estipulado_t* alguem_comeu_estipulado;
} pkg_filosofo_t;

pkg_filosofo_t* create_pkg_filosofo(filosofo_t* f,
	hashi_t* h1, hashi_t* h2,
	int* filosofos_comendo, int* qtd_que_precisa_comer, int numero_de_filosofos,
	comeu_o_estipulado_t* comeram_o_estipulado
)
{
	pkg_filosofo_t* pkg = malloc(sizeof(pkg_filosofo_t));
	pkg->filosofo = f;
	pkg->hashi_1 = h1;
	pkg->hashi_2 = h2;
	pkg->filosofos_comendo = filosofos_comendo;
	pkg->numero_de_filosofos = numero_de_filosofos;
	pkg->qtd_que_precisa_comer = qtd_que_precisa_comer;
	pkg->alguem_comeu_estipulado = comeram_o_estipulado;
	return pkg;
}

void estou_comendo_com(filosofo_t* f, pkg_filosofo_t* pf)
{
	printf("estou_comendo_com => Filosofo %d comecou a observar\n", f->id);
	int cont = 0;
	char aux[40];
	int tamanho_frase = 0;
	tamanho_frase = sprintf_s(aux, 40, "Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
	int posicoes_a_mais = pf->numero_de_filosofos * 2;
	int tamanho_total = tamanho_frase + posicoes_a_mais + 2;
	char* saida = malloc(sizeof(char) * tamanho_total); // soma mais 2 para não dar erro n substituição da virgula por ponto
	int i, escreveu_em;
	if (saida != 0)
	{
		escreveu_em = sprintf_s(saida, tamanho_total, "Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
	}
	for (i = 0; i < pf->numero_de_filosofos - 1; i++) {
		if (i == f->id)
		{
			continue;
		}
		if (pf->filosofos_comendo[i] == ESTA_COMENDO)
		{
			cont++;
			escreveu_em += sprintf_s(saida + escreveu_em, tamanho_total - escreveu_em, "%d,", i);
		}
	}
	if (cont)
	{
		saida[tamanho_frase + escreveu_em - 1] = '.';
		saida[tamanho_frase + escreveu_em] = '\n';
		printf(saida);
	}
	printf("estou_comendo_com => Filosofo %d terminou de observar\n", f->id);
}

void* filosofo(void* arg)
{
	int vezes_que_comeu = 0;
	pkg_filosofo_t* pf = (pkg_filosofo_t*)arg;
	printf("Filosofo %d foi criado\n", pf->filosofo->id);
	while (vezes_que_comeu < *(pf->qtd_que_precisa_comer)) {
		printf("Entrou no while\n");
		printf("Filosofo %d vai lockar 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
		pthread_mutex_lock(pf->alguem_comeu_estipulado->mutex);
		printf("Filosofo %d lockou 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
		if (pf->alguem_comeu_estipulado->alguem_comeu == TRUE)
		{
			printf("if => Filosofo %d vai deslockar 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
			pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
			printf("if => Filosofo %d deslockou 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
			return (void*)vezes_que_comeu;
		}
		else
		{
			printf("else => Filosofo %d vai deslockar 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
			pthread_mutex_unlock(pf->alguem_comeu_estipulado->mutex);
			printf("else => Filosofo %d deslockou 'pf->alguem_comeu_estipulado->mutex'\n", pf->filosofo->id);
			// esperar no semaforo do hashi_1 pra comer
			printf("else => Filosofo %d vai esperar 'pf->hashi_1->sem_pode_comer'\n", pf->filosofo->id);
			sem_wait(pf->hashi_1->sem_pode_comer);
			printf("else => Filosofo %d esperou 'pf->hashi_1->sem_pode_comer'\n", pf->filosofo->id);
			
			// esperar no semaforo do hashi_2 pra comer
			printf("else => Filosofo %d vai esperar 'pf->hashi_2->sem_pode_comer'\n", pf->filosofo->id);
			sem_wait(pf->hashi_2->sem_pode_comer);
			printf("else => Filosofo %d esperou 'pf->hashi_2->sem_pode_comer'\n", pf->filosofo->id);

			printf("Filosofo %d vai comer\n", pf->filosofo->id);
			vezes_que_comeu++;

			// travar mutex do filosofos_comendo
			//  IDEIA DE UM IMPRESSOR, QUE RECEBE UMA NOTIFICAÇÂO DE QUE O FILOSOFO ESTÁ COMENDO E IMPRIME TODOS QUE ESTÃO COMENDO COM ELE
			estou_comendo_com(pf->filosofo, pf);
			pf->filosofo_terminou_de_comer(pf->filosofo, pf->filosofos_comendo);

			printf("Filosofo %d comeu\n", pf->filosofo->id);

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
		
	sem_t sem_alguem_comeu_o_estipulado;
	sem_init(&sem_alguem_comeu_o_estipulado, 0, 1); // Começa liberado para um acesso porque o primeiro que acessar já mudará a flag que todos irão ler

	pthread_mutex_t mutex_estipulado;
	pthread_mutex_init(&mutex_estipulado, NULL);

	printf("Criou as variáveis\n");
	comeu_o_estipulado_t* estipulado = create_comeu_o_estipulado(&mutex_estipulado, &sem_alguem_comeu_o_estipulado);
	
	printf("Criou estipulado\n");

	// lista de filosofos que estão comendo

	// lista de thread de filosofos
	filosofo_t** filosofos = malloc(sizeof(filosofo_t*) * n_filosofos);

	int* filosofos_comendo = malloc(sizeof(int) * n_filosofos);
	int i;
	for (i = 0; i < n_filosofos; i++)
	{
		printf("Filosofo %d será criado\n", i);
		filosofos[i] = create_filoso(i);
	}

	printf("Criou filosofos comenfo\n");

	// lista de semaforos para os hashis
	sem_t* semaphores_hashis = malloc(sizeof(sem_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++)
	{
		sem_init(&semaphores_hashis[i], 0, 1);
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
		pkgs[i] = create_pkg_filosofo(filosofos[i], hashis[i], hashis[(n_filosofos + i) % n_filosofos], filosofos_comendo, &vezes_a_comer, n_filosofos, estipulado);
	}

	// criação das threads
	pthread_t* threads_filosofos = (pthread_t*)malloc(sizeof(pthread_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++) {
		pthread_create(&threads_filosofos[i], NULL, &filosofo, (void*)pkgs[i]);
	}
	for (i = 0; i < n_filosofos; i++) {
		pthread_join(threads_filosofos[i], NULL);
	}

	return 0;
}