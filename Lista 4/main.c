// C�digo necess�rio para o Visual Studio n�o acusar fun��es inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)

// C�digo come�a aqui

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define ANSI_COLOR_RED     	 "\x1b[31m" //cores em ANSI utilizadas 
#define ANSI_COLOR_GRAY    	 "\e[0;37m"
#define ANSI_COLOR_DARK_GRAY "\e[1;30m"
#define ANSI_COLOR_GREEN	 "\e[0;32m"
#define ANSI_COLOR_RESET     "\x1b[0m"
/*
explica��o do sem_trywait() => https://pubs.opengroup.org/onlinepubs/000095399/functions/sem_trywait.html
fechamento de thread => https://stackoverflow.com/questions/11624545/how-to-make-main-thread-wait-for-all-child-threads-finish
*/

#define QTD_CANIBAIS 10

#define MAX_TENTATIVAS_CACAR 3
#define QTD_COMIDAS 4

typedef struct
{
	int* insumos;
	int* caldeirao_comidas;
	int comidas_produzidas;
	int qtd_min_insumos_para_cozinhar;
	int max_caldeirao;
	int vezes_que_dormiu;

	sem_t* pode_comer;
	sem_t* pode_cozinhar;

	pthread_cond_t* cond;

	pthread_mutex_t* mutex_insumos;
	pthread_mutex_t* mutex_caldeirao_comidas;
} pkg_cozinheiro_t;

void pkg_cozinheiro_init(
	pkg_cozinheiro_t* pkg, int* insumos, int* caldeirao_comidas, int qtd_min_insumos_para_cozinhar, int max_caldeirao,
	sem_t* pode_comer, sem_t* pode_cozinhar,
	pthread_cond_t* cond,
	pthread_mutex_t* mutex_insumos, pthread_mutex_t* mutex_caldeirao_comidas
)
{
	pkg->insumos = insumos;
	pkg->caldeirao_comidas = caldeirao_comidas;
	pkg->comidas_produzidas = 0;
	pkg->qtd_min_insumos_para_cozinhar = qtd_min_insumos_para_cozinhar;
	pkg->max_caldeirao = max_caldeirao;
	pkg->vezes_que_dormiu = 0;
	pkg->pode_comer = pode_comer;
	pkg->pode_cozinhar = pode_cozinhar;
	pkg->cond = cond;
	pkg->mutex_insumos = mutex_insumos;
	pkg->mutex_caldeirao_comidas = mutex_caldeirao_comidas;
}

void* fcozinheiro(void* arg)
{
	pkg_cozinheiro_t* pkg = (pkg_cozinheiro_t*)arg;
	int ja_produzidos = 0;

	// Produz 1 por 1, at� M comidas, depois que chegar a M comidas libera para comer e dorme;
	
	while (1)
	{
		// verificar se os insumos s�o suficientes para cozinhar
		pthread_mutex_lock(pkg->mutex_insumos);

		// S� posso usar o cond wait fazendo o lock no mutex antes
		pkg->vezes_que_dormiu++;
		pthread_cond_wait(pkg->cond, pkg->mutex_insumos);
		
		if (*(pkg->insumos) < pkg->qtd_min_insumos_para_cozinhar)
		{
			printf(ANSI_COLOR_RED "Insumos %d \n" ANSI_COLOR_RESET, *(pkg->insumos));
			pthread_mutex_unlock(pkg->mutex_insumos);
		}
		else
		{
			pthread_mutex_unlock(pkg->mutex_insumos);

			// Canibais podem comer enquanto o cozinehro cozinha
			// O �ltimo a comer avisa que o caldeir�o vai zerar
			while (*(pkg->caldeirao_comidas) < pkg->max_caldeirao)
			{
				pthread_mutex_lock(pkg->mutex_insumos);
				(*(pkg->insumos))--; // valor 56 => 55
				pthread_mutex_unlock(pkg->mutex_insumos);
				pthread_mutex_lock(pkg->mutex_caldeirao_comidas);
				(*(pkg->caldeirao_comidas))++;
				//printf(ANSI_COLOR_RED "O Cozinheiro aumentou a comidas_produzidas para %d \n" ANSI_COLOR_RESET, *(pkg->caldeirao_comidas));
				pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
				sem_post(pkg->pode_comer);
				pkg->comidas_produzidas++;
			}
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
	int vezes_que_dormiu;
	int vezes_que_cacou;

	sem_t* pode_comer;
	sem_t* pode_cozinhar;

	pthread_cond_t* cond;

	pthread_mutex_t* mutex_insumos;
	pthread_mutex_t* mutex_caldeirao_comidas;
} pkg_canibal_t;

void pkg_canibal_init(
	pkg_canibal_t* pkg, int id, int* insumos, int* caldeirao_comidas,
	sem_t* pode_comer, sem_t* pode_cozinhar,
	pthread_cond_t* cond,
	pthread_mutex_t* mutex_insumos, pthread_mutex_t* mutex_caldeirao_comidas
)
{
	pkg->id = id;
	pkg->insumos = insumos;
	pkg->caldeirao_comidas = caldeirao_comidas;
	pkg->comidas_consumidas = 0;
	pkg->vezes_que_dormiu = 0;
	pkg->vezes_que_cacou = 0;
	pkg->pode_comer = pode_comer;
	pkg->pode_cozinhar = pode_cozinhar;
	pkg->cond = cond;
	pkg->mutex_insumos = mutex_insumos;
	pkg->mutex_caldeirao_comidas = mutex_caldeirao_comidas;
}

void* fcanibal(void* arg)
{
	pkg_canibal_t* pkg = (pkg_canibal_t*)arg;

	// 0.      Tenta comer
	// 0.1     Se n�o conseguir ele ca�a
	// 0.1.1   Tenta comer de novo
	// 0.1.1.1 Se n�o conseguir 3 vezes
	// 0.1.1.1 Se conseguir ele tenta comer de novo, e o processo repete

	int tentativas;
	//long execucoes = 0;

	while (1)
	{
		tentativas = 0;
		while(tentativas++ < MAX_TENTATIVAS_CACAR)
		{
			// tenta comer
			// se n�o conseguir ele ca�a
			int pode_comer = sem_trywait(pkg->pode_comer);
			if (pode_comer == 0)
			{
				printf("#%d - Conseguiu comer: %d\n", pkg->id, pode_comer);
				//printf(ANSI_COLOR_RED "Canibal #%d comeu %d \n" ANSI_COLOR_RESET, pkg->id, execucoes++);
				pthread_mutex_lock(pkg->mutex_caldeirao_comidas);
				(*(pkg->caldeirao_comidas))--;
				// Se ap�s o canibal comer do caldeirao e perceber que foi o �ltimo, ele ir� avisar o cozinheiro
				if (*(pkg->caldeirao_comidas) == 1)
				{
					pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
					pthread_cond_signal(pkg->cond);
				} else
				{
					pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
				}
				pkg->comidas_consumidas++;
			}
			else
			{
				pthread_mutex_lock(pkg->mutex_insumos);
				(*(pkg->insumos))++;
				//printf(ANSI_COLOR_RED "A Canibal #%d mudou o insumo de %d para %d \n" ANSI_COLOR_RESET, pkg->id, *(pkg->insumos) - 1, *(pkg->insumos));
				pthread_mutex_unlock(pkg->mutex_insumos);
				pkg->vezes_que_cacou++;
				pthread_cond_signal(pkg->cond);
			}
		}

		// espera at� conseguir comer
		pkg->vezes_que_dormiu++;
		sem_wait(pkg->pode_comer);

		pthread_mutex_lock(pkg->mutex_caldeirao_comidas);
		(*(pkg->caldeirao_comidas))--;
		pthread_mutex_unlock(pkg->mutex_caldeirao_comidas);
		pkg->comidas_consumidas++;
	}
	return NULL;
}

/*
canibais ca�am
insumos s�o coletados na ca�a
comida � gerada com base no insumo
cozinheiro s� cozinha com calder�o zerado com pelo menos 1 insumo
n�o existe um limite para insimuos => USAR int PRA GUARDAR
canibal que n�o comer ir� ca�ar, e depois de ca�ar espera a sua vez de comer
	ca�am apenas uma vez e depois ficam esperando a comida
eles sempre tem sucesso na ca�ada mas somente um insumo � conseguido por cada ca�ada
	apenas um ca�ador conseguir� ca�ar de fato
	apenas um insumo por ca�ada
	PARA ESSA PARTE PESQUISAR SOBRE A FUN��O try PARA SEMAFORO
	CUIDADO PARA N�O FAZER UM C�DIGO DE busy-wait
limitar execu��o at� 20 segundos
imprimir quanto
	cada canibal comeu
	quanto o coxinheiro cozinhou
	insumo restante no estoque
*/

/*
	pthread_cond_t;
	pthread_cond_init; Inicializa o condicional
	pthread_cond_signal; Envia sinal informando que pode continuar | Usado no canibal
	pthread_cond_broadcast; Todas as threads s�o liberadas
	pthread_cond_destroy; Finalizar a vari�vel
	pthread_cond_timedwait; Tenta acessar, esperar X segundos (como se fosse um sleep), se n�o conseguir ele sai
	pthread_cond_wait; Desloca o mutex e aguarda um sinal de que pode continuar
*/

int main(int argc, char** argv)
{
	int insumos = 0, caldeirao_comidas = 0, insumos_min = QTD_COMIDAS , max_caldeirao = QTD_COMIDAS;
	sem_t pode_comer, pode_cozinhar;
	sem_init(&pode_comer, 0, 0);
	sem_init(&pode_cozinhar, 0, 0);

	pthread_cond_t cond_COZINHAR;
	pthread_cond_init(&cond_COZINHAR, NULL);

	pthread_mutex_t mutex_insumos, mutex_caldeirao_comidas;
	pthread_mutex_init(&mutex_insumos, NULL);
	pthread_mutex_init(&mutex_caldeirao_comidas, NULL);

	pthread_t cozinheiro, * canibais = (pthread_t*)malloc(sizeof(pthread_t) * QTD_CANIBAIS);
	if (!canibais) return NULL;

	pkg_cozinheiro_t pkg_co;

	pkg_cozinheiro_init(&pkg_co, &insumos, &caldeirao_comidas, insumos_min, max_caldeirao, &pode_comer, &pode_cozinhar, &cond_COZINHAR, &mutex_insumos, &mutex_caldeirao_comidas);

	pthread_create(&cozinheiro, NULL, &fcozinheiro, (void*)&pkg_co);

	pkg_canibal_t* pkgs_ca = (pkg_canibal_t*)malloc(sizeof(pkg_canibal_t) * QTD_CANIBAIS);
	if (!pkgs_ca) return NULL;

	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		pkg_canibal_init(&pkgs_ca[i], i + 1, &insumos, &caldeirao_comidas, &pode_comer, &pode_cozinhar, &cond_COZINHAR, &mutex_insumos, &mutex_caldeirao_comidas);
		pthread_create(&canibais[i], NULL, &fcanibal, (void*)&pkgs_ca[i]);
	}

	Sleep(20000);

	printf("\n\nCANCELAMENTO DE THREADS\n");
	// Limpa Vari�veis
	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		pthread_cancel(canibais[i]);
	}

	pthread_cancel(cozinheiro);

	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		printf("Vou cancelar a thread #%d\n", pkgs_ca[i].id);
		pthread_join(canibais[i], NULL);
		printf("Cancelei a thread #%d\n", pkgs_ca[i].id);
	}

	printf("Vou cancelar a thread do cozinheiro\n");
	pthread_join(cozinheiro, NULL);
	printf("Cancelei a thread do cozinheiro\n");

	printf("\n *** CANCELAMENTO DE THREADS *** \n\n");


	pkg_canibal_t p;
	int consumo_canibal = 0;

	printf("Canibal\tCOMEU\tDORMIU\tCACOU\n");
	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		p = pkgs_ca[i];
		printf("%d\t%d\t%d\t%d\n", p.id, p.comidas_consumidas, p.vezes_que_dormiu, p.vezes_que_cacou);
		consumo_canibal += p.comidas_consumidas;
	}
	printf("----------------------------------\n");
	printf("Total consumido pelos canibais: %d\n", consumo_canibal);

	printf("\n");

	printf("Chef\tCOZINHOU\tDORMIU\n");
	printf("%d\t%d\t%d\n", 1, pkg_co.comidas_produzidas, pkg_co.vezes_que_dormiu);
	printf("Estoque\t%d\n", insumos);
	printf("Caldeirao\t%d\n", caldeirao_comidas);

	/*
	// quanto cada canibal comeu
	for (int i = 0; i < QTD_CANIBAIS; i++)
	{
		printf("O canibal #%d consumiu %d comidas\n", pkgs_ca[i].id, pkgs_ca[i].comidas_consumidas);
	}

	printf("O cozinheiro poroduziu %d comidas\n", pkg_co.comidas_produzidas);

	printf("Restaram %d insumos no estoque\n", insumos);

	*/

	// Caso algu�m esteja esperado com sem_wait com o sem_close eles s�o liberados
	sem_close(&pode_comer);
	sem_close(&pode_cozinhar);

	sem_destroy(&pode_comer);
	sem_destroy(&pode_cozinhar);

	pthread_cond_destroy(&cond_COZINHAR);

	pthread_mutex_destroy(&mutex_insumos);
	pthread_mutex_destroy(&mutex_caldeirao_comidas);

	return 0;
}