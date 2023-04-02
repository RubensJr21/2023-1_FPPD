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

/*
explicação do sem_trywait() => https://pubs.opengroup.org/onlinepubs/000095399/functions/sem_trywait.html
fechamento de thread => https://stackoverflow.com/questions/11624545/how-to-make-main-thread-wait-for-all-child-threads-finish
*/

#define QTD_CANIBAIS 64
#define MIL 1000
#define MILHOES 1000000
#define QTD_COMIDAS 3 * MILHOES

typedef struct
{
	int* insumos;
	int* caldeirao_comidas;
	int comidas_produzidas;
	int qtd_comidas_para_produzir;
	sem_t* pode_comer;
	sem_t* pode_cozinhar;
	sem_t* pode_cacar;
	pthread_mutex_t* mutex_insumos;
	pthread_mutex_t* mutex_caldeirao_comidas;
} pkg_cozinheiro_t;

void pkg_cozinheiro_init(
	pkg_cozinheiro_t* pkg, int* insumos, int* caldeirao_comidas, int qtd_comidas_para_produzir,
	sem_t* pode_comer, sem_t* pode_cozinhar, sem_t* pode_cacar,
	pthread_mutex_t* mutex_insumos, pthread_mutex_t* mutex_caldeirao_comidas
)
{
	pkg->insumos = insumos;
	pkg->caldeirao_comidas = caldeirao_comidas;
	pkg->comidas_produzidas = 0;
	pkg->qtd_comidas_para_produzir = qtd_comidas_para_produzir;
	pkg->pode_comer = pode_comer;
	pkg->pode_cozinhar = pode_cozinhar;
	pkg->pode_cacar = pode_cacar;
	pkg->mutex_insumos = mutex_insumos;
	pkg->mutex_caldeirao_comidas = mutex_caldeirao_comidas;
}

void* fcozinheiro(void* arg)
{
	pkg_cozinheiro_t* pkg = (pkg_cozinheiro_t*)arg;
	int ja_produzidos = 0;
	while (ja_produzidos < pkg->qtd_comidas_para_produzir)
	{
		// verificar se os insumos são suficientes para cozinhar
		pthread_mutex_lock(pkg->mutex_insumos);
		if (pkg->insumos > 0)
		{
			pthread_mutex_lock(pkg->mutex_caldeirao_comidas);
			if (pkg->caldeirao_comidas > 0)
			{
				// se tiver insumos e caldeirao vazio ele cozinha
				pkg->caldeirao_comidas++;
				ja_produzidos++;
				pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
				pkg->insumos--;
				pthread_mutex_unlock(pkg->mutex_insumos);
				pkg->comidas_produzidas++;
				sem_post(pkg->pode_comer);
			}
			else {

				pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
				pthread_mutex_unlock(pkg->mutex_insumos);
				sem_wait(pkg->pode_cozinhar);
			}
		}
		else
		{
			sem_post(pkg->pode_cacar);
			pthread_mutex_unlock(pkg->mutex_insumos);
			// se não ele espera
			sem_wait(pkg->pode_cozinhar);
		}

	}
	return NULL;
}

typedef struct
{
	int id;
	int* insumos;
	int* caldeirao_comidas;
	int comidas_consumidas;
	sem_t* pode_comer;
	sem_t* pode_cozinhar;
	sem_t* pode_cacar;
	pthread_mutex_t* mutex_insumos;
	pthread_mutex_t* mutex_caldeirao_comidas;
} pkg_canibal_t;

void pkg_canibal_init(
	pkg_canibal_t* pkg, int id, int* insumos, int* caldeirao_comidas,
	sem_t* pode_comer, sem_t* pode_cozinhar, sem_t* pode_cacar,
	pthread_mutex_t* mutex_insumos, pthread_mutex_t* mutex_caldeirao_comidas
)
{
	pkg->id = id;
	pkg->insumos = insumos;
	pkg->caldeirao_comidas = caldeirao_comidas;
	pkg->comidas_consumidas = 0;
	pkg->pode_comer = pode_comer;
	pkg->pode_cozinhar = pode_cozinhar;
	pkg->pode_cacar = pode_cacar;
	pkg->mutex_insumos = mutex_insumos;
	pkg->mutex_caldeirao_comidas = mutex_caldeirao_comidas;
}

void* fcanibal(void* arg)
{
	pkg_canibal_t* pkg = (pkg_canibal_t*)arg;
	while (1)
	{
		// tentando caçar
		int consegiu_cacar = sem_trywait(pkg->pode_cacar);
		if (consegiu_cacar) {
			pthread_mutex_lock(pkg->mutex_insumos);
			pkg->insumos++;
			pthread_mutex_unlock(pkg->mutex_insumos);
			// informando o cozinheiro que ele pode cozinhar
			sem_post(pkg->pode_cozinhar);
			sem_wait(pkg->pode_comer);
			pthread_mutex_lock(pkg->mutex_caldeirao_comidas);
			pkg->caldeirao_comidas--;
			pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
			pkg->comidas_consumidas++;
		}
	}
	return NULL;
}

/*
canibais caçam
insumos são coletados na caça
comida é gerada com base no insumo
cozinheiro só cozinha com calderão zerado com pelo menos 1 insumo
não existe um limite para insimuos => USAR int PRA GUARDAR
canibal que não comer irá caçar, e depois de caçar espera a sua vez de comer
	caçam apenas uma vez e depois ficam esperando a comida
eles sempre tem sucesso na caçada mas somente um insumo é conseguido por cada caçada
	apenas um caçador conseguirá caçar de fato
	apenas um insumo por caçada
	PARA ESSA PARTE PESQUISAR SOBRE A FUNÇÃO try PARA SEMAFORO
	CUIDADO PARA NÃO FAZER UM CÓDIGO DE busy-wait
limitar execução até 20 segundos
imprimir quanto
	cada canibal comeu
	quanto o coxinheiro cozinhou
	insumo restante no estoque
*/

int main(int argc, char** argv)
{
	int insumos = 0, caldeirao_comidas = 0;
	sem_t pode_comer, pode_cozinhar, pode_cacar;
	sem_init(&pode_comer, 0, 0);
	sem_init(&pode_cozinhar, 0, 0);
	sem_init(&pode_cacar, 0, 0);

	pthread_mutex_t mutex_insumos, mutex_caldeirao_comidas;
	pthread_mutex_init(&mutex_insumos, NULL);
	pthread_mutex_init(&mutex_caldeirao_comidas, NULL);

	pthread_t cozinheiro, * canibais = (pthread_t*)malloc(sizeof(pthread_t) * QTD_CANIBAIS);
	if (!canibais)
	{
		return NULL;
	}

	pkg_cozinheiro_t* pkg_co = (pkg_cozinheiro_t*)malloc(sizeof(pkg_cozinheiro_t));

	if (!pkg_co)
	{
		return NULL;
	}

	pkg_cozinheiro_init(pkg_co, &insumos, &caldeirao_comidas, QTD_COMIDAS, &pode_comer, &pode_cozinhar, &pode_cacar, &mutex_insumos, &mutex_caldeirao_comidas);

	pthread_create(&cozinheiro, NULL, &fcozinheiro, (void*)pkg_co);

	pkg_canibal_t* pkgs_ca = (pkg_canibal_t*)malloc(sizeof(pkg_canibal_t) * QTD_CANIBAIS);
	if (!pkgs_ca)
	{
		return NULL;
	}

	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		pkg_canibal_init(&pkgs_ca[i], i + 1, &insumos, &caldeirao_comidas, &pode_comer, &pode_cozinhar, &pode_cacar, &mutex_insumos, &mutex_caldeirao_comidas);
		pthread_create(&canibais[i], NULL, &fcanibal, (void*)&pkgs_ca[i]);
	}

	Sleep(20000);

	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		pthread_cancel(canibais[i]);
	}

	pthread_cancel(cozinheiro);

	// quanto cada canibal comeu
	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		printf("O canibal #%d consumiu %d comidas\n", pkgs_ca[i].id, pkgs_ca[i].comidas_consumidas);
	}

	printf("O cozinheiro poroduziu %d comidas\n", pkg_co->comidas_produzidas);

	printf("Restaram %d comidas no estoque\n", insumos);

	sem_close(&pode_comer);
	sem_close(&pode_cozinhar);
	sem_close(&pode_cacar);

	pthread_mutex_destroy(&mutex_insumos);
	pthread_mutex_destroy(&mutex_caldeirao_comidas);

	return 0;
}