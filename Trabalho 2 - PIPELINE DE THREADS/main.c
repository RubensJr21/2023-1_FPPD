// C�digo necess�rio para o Visual Studio n�o acusar fun��es inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)
#pragma warning(disable:6386)
// https://learn.microsoft.com/pt-br/cpp/code-quality/c6386?view=msvc-170

#include <pthread.h>
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include<math.h>

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

#define ANSI_COLOR_RED     	 "\x1b[31m" //cores em ANSI utilizadas 
#define ANSI_COLOR_GRAY    	 "\e[0;37m"
#define ANSI_COLOR_DARK_GRAY "\e[1;30m"
#define ANSI_COLOR_GREEN	 "\e[0;32m"
#define ANSI_COLOR_RESET     "\x1b[0m"

#define id_t int
//#define buffer_primos_pt long long*
#define bignumber_t long long
#define primo_t long long
#define round_t int

#define TRUE 1
#define FALSE 0
#define bool_t int

#define MULTIPLAS_THREADS TRUE

typedef struct {
	bignumber_t number;
	round_t round;
	int contador;
	int qtd_de_threads;
	id_t id_Thread_que_resolveu;
} pkg_number_to_veriry_t, *pkg_number_to_veriry_pt, numeros_primos_t, *numeros_primos_pt;

pkg_number_to_veriry_pt create_pkg_number_to_veriry(bignumber_t number, int qtd_de_threads)
{
	pkg_number_to_veriry_pt pkg = malloc(sizeof(pkg_number_to_veriry_t));
	pkg->number = number;
	pkg->round = 0;
	pkg->contador = 0;
	pkg->qtd_de_threads = qtd_de_threads;
	pkg->id_Thread_que_resolveu = -1;
	return pkg;
}

typedef struct {
	int current_index;
	pkg_number_to_veriry_pt* buffer; // tamanho = X/ln(x)
	int max_size_buffer; // usado para calcular o round do number_to_verify
	pthread_mutex_t* mutex;
	pthread_cond_t* cond; // usado na inser��o e no get
}buffer_resultados_t, * buffer_resultados_pt;

void insert_in_buffer_resultados(buffer_resultados_pt buffer_resultados, pkg_number_to_veriry_pt number_to_verify)
{
	pthread_mutex_lock(buffer_resultados->mutex);
	// envia o n�mero para a primeira thread de processamento
	buffer_resultados->buffer[buffer_resultados->current_index++] = number_to_verify;
	pthread_mutex_unlock(buffer_resultados->mutex);
}

primo_t get_number_from_buffer_resultados(buffer_resultados_pt buffer_resultados)
{
	pthread_mutex_lock(buffer_resultados->mutex);
	pkg_number_to_veriry_pt number_to_verify = buffer_resultados->buffer[buffer_resultados->current_index];
	while (number_to_verify == NULL)
	{
		pthread_cond_wait(buffer_resultados->cond, buffer_resultados->mutex);
		number_to_verify = buffer_resultados->buffer[buffer_resultados->current_index];
	}
	buffer_resultados->current_index++;
	pthread_mutex_unlock(buffer_resultados->mutex);
	return number_to_verify->number;
}

buffer_resultados_pt create_buffer_resultados(int numeros_a_serem_calculados)
{
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_t* cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond, NULL);

	buffer_resultados_pt b = malloc(sizeof(buffer_resultados_t));
	// https://www.clubedohardware.com.br/forums/topic/933851-como-fazer-lnx-em-c/
	int max_size_buffer = (int)(numeros_a_serem_calculados / log(numeros_a_serem_calculados));
	b->buffer = malloc(sizeof(pkg_number_to_veriry_pt) * max_size_buffer); // tamanho = X / ln(x)
	b->current_index = 0;
	for (int index = 0; index < max_size_buffer; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = max_size_buffer;
	b->mutex = mutex;
	b->cond = cond; // usado na inser��o e no get
	return b;
}

/////////////////////////////////////////////////////////////////////////////////

typedef struct {
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer; // usado para calcular o round do number_to_verify
	pthread_mutex_t* mutex;
}buffer_de_primos_t, *buffer_de_primos_pt;

void insert_in_buffer_internal_primos(buffer_de_primos_pt buffer_primos, pkg_number_to_veriry_pt number_to_verify)
{
	pthread_mutex_lock(buffer_primos->mutex);
	// envia o n�mero para a primeira thread de processamento
	buffer_primos->buffer[number_to_verify->round] = number_to_verify;
	pthread_mutex_unlock(buffer_primos->mutex);
}

primo_t get_number_from_buffer_internal_primos(buffer_de_primos_pt buffer_primos, int index)
{
	pthread_mutex_lock(buffer_primos->mutex);
	// Caso -1
	if (index >= buffer_primos->max_size_buffer) // quer dizer que o espa�o no buffer acabou, sendo assim encerrando o processo
	{
		return -2;
	}
	pkg_number_to_veriry_pt number_to_verify = buffer_primos->buffer[index];
	pthread_mutex_unlock(buffer_primos->mutex);
	return number_to_verify->number;
}

buffer_de_primos_pt create_buffer_internal_primos(int max_size_buffer)
{
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);

	buffer_de_primos_pt b = malloc(sizeof(buffer_de_primos_t));
	b->buffer = malloc(sizeof(primo_t) * max_size_buffer);
	for (int index = 0; index < max_size_buffer; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = max_size_buffer;
	b->mutex = mutex;
	return b;
}

typedef struct {
	int current_index_writing;
	int current_index_reading;
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer;
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
}buffer_IO_t, *buffer_IO_pt;

void insert_in_buffer_IO(buffer_IO_pt buffer, pkg_number_to_veriry_pt number_to_verify)
{
	bool_t position_free = FALSE;
	pthread_mutex_lock(buffer->mutex);
	// envia o n�mero para a primeira thread de processamento
	position_free = buffer->buffer[buffer->current_index_writing] == NULL;
	if (position_free)
	{
		buffer->buffer[buffer->current_index_writing++] = number_to_verify;
		// buffer_circular
		buffer->current_index_writing = (buffer->current_index_writing % buffer->max_size_buffer);
		pthread_cond_signal(buffer->cond);
	}
	else
	{
		// esperar liberar
		// usar cond
		pthread_cond_wait(buffer->cond, buffer->mutex);
		buffer->buffer[buffer->current_index_writing++] = number_to_verify;
		// buffer_circular
		buffer->current_index_writing = (buffer->current_index_writing % buffer->max_size_buffer);
	}
	pthread_mutex_unlock(buffer->mutex);
}

pkg_number_to_veriry_pt get_number_from_buffer_IO(buffer_IO_pt buffer)
{
	pthread_mutex_lock(buffer->mutex);
	pkg_number_to_veriry_pt number = buffer->buffer[buffer->current_index_reading];
	// buffer_circular
	if (number == NULL)
	{
		pthread_cond_wait(buffer->cond, buffer->mutex);
		number = buffer->buffer[buffer->current_index_reading];
	}
	// indicar que posi��o est� livre
	buffer->buffer[buffer->current_index_reading++] = NULL;
	// atualizar index
	buffer->current_index_reading = (buffer->current_index_reading % buffer->max_size_buffer);
	pthread_mutex_unlock(buffer->mutex);
	return number;
}

buffer_IO_pt create_buffer_IO(int max_size_buffer)
{
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_t* cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond, NULL);

	buffer_IO_pt b = malloc(sizeof(buffer_IO_t));
	b->current_index_writing = 0;
	b->current_index_reading = 0;
	b->buffer = malloc(sizeof(pkg_number_to_veriry_t) * max_size_buffer);
	for (int index = 0; index < max_size_buffer; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = max_size_buffer;
	b->mutex = mutex;
	b->cond = cond;
	return b;
}

typedef struct {
	id_t id;
	int numeros_a_serem_gerados;
	buffer_IO_pt buffer_out;
	int qtd_de_threads;
}pkg_thread_geradora_t, *pkg_thread_geradora_pt;

pkg_thread_geradora_pt create_pkg_thread_geradora(id_t id, int size_buffer, int qtd_threads, int numeros_a_serem_gerados)
{
	buffer_IO_pt buffer = create_buffer_IO(size_buffer);
	pkg_thread_geradora_pt tg = malloc(sizeof(pkg_thread_geradora_t));
	tg->id = id;
	tg->numeros_a_serem_gerados = numeros_a_serem_gerados;
	tg->buffer_out = buffer;
	return tg;
}

void* thread_geradora(void* args)
{
	pkg_thread_geradora_pt pkg = (pkg_thread_geradora_pt)args;
	buffer_IO_pt buffer = pkg->buffer_out;
	int qtd_de_threads = pkg->qtd_de_threads;
	bignumber_t number = 1;
	pkg_number_to_veriry_pt ntv;
	bool_t position_free = FALSE;
	int numeros_gerados = 0;
	while (numeros_gerados < pkg->numeros_a_serem_gerados)
	{
		ntv = create_pkg_number_to_veriry(number, qtd_de_threads);
		printf("numero gerado: %lld\n", number);
		insert_in_buffer_IO(buffer, ntv);
		number++;
	}
	return NULL;
}

typedef struct {
	id_t id;
	buffer_IO_pt buffer_in; // tamanho K
	buffer_IO_pt buffer_out; // tamanho K
	buffer_resultados_pt buffer_resultados;
	int size_buffer_internal; // valor do X
}pkg_thread_sieve_processamento_t, * pkg_thread_sieve_processamento_pt;

pkg_thread_sieve_processamento_pt create_pkg_thread_sieve_processamento(id_t id, buffer_IO_pt buffer_in, buffer_IO_pt buffer_out, int buffer_size_internal, buffer_resultados_pt buffer_resultados)
{
	pkg_thread_sieve_processamento_pt tsp = malloc(sizeof(pkg_thread_sieve_processamento_t));
	tsp->id = id;
	tsp->buffer_in = buffer_in;
	tsp->buffer_out = buffer_out;
	tsp->buffer_resultados = buffer_resultados;
	tsp->size_buffer_internal = buffer_size_internal;
	return tsp;
}

void update_round(pkg_number_to_veriry_pt number_to_verify)
{
	// incrementa number_to_verify->contador
	number_to_verify->contador++;
	// atualiza o number_to_verify->round
	number_to_verify->round = number_to_verify->contador / number_to_verify->qtd_de_threads; // retorna valor inteiro que corresponde ao round
}

bool_t process_number(id_t processing_thread, buffer_IO_pt buffer_out, buffer_de_primos_pt buffer_primos, pkg_number_to_veriry_pt number_to_verify)
{
	// obt�m n�mero primo que tentar� dividir o number_to_verify
	primo_t numero_do_vetor_de_primos = get_number_from_buffer_internal_primos(buffer_primos, number_to_verify->round); // Nunca retornar� NULL, pois existe um 
	// verifica se existe algum n�mero no buffer de primos na posi��o number_to_verify->round
	// numero_do_vetor_de_primos == -1; n�o existe n�mero naquela posi��o
	// numero_do_vetor_de_primos != -1; existe n�mero naquela posi��o
	// Caso -1
	// buffer n�o tem mais espa�o, encerra o processamento de todas as threads
	if (numero_do_vetor_de_primos == -2)
	{
		return FALSE;
	}
	// Caso 0
	if (numero_do_vetor_de_primos == -1)
	{
		// insere n�mero
		number_to_verify->id_Thread_que_resolveu = processing_thread;
		insert_in_buffer_internal_primos(buffer_primos, number_to_verify);
		printf(ANSI_COLOR_GREEN "numero %lld foi verificado como PRIMO na Thread #%d\n", number_to_verify->number, processing_thread);
		printf(ANSI_COLOR_RESET);
		// enviar para thread resultado
	}
	else
	{
		// verificar se o number_to_verify � divis�vel pelo numero_do_vetor_de_primos
		bignumber_t resultado = number_to_verify->number % numero_do_vetor_de_primos;
		// resultado == 0, � divis�vel. N�o � primo, descarta
		// resultado != 0, n�o � divis�vel
		// Caso 1
		if (resultado != 0)
		{
			// incrementa number_to_verify->contador e atualiza o number_to_verify->round
			update_round(number_to_verify);
			// passa para a pr�xima thread => escrver no buffer_out
			insert_in_buffer_IO(buffer_out, number_to_verify);
		}
		else
		{
			printf(ANSI_COLOR_RED);
			printf("numero %lld foi verificado como N�O PRIMO na Thread #%d\n", number_to_verify->number, processing_thread);
			printf(ANSI_COLOR_RESET);
		}
	}
	return TRUE;
}

void* thread_sieve_processamento(void* args)
{
	pkg_thread_sieve_processamento_pt pkg = (pkg_thread_sieve_processamento_pt)args;
	buffer_IO_pt buffer_in = pkg->buffer_in;
	buffer_IO_pt buffer_out = pkg->buffer_out;
	buffer_de_primos_pt buffer_primos = create_buffer_internal_primos(pkg->size_buffer_internal);
	pkg_number_to_veriry_pt ntv;
	bool_t continuar_processamento = TRUE;
	while (continuar_processamento)
	{
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
		pthread_mutex_lock(buffer_in->mutex);
		// obt�m n�mero que ser� verificado
		ntv = get_number_from_buffer_IO(buffer_in); // fun��o assincrona
		// processar()
		continuar_processamento = process_number(pkg->id, buffer_out, buffer_primos, ntv);
	}
	return NULL;
}

typedef struct {
	id_t id;
	buffer_resultados_pt buffer_resultados; // tamanho = X/ln(x)
}pkg_thread_resultado_t, * pkg_thread_resultado_pt;

pkg_thread_resultado_pt create_pkg_thread_resultado(id_t id, int size_buffer_resultados)
{
	pkg_thread_resultado_pt ts = malloc(sizeof(pkg_thread_resultado_t));
	ts->id = id;
	ts->buffer_resultados = create_buffer_resultados(size_buffer_resultados);
	return ts;
}

void* thread_resultado(void* args)
{
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t t_geradora, t_resultado;
	pthread_t t_sieve_processamento;

	pthread_t* ts_sieves_processamento;
	pkg_thread_sieve_processamento_pt* pkgs_tsp;

	/*
		N: quantidade de primos que a devem ter na thread RESULTADO
		M : quantidade de threads de processamento
		K : tamanho dos buffers de comunica��o
		X : tamanho do buffer de primos das threads de processamento
		buffer interno(que guarda os numeros primos)
	*/
	// N
	int numero_a_serem_testados = 131313;
	// M
	int qtd_de_threads_de_processamento = 4;
	// K
	int max_size_comunication_buffer = 10;
	// X
	int max_size_internal_buffer = 5;
	
	pkg_thread_geradora_pt pkg_tg = create_pkg_thread_geradora(1, max_size_comunication_buffer, qtd_de_threads_de_processamento, numero_a_serem_testados);
	buffer_IO_pt buffer_out_geradora = pkg_tg->buffer_out;

	pkg_thread_resultado_pt pkg_tr = create_pkg_thread_resultado(2, numero_a_serem_testados);
	buffer_resultados_pt buffer_resultados_numeros_primos = pkg_tr->buffer_resultados;

	pthread_create(&t_geradora, NULL, &thread_geradora, (void*)pkg_tg);
	pthread_create(&t_resultado, NULL, &thread_resultado, (void*)pkg_tr);

	buffer_IO_pt buffer_in_tsp = buffer_out_geradora;
	buffer_IO_pt buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);
	
	if (MULTIPLAS_THREADS)
	{
		ts_sieves_processamento = malloc(sizeof(pthread_t) * qtd_de_threads_de_processamento);
		pkgs_tsp = malloc(sizeof(pkg_thread_sieve_processamento_pt) * qtd_de_threads_de_processamento);

		for (id_t id = 3; id <= qtd_de_threads_de_processamento - 2; id++)
		{
			pkgs_tsp[id] = create_pkg_thread_sieve_processamento(id, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos);
			buffer_in_tsp = buffer_out_tsp;
			buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);
		}

		for (id_t id = 3; id <= qtd_de_threads_de_processamento - 2; id++)
		{
			pthread_create(&ts_sieves_processamento[id], NULL, &thread_sieve_processamento, (void*)pkgs_tsp[id]);
		}
		for (id_t id = 3; id <= qtd_de_threads_de_processamento - 2; id++)
		{
			pthread_join(ts_sieves_processamento[id], NULL);
		}
	}
	else
	{
		pkg_thread_sieve_processamento_pt pkg_tsp = create_pkg_thread_sieve_processamento(3, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos);

		pthread_create(&t_sieve_processamento, NULL, &thread_sieve_processamento, (void*)pkg_tsp);
	}

	pthread_join(t_geradora, NULL);
	pthread_join(t_resultado, NULL);

	if(!MULTIPLAS_THREADS)
	{
		pthread_join(t_sieve_processamento, NULL);
	}

	/*
		threads de processamento
	*/

	/*
	pthread_cancel(t_geradora);
	pthread_cancel(t_resultado);
	*/

	return 0;
}