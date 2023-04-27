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

typedef struct {
	pthread_mutex_t* mutex;
} saleiro_t;

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
	filosofo_t* filosofo;
	
	hashi_t* hashi_1;
	hashi_t* hashi_2;
	pthread_mutex_t* mutex_saleiro;

	int* filosofos_comendo;
	int numero_de_filosofos;
	void (*filosofo_estah_comendo) (filosofo_t*, int*);
	void (*filosofo_terminou_de_comer) (filosofo_t*, int*);
	int* qtd_que_precisa_comer;

	int vezes_que_comeu;

	sem_t* sem_pode_encerrar;
} pkg_filosofo_t;

pkg_filosofo_t* create_pkg_filosofo(filosofo_t* f,
	hashi_t* h1, hashi_t* h2, pthread_mutex_t* mutex_saleiro,
	int* filosofos_comendo, int* qtd_que_precisa_comer, int numero_de_filosofos,
	sem_t* pode_encerrar
)
{
	pkg_filosofo_t* pkg = malloc(sizeof(pkg_filosofo_t));
	pkg->filosofo = f;
	pkg->hashi_1 = h1;
	pkg->hashi_2 = h2;
	pkg->mutex_saleiro = mutex_saleiro;
	pkg->filosofos_comendo = filosofos_comendo;
	pkg->numero_de_filosofos = numero_de_filosofos;
	pkg->qtd_que_precisa_comer = qtd_que_precisa_comer;
	pkg->vezes_que_comeu = 0;
	pkg->sem_pode_encerrar = pode_encerrar;
	return pkg;
}

#define BUFFER_SIZE 1000

void estou_comendo_com(filosofo_t* f, pkg_filosofo_t* pf)
{
	//printf("estou_comendo_com => Filosofo %d comecou a observar\n", f->id);
	int cont = 0;
	char aux[BUFFER_SIZE];
	int tamanho_frase = 0;
	tamanho_frase = sprintf_s(aux, BUFFER_SIZE, "Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
	//printf("Tamanho da frase: 'Filosofo %d esta comendo ao mesmo tempo que os filosofos ' é %d\n", f->id, tamanho_frase);
	int posicoes_a_mais = pf->numero_de_filosofos * 2;
	int tamanho_total = tamanho_frase + posicoes_a_mais + 2;
	char* saida = malloc(sizeof(char) * tamanho_total); // soma mais 2 para não dar erro n substituição da virgula por ponto
	int i, escreveu_ate = 0;
	escreveu_ate = sprintf_s(saida, tamanho_total, "Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
	for (i = 0; i < pf->numero_de_filosofos - 1; i++) {
		if (i == f->id)
		{
			continue;
		}
		if (pf->filosofos_comendo[i] == ESTA_COMENDO)
		{
			cont++;
			escreveu_ate += sprintf_s(saida + escreveu_ate, tamanho_total - escreveu_ate, "%d,", i);
		}
	}
	//printf("(#%d) Valor do cont = %d\n", f->id, cont);
	if (cont > 0)
	{
		int pos_1 = escreveu_ate - 1, pos_2 = escreveu_ate, pos_3 = escreveu_ate + 1;
		//printf("tamanho_frase = %d, escreveu_ate = %d, contante 1\n", tamanho_frase, escreveu_ate);
		//printf("'%c' '%c'\n", saida[pos_1], saida[pos_2]);
		//printf("'%d' '%d'\n", pos_1, pos_2);
		saida[pos_1] = '.';
		saida[pos_2] = '\n';
		saida[pos_3] = '\0';
		printf(saida);
	}
	//printf("estou_comendo_com => Filosofo %d terminou de observar\n", f->id);
}

void* filosofo(void* arg)
{
	pkg_filosofo_t* pf = (pkg_filosofo_t*)arg;
	//printf("Filosofo %d foi criado\n", pf->filosofo->id);
	while (pf->vezes_que_comeu < *(pf->qtd_que_precisa_comer)) {
		//printf("while => Filosofo %d vai lockar 'pf->mutex_saleiro'\n", pf->filosofo->id);
		pthread_mutex_lock(pf->mutex_saleiro);
		//printf("while => Filosofo %d lockou 'pf->mutex_saleiro'\n", pf->filosofo->id);

		//printf("enderecos de hashi_1 = %p, hashi_2 = %p\n", pf->hashi_1, pf->hashi_2);

		// esperar no semaforo do hashi_1 pra comer
		//printf("while => Filosofo %d vai esperar 'pf->hashi_1->sem_pode_comer'\n", pf->filosofo->id);
		sem_wait(pf->hashi_1->sem_pode_comer);
		//printf("while => Filosofo %d esperou 'pf->hashi_1->sem_pode_comer'\n", pf->filosofo->id);

		// esperar no semaforo do hashi_2 pra comer
		//printf("while => Filosofo %d vai esperar 'pf->hashi_2->sem_pode_comer'\n", pf->filosofo->id);
		sem_wait(pf->hashi_2->sem_pode_comer);
		//printf("while => Filosofo %d esperou 'pf->hashi_2->sem_pode_comer'\n", pf->filosofo->id);

		//printf("while => Filosofo %d vai deslockar 'pf->mutex_saleiro'\n", pf->filosofo->id);
		pthread_mutex_unlock(pf->mutex_saleiro);
		//printf("while => Filosofo %d deslockou 'pf->mutex_saleiro'\n", pf->filosofo->id);

		//printf("Filosofo %d vai comer\n", pf->filosofo->id);
		pf->vezes_que_comeu++;

		filosofo_esta_comendo(pf->filosofo, pf->filosofos_comendo);
		
		estou_comendo_com(pf->filosofo, pf);

		filosofo_terminou_de_comer(pf->filosofo, pf->filosofos_comendo);

		//printf("Filosofo %d comeu\n", pf->filosofo->id);

		// informa pelo semaforo do hashi_1 que está livre pra comer
		sem_post(pf->hashi_1->sem_pode_comer);
		// informa pelo semaforo do hashi_2 que está livre pra comer
		sem_post(pf->hashi_2->sem_pode_comer);
	}

	sem_post(pf->sem_pode_encerrar);
	// a main irá cancelar todas as outras threads e esperará num join
}

int main(int argc, char* argv[])
{
	if (argc != 3) { // 1 do nome do programa e 2 dos parâmetros
		return 0;
	}
	int n_filosofos, vezes_a_comer;
	n_filosofos = atoi(argv[1]);
	vezes_a_comer = atoi(argv[2]);
	//int n_filosofos = 500, vezes_a_comer = 10;
		
	sem_t pode_encerrar;
	sem_init(&pode_encerrar, 0, 0); // Começa liberado para um acesso porque o primeiro que acessar já mudará a flag que todos irão ler

	// lista de filosofos que estão comendo

	// lista de thread de filosofos
	filosofo_t** filosofos = malloc(sizeof(filosofo_t*) * n_filosofos);

	int* filosofos_comendo = malloc(sizeof(int) * n_filosofos);
	int i;
	for (i = 0; i < n_filosofos; i++)
	{
		//printf("Filosofo %d será criado\n", i);
		filosofos[i] = create_filoso(i);
	}

	//printf("Criou filosofos comenfo\n");

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

	pthread_mutex_t mutex_saleiro;
	pthread_mutex_init(&mutex_saleiro, NULL);

	// lista de pkgs_filosofos
	pkg_filosofo_t** pkgs = (pkg_filosofo_t**)malloc(sizeof(pkg_filosofo_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++)
	{
		//printf("Filosofo #%d recebeu os hashis (%d, %d)\n", filosofos[i]->id, i, (n_filosofos + i + 1) % n_filosofos);
		pkgs[i] = create_pkg_filosofo(filosofos[i], hashis[i], hashis[(n_filosofos + i + 1) % n_filosofos], &mutex_saleiro, filosofos_comendo, &vezes_a_comer, n_filosofos, &pode_encerrar);
	}

	// criação das threads
	pthread_t* threads_filosofos = (pthread_t*)malloc(sizeof(pthread_t) * n_filosofos);
	for (i = 0; i < n_filosofos; i++) {
		pthread_create(&threads_filosofos[i], NULL, &filosofo, (void*)pkgs[i]);
	}

	sem_wait(&pode_encerrar);

	//printf("VOU CANCELAR TODO MUNDO\n");

	for (i = 0; i < n_filosofos; i++)
	{
		// https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-pthread-cancel-cancel-thread
		pthread_cancel(threads_filosofos[i]);
	}

	sem_destroy(&pode_encerrar);

	for (i = 0; i < n_filosofos; i++)
	{
		sem_destroy(&semaphores_hashis[i]);
	}

	pthread_mutex_destroy(&mutex_saleiro);

	pkg_filosofo_t* pkg;

	for (i = 0; i < n_filosofos; i++)
	{
		pkg = pkgs[i];
		printf("Filosofo %d comeu %d vezes.\n", pkg->filosofo->id, pkg->vezes_que_comeu);
	}
	/*for (i = 0; i < n_filosofos; i++) {
		pthread_join(threads_filosofos[i], NULL);
	}*/



	return 0;
}