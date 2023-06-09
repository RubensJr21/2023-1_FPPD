// C�digo necess�rio para o Visual Studio n�o acusar fun��es inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)
#pragma warning(disable:6001)
#pragma warning(disable:6386)
// https://learn.microsoft.com/pt-br/cpp/code-quality/c6386?view=msvc-170

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

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
#define bignumber_t long long
#define primo_t long long
#define round_t int

#define TRUE 1
#define FALSE 0
#define bool_t int

typedef struct PKG_NUMBER_TO_VERIFY{
	bignumber_t number;
	round_t round;
	int contador;
	int qtd_de_threads;
	id_t id_Thread_que_resolveu;
	bignumber_t divided_number;
	bool_t eh_primo;
	sem_t* sem_pode_imprimir;
} pkg_number_to_veriry_t, * pkg_number_to_veriry_pt, numeros_primos_t, * numeros_primos_pt;

typedef struct CELL_OF_NUMBER_TO_VERIFY {
	pkg_number_to_veriry_pt atual;
	struct CELL_OF_NUMBER_TO_VERIFY* proximo;
} cell_of_number_to_verify_t, *cell_of_number_to_verify_pt;

cell_of_number_to_verify_pt create_cell_of_number_to_verify(pkg_number_to_veriry_pt ntv)
{
	cell_of_number_to_verify_pt contv = malloc(sizeof(cell_of_number_to_verify_t));
	contv->atual = ntv;
	contv->proximo = NULL;
	return contv;
}

typedef struct FILA_BUFFER_IO {
	cell_of_number_to_verify_pt head;
	cell_of_number_to_verify_pt last;
	sem_t* sem_mutex;
	sem_t* sem_vazio;
	sem_t* sem_cheio;
	int max_size;
	int current_size;
} fila_buffer_IO_t, *fila_buffer_IO_pt;

fila_buffer_IO_pt create_fila_buffer_IO(int max_size_buffer)
{
	fila_buffer_IO_pt fbIO = malloc(sizeof(fila_buffer_IO_t));
	fbIO->head = NULL;
	fbIO->last = NULL;

	fbIO->sem_mutex = malloc(sizeof(sem_t));
	sem_init(fbIO->sem_mutex, 0, 1);
	fbIO->sem_vazio = malloc(sizeof(sem_t));
	sem_init(fbIO->sem_vazio, 0, max_size_buffer); // Come�a com todas as posi��es livres para escrita
	fbIO->sem_cheio = malloc(sizeof(sem_t));
	sem_init(fbIO->sem_cheio, 0, 0); // Come�a com nenhuma posi��o livre para leitura
	
	fbIO->max_size = max_size_buffer;
	fbIO->current_size = 0;
	return fbIO;
}

cell_of_number_to_verify_pt get_from_fila_buffer_IO(fila_buffer_IO_pt fila_in, id_t id)
{
	cell_of_number_to_verify_pt cabeca;
	int o;
	sem_getvalue(fila_in->sem_cheio, &o);
	sem_wait(fila_in->sem_cheio);
	sem_wait(fila_in->sem_mutex);
	sem_getvalue(fila_in->sem_cheio, &o);

	cabeca = fila_in->head;

	fila_in->head = fila_in->head->proximo;

	if (fila_in->head == NULL)
	{
		fila_in->last = NULL;
	}
	fila_in->current_size--;

	sem_post(fila_in->sem_mutex);
	sem_post(fila_in->sem_vazio);

	sem_getvalue(fila_in->sem_vazio, &o);

	return cabeca;
}

void insert_in_fila_buffer_IO(fila_buffer_IO_pt fila_out, cell_of_number_to_verify_pt cell, id_t id)
{
	int o;
	sem_getvalue(fila_out->sem_vazio, &o);
	sem_wait(fila_out->sem_vazio);
	sem_wait(fila_out->sem_mutex);
	sem_getvalue(fila_out->sem_vazio, &o);

	if (fila_out->head == NULL)
	{
		fila_out->head = fila_out->last = cell;
	}
	else
	{
		fila_out->last->proximo = cell;
		fila_out->last = cell;
	}
	fila_out->current_size++;

	sem_post(fila_out->sem_mutex);
	sem_post(fila_out->sem_cheio);

	sem_getvalue(fila_out->sem_cheio, &o);
}

bool_t insert_in_fila_buffer_IO_GERADORA(fila_buffer_IO_pt fila_out, cell_of_number_to_verify_pt cell, id_t id)
{
	int o;
	sem_getvalue(fila_out->sem_vazio, &o);
	sem_wait(fila_out->sem_vazio);
	sem_wait(fila_out->sem_mutex);
	sem_getvalue(fila_out->sem_vazio, &o);


	if (fila_out->current_size + 2 >= fila_out->max_size) // verifica se a Thread geradora vai encher a fila
	{
		sem_post(fila_out->sem_mutex);
		sem_post(fila_out->sem_vazio);
		return FALSE;
	}

	if (fila_out->head == NULL) // Fila vazia
	{
		fila_out->head = fila_out->last = cell;
	}
	else
	{
		fila_out->last->proximo = cell;
		fila_out->last = cell;
	}
	fila_out->current_size++;

	sem_post(fila_out->sem_mutex);
	sem_post(fila_out->sem_cheio);

	sem_getvalue(fila_out->sem_cheio, &o);

	return TRUE;
}


pkg_number_to_veriry_pt create_pkg_number_to_veriry(bignumber_t number, int qtd_de_threads)
{
	pkg_number_to_veriry_pt pkg = malloc(sizeof(pkg_number_to_veriry_t));
	pkg->number = number;
	pkg->round = 0;
	pkg->contador = 0;
	pkg->qtd_de_threads = qtd_de_threads;
	pkg->id_Thread_que_resolveu = -1;
	pkg->divided_number = -1;
	pkg->eh_primo = FALSE;
	pkg->sem_pode_imprimir = malloc(sizeof(sem_t));
	sem_init(pkg->sem_pode_imprimir, 0, 0);
	return pkg;
}

typedef struct BUFFER_RESULTADOS {
	int current_index;
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer; // usado para calcular o round do number_to_verify
	pthread_mutex_t* mutex;
	sem_t* sem;
}buffer_resultados_t, * buffer_resultados_pt;

#define INSERT_IN_BUFFER_RESULTADOS_SUCESSO 1
#define INSERT_IN_BUFFER_RESULTADOS_FALHOU -1

int insert_in_buffer_resultados(buffer_resultados_pt buffer_resultados, pkg_number_to_veriry_pt ntv, id_t thread_id)
{
	pthread_mutex_lock(buffer_resultados->mutex);
	buffer_resultados->buffer[ntv->number - 2] = ntv;
	pthread_mutex_unlock(buffer_resultados->mutex);
	return INSERT_IN_BUFFER_RESULTADOS_SUCESSO;
}

pkg_number_to_veriry_pt get_from_buffer_resultados(buffer_resultados_pt buffer_resultados, int index, id_t thread_id)
{
	pkg_number_to_veriry_pt ntv;
	pthread_mutex_lock(buffer_resultados->mutex);
	ntv = buffer_resultados->buffer[index];
	pthread_mutex_unlock(buffer_resultados->mutex);
	return ntv;
}

buffer_resultados_pt create_buffer_resultados(int numeros_a_serem_calculados)
{
	buffer_resultados_pt b = malloc(sizeof(buffer_resultados_t));
	b->buffer = malloc(sizeof(pkg_number_to_veriry_pt) * numeros_a_serem_calculados - 2); // pois ser�o exclu�dos os n�meros 0 e 1, e o processamento come�ar� do 2
	b->current_index = 0;
	for (int index = 0; index < numeros_a_serem_calculados - 2; index++) // O - 2 � porque os n�meros come�am a serem calculados do 2, logo, pulando o 0 e o 1.
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = numeros_a_serem_calculados - 2; // O - 2 � porque os n�meros come�am a serem calculados do 2, logo, pulando o 0 e o 1.
	b->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(b->mutex, NULL);
	b->sem = malloc(sizeof(sem_t));
	sem_init(b->sem, 0, 1);
	return b;
}

/////////////////////////////////////////////////////////////////////////////////

typedef struct BUFFER_DE_PRIMOS{
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer; // usado para calcular o round do number_to_verify
}buffer_de_primos_t, * buffer_de_primos_pt;

void insert_in_buffer_internal_primos(buffer_de_primos_pt buffer_primos, pkg_number_to_veriry_pt number_to_verify, int id)
{
	buffer_primos->buffer[number_to_verify->round] = number_to_verify;
}

primo_t get_number_from_buffer_internal_primos(buffer_de_primos_pt buffer_primos, int index, int id)
{
	// Caso -1
	if (index >= buffer_primos->max_size_buffer) // quer dizer que o espa�o no buffer acabou, sendo assim encerrando o processo
	{
		return -2;
	}

	pkg_number_to_veriry_pt number_to_verify = buffer_primos->buffer[index];

	if (number_to_verify == NULL)
	{
		return -1;
	}
	else
	{
		return number_to_verify->number;
	}
}

buffer_de_primos_pt create_buffer_internal_primos(int max_size_buffer)
{
	buffer_de_primos_pt b = malloc(sizeof(buffer_de_primos_t));
	b->buffer = malloc(sizeof(primo_t) * max_size_buffer);
	for (int index = 0; index < max_size_buffer; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = max_size_buffer;
	return b;
}

typedef struct BUFFER_IO{
	fila_buffer_IO_pt buffer;
	int max_size_buffer;
}buffer_IO_t, * buffer_IO_pt;

void insert_in_buffer_IO(buffer_IO_pt buffer_out, pkg_number_to_veriry_pt ntv, id_t id)
{
	insert_in_fila_buffer_IO(buffer_out->buffer, create_cell_of_number_to_verify(ntv), id);
}

pkg_number_to_veriry_pt get_from_buffer_IO(buffer_IO_pt buffer_in, id_t id)
{
	return get_from_fila_buffer_IO(buffer_in->buffer, id)->atual;
}

buffer_IO_pt create_buffer_IO(int max_size_buffer)
{
	buffer_IO_pt b = malloc(sizeof(buffer_IO_t));
	b->buffer = create_fila_buffer_IO(max_size_buffer);
	b->max_size_buffer = max_size_buffer;
	return b;
}

typedef struct END_MAIN{
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
} end_main_t, *end_main_pt;

end_main_pt create_end_main()
{
	end_main_pt em = malloc(sizeof(end_main_t));
	em->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(em->mutex, NULL);
	em->cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(em->cond, NULL);
	return em;
}

typedef struct PKG_THREAD_GERADORA{
	id_t id;
	int numeros_a_serem_gerados;
	buffer_IO_pt buffer_out;
	int qtd_de_threads;
	buffer_resultados_pt buffer_resultados;
}pkg_thread_geradora_t, * pkg_thread_geradora_pt;

pkg_thread_geradora_pt create_pkg_thread_geradora(id_t id, int size_buffer, int qtd_threads, int numeros_a_serem_gerados, buffer_resultados_pt buffer_resultados)
{
	buffer_IO_pt buffer = create_buffer_IO(size_buffer);
	pkg_thread_geradora_pt tg = malloc(sizeof(pkg_thread_geradora_t));
	tg->id = id;
	tg->numeros_a_serem_gerados = numeros_a_serem_gerados;
	tg->qtd_de_threads = qtd_threads;
	tg->buffer_out = buffer;
	tg->buffer_resultados = buffer_resultados;
	return tg;
}

void* thread_geradora(void* args)
{
	pkg_thread_geradora_pt pkg = (pkg_thread_geradora_pt)args;
	buffer_IO_pt buffer_out = pkg->buffer_out;
	int qtd_de_threads = pkg->qtd_de_threads;
	bignumber_t number = 2;
	pkg_number_to_veriry_pt ntv;
	bool_t position_free = FALSE;
	int numeros_gerados = 0;
	bool_t deu_certo = TRUE;
	ntv = create_pkg_number_to_veriry(number, qtd_de_threads);
	insert_in_buffer_resultados(pkg->buffer_resultados, ntv, pkg->id);
	sem_post(pkg->buffer_resultados->sem); // garante que a thread resultado tenha o number_to_verify (ntv) para ler

	while (numeros_gerados < pkg->numeros_a_serem_gerados)
	{
		fila_buffer_IO_pt fila_out = buffer_out->buffer;
		cell_of_number_to_verify_pt cell = create_cell_of_number_to_verify(ntv);

		deu_certo = insert_in_fila_buffer_IO_GERADORA(buffer_out->buffer, cell, pkg->id);

		if (deu_certo) // garante que a Thread Geradora n�o vai encher o buffer da primeira thread
		{
			number++;
			numeros_gerados++;
			ntv = create_pkg_number_to_veriry(number, qtd_de_threads);
			insert_in_buffer_resultados(pkg->buffer_resultados, ntv, pkg->id);
			sem_post(pkg->buffer_resultados->sem); // garante que a thread resultado tenha o number_to_verify (ntv) para ler	
		}
	}
	return NULL;
}

typedef struct CONDICAO_DE_PARADA_THREADS_PROCESSAMENTO {
	pthread_mutex_t* mutex;
	bool_t deve_continuar;
} condicao_de_parada_threads_processamento_t, *condicao_de_parada_threads_processamento_pt;

condicao_de_parada_threads_processamento_pt create_condicao_de_parada_threads_processamento()
{
	condicao_de_parada_threads_processamento_pt cptp = malloc(sizeof(condicao_de_parada_threads_processamento_t));
	cptp->deve_continuar = TRUE;
	cptp->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(cptp->mutex, NULL);
	return cptp;
}

typedef struct PKG_THREAD_SIEVE_PROCESSAMENTO{
	id_t id;
	buffer_IO_pt buffer_in; // tamanho K
	buffer_IO_pt buffer_out; // tamanho K
	buffer_resultados_pt buffer_resultados;
	int size_buffer_internal; // valor do X

	sem_t* sem_end_process;
	condicao_de_parada_threads_processamento_pt condicao_parada;
}pkg_thread_sieve_processamento_t, * pkg_thread_sieve_processamento_pt;

pkg_thread_sieve_processamento_pt create_pkg_thread_sieve_processamento(id_t id, buffer_IO_pt buffer_in, buffer_IO_pt buffer_out, int buffer_size_internal, buffer_resultados_pt buffer_resultados, sem_t* sem_end_process, condicao_de_parada_threads_processamento_pt cptp)
{
	pkg_thread_sieve_processamento_pt tsp = malloc(sizeof(pkg_thread_sieve_processamento_t));
	tsp->id = id;
	tsp->buffer_in = buffer_in;
	tsp->buffer_out = buffer_out;
	tsp->buffer_resultados = buffer_resultados;
	tsp->size_buffer_internal = buffer_size_internal;
	tsp->sem_end_process = sem_end_process;
	tsp->condicao_parada = cptp;
	return tsp;
}

void update_round(pkg_number_to_veriry_pt number_to_verify)
{
	// incrementa number_to_verify->contador
	number_to_verify->contador++;
	// atualiza o number_to_verify->round
	number_to_verify->round = (int)(number_to_verify->contador / number_to_verify->qtd_de_threads); // retorna valor inteiro que corresponde ao round
}

void* thread_sieve_processamento(void* args)
{
	pkg_thread_sieve_processamento_pt pkg = (pkg_thread_sieve_processamento_pt)args;

	buffer_IO_pt buffer_in = pkg->buffer_in;
	buffer_IO_pt buffer_out = pkg->buffer_out;
	buffer_de_primos_pt buffer_primos = create_buffer_internal_primos(pkg->size_buffer_internal);
	pkg_number_to_veriry_pt ntv;
	// verifica por meio de um mutex se deve continuar ou n�o
	// Vai recever um atributo que indica se precisa continuar ou n�o
	// Isso servir� para que todos par�m "ao mesmo tempo" e n�o imprimam mais nada
	condicao_de_parada_threads_processamento_pt condicao_parada = pkg->condicao_parada;
	pthread_mutex_lock(condicao_parada->mutex);
	while (condicao_parada->deve_continuar)
	{
		pthread_mutex_unlock(condicao_parada->mutex);
		/*
			Casos poss�veis:
			-1. Buffer estourado
			0. N�o ter n�mero no vetor de primos e ele ser automaticamente primo
			1. Ser primo:
				1.1: N�o foi poss�vel dividir
					=> Passa para a pr�xima thread e e pega o pr�ximo do buffer
			2. N�o ser primo
				2.1: Descartar o n�mero, pegando o pr�ximo do buffer
		*/
		
		ntv = get_from_buffer_IO(buffer_in, pkg->id);
				
		// obt�m n�mero primo que tentar� dividir o ntv

		primo_t numero_do_vetor_de_primos = get_number_from_buffer_internal_primos(buffer_primos, ntv->round, pkg->id);
		// verifica se existe algum n�mero no buffer de primos na posi��o ntv->round
		// numero_do_vetor_de_primos == -1; n�o existe n�mero naquela posi��o
		// numero_do_vetor_de_primos != -1; existe n�mero naquela posi��o
		// Caso -1
		// buffer n�o tem mais espa�o, encerra o processamento de todas as threads
		if (numero_do_vetor_de_primos == -2)
		{
			// aqui deve parar de rodar o c�digo
			sem_post(pkg->sem_end_process);
			pthread_mutex_lock(condicao_parada->mutex);
			condicao_parada->deve_continuar = FALSE;
			pthread_mutex_unlock(condicao_parada->mutex);
			printf("%lld CAUSED INTERNAL BUFFER OVERFLOW IN thread %d at round %d\n", ntv->number, pkg->id, ntv->round);
			break;
		}
		// Caso 0
		else if (numero_do_vetor_de_primos == -1)
		{
			ntv->id_Thread_que_resolveu = pkg->id;
			ntv->eh_primo = TRUE;
			insert_in_buffer_internal_primos(buffer_primos, ntv, pkg->id);
			// enviar para thread resultado
			sem_post(ntv->sem_pode_imprimir);
		}
		else
		{
			// verificar se o ntv � divis�vel pelo numero_do_vetor_de_primos
			bignumber_t resto_da_divisao = ntv->number % numero_do_vetor_de_primos;
			// resto_da_divisao == 0, � divis�vel. N�o � primo, descarta
			// resto_da_divisao != 0, n�o � divis�vel
			// Caso 1
			if (resto_da_divisao != 0)
			{
				update_round(ntv);

				insert_in_buffer_IO(buffer_out, ntv, pkg->id);
				}
			else
			{
				ntv->id_Thread_que_resolveu = pkg->id;
				ntv->eh_primo = FALSE;
				ntv->divided_number = numero_do_vetor_de_primos;
				// enviar para thread resultado
				sem_post(ntv->sem_pode_imprimir);
			}
		}
		pthread_mutex_lock(condicao_parada->mutex);
	}
	return NULL;
}

typedef struct PKG_THREAD_RESULTADO{
	id_t id;
	buffer_resultados_pt buffer_resultados;
	sem_t* end_process;
	end_main_pt end_main;
	condicao_de_parada_threads_processamento_pt condicao_de_parada_thread_processamento;
}pkg_thread_resultado_t, * pkg_thread_resultado_pt;

pkg_thread_resultado_pt create_pkg_thread_resultado(id_t id, int size_buffer_resultados, sem_t* end_process, end_main_pt end_main, condicao_de_parada_threads_processamento_pt cptp)
{
	pkg_thread_resultado_pt tr = malloc(sizeof(pkg_thread_resultado_t));
	tr->id = id;
	tr->buffer_resultados = create_buffer_resultados(size_buffer_resultados);
	tr->end_process = end_process;
	tr->end_main = end_main;
	tr->condicao_de_parada_thread_processamento = cptp;
	return tr;
}

void* thread_resultado(void* args)
{
	pkg_thread_resultado_pt pkg = (pkg_thread_resultado_pt) args;
	buffer_resultados_pt buffer_resultados = pkg->buffer_resultados;
	pkg_number_to_veriry_pt ntv;
	for (int i = 0; i < buffer_resultados->max_size_buffer; i++)
	{
		do
		{
			sem_wait(buffer_resultados->sem);
			ntv = get_from_buffer_resultados(buffer_resultados, i, pkg->id);
		} while (ntv == NULL);
		sem_wait(ntv->sem_pode_imprimir);

		if (ntv->eh_primo)
		{
			 printf("%lld is prime in thread %d at round %d\n", ntv->number, ntv->id_Thread_que_resolveu, ntv->round);
		}
		else
		{
			 printf("%lld divided by %lld in thread %d at round %d\n", ntv->number, ntv->divided_number, ntv->id_Thread_que_resolveu, ntv->round);
		}
		if (sem_trywait(pkg->end_process) == 0)
		{
			pthread_cond_signal(pkg->end_main->cond);
			break;
		}
	}	
	return NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		return 0;
	}
	pthread_t t_geradora, t_resultado;

	pthread_t* ts_sieves_processamento;
	pkg_thread_sieve_processamento_pt* pkgs_tsp;
	
	/*
		N: quantidade de primos que a devem ter na thread RESULTADO
		M : quantidade de threads de processamento
		K : tamanho dos buffers de comunica��o
		X : tamanho do buffer de primos das threads de processamento
		buffer interno(que guarda os numeros primos)
	*/
	// N                               	         // M                                             // K                                          // X
	//int numero_a_serem_testados = 1000,          qtd_de_threads_de_processamento = 3,             max_size_comunication_buffer = 10,            max_size_internal_buffer = 50;
	int numero_a_serem_testados = atoi(argv[1]), qtd_de_threads_de_processamento = atoi(argv[2]), max_size_comunication_buffer = atoi(argv[3]), max_size_internal_buffer = atoi(argv[4]);

	// CRIA��O DOS CONTROLES DE ENCERRAMENTO DO PROGRAMA
	end_main_pt end_main = create_end_main();
	sem_t sem_end_process;
	sem_init(&sem_end_process, 0, 0);
	condicao_de_parada_threads_processamento_pt cptp = create_condicao_de_parada_threads_processamento();
	// FIM CRIA��O DOS CONTROLES DE ENCERRAMENTO DO PROGRAMA

	pkg_thread_resultado_pt pkg_tr = create_pkg_thread_resultado(1, numero_a_serem_testados, &sem_end_process, end_main, cptp);
	buffer_resultados_pt buffer_resultados_numeros_primos = pkg_tr->buffer_resultados;

	pkg_thread_geradora_pt pkg_tg = create_pkg_thread_geradora(0, max_size_comunication_buffer, qtd_de_threads_de_processamento, numero_a_serem_testados, buffer_resultados_numeros_primos);
	buffer_IO_pt buffer_out_geradora = pkg_tg->buffer_out;

	pthread_create(&t_geradora, NULL, &thread_geradora, (void*)pkg_tg);
	pthread_create(&t_resultado, NULL, &thread_resultado, (void*)pkg_tr);

	buffer_IO_pt buffer_in_tsp = buffer_out_geradora;
	buffer_IO_pt buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);

	ts_sieves_processamento = malloc(sizeof(pthread_t) * qtd_de_threads_de_processamento);
	pkgs_tsp = malloc(sizeof(pkg_thread_sieve_processamento_pt) * qtd_de_threads_de_processamento);

	id_t id;


	for (id = 0; id < qtd_de_threads_de_processamento - 1; id++) // id <= qtd_de_threads_de_processamento - 1 para parar na �ltima e ser criada manualmente
	{
		pkgs_tsp[id] = create_pkg_thread_sieve_processamento(id + 2, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos, &sem_end_process, cptp);

		buffer_in_tsp = buffer_out_tsp;
		buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);
	}

	buffer_out_tsp = buffer_out_geradora;
	pkgs_tsp[id] = create_pkg_thread_sieve_processamento(id + 2, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos, &sem_end_process, cptp);

	for (id = 0; id < qtd_de_threads_de_processamento; id++)
	{
		pthread_create(&ts_sieves_processamento[id], NULL, &thread_sieve_processamento, (void*)pkgs_tsp[id]);
	}
	pthread_mutex_lock(end_main->mutex);
	// espera receber um sinal de que pode parar o processo
	pthread_cond_wait(end_main->cond, end_main->mutex);
	pthread_mutex_unlock(end_main->mutex);

	pthread_cancel(t_geradora);

	for (id = 0; id < qtd_de_threads_de_processamento; id++)
	{
		pthread_cancel(ts_sieves_processamento[id]);
	}

	return 1;
}