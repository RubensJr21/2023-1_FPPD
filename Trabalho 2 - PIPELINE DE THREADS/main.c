// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)
#pragma warning(disable:6386)
// https://learn.microsoft.com/pt-br/cpp/code-quality/c6386?view=msvc-170

#define PDC_DLL_BUILD

#include <pthread.h>
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>

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

// https://medium.com/@selvarajk/adding-color-to-your-output-from-c-58f1a4dc4e75

#define ANSI_COLOR_RED     	 "\033[0;31m" //cores em ANSI utilizadas 
#define ANSI_COLOR_GREEN	 "\033[0;32m"
#define ANSI_COLOR_RESET     "\x1b[0m"

#define id_t int
//#define buffer_primos_pt long long*
//#define bignumber_t long long
#define bignumber_t long long
#define primo_t long long
#define round_t int

#define TRUE 1
#define FALSE 0
#define bool_t int

#define WINDOW FALSE

#define DEBUG_PRINTS FALSE
#define PRINT_MUTEXES FALSE
#define PRINT_UPDATE_ROUND FALSE
#define PRINT_NUM_GERADO FALSE
#define PRINT_SEMAFOROS FALSE
#define PRINT_LIBERAR_POSICAO FALSE
#define PRINT_BUFFER TRUE

// Estrutura para os argumentos da janela da thread
typedef struct ARGSWINDOW{
	int x;
	int y;
	int width;
	int height;
} ArgsWindow;

// Função para escrever uma linha na janela
void writeLineToWindow(HWND text_hwnd, const char* line);

void printf_(HWND text_hwnd, const char* formato, ...)
{
	va_list argumentos;
	va_start(argumentos, formato);

	// Determinar o tamanho da string resultante
	int tamanho = vsnprintf(NULL, 0, formato, argumentos);
	va_end(argumentos);

	// Alocar memória para a string resultante
	char* resultado = (char*)malloc((tamanho + 1) * sizeof(char));

	// Formatando os argumentos na string resultante
	va_start(argumentos, formato);
	vsprintf(resultado, formato, argumentos);
	va_end(argumentos);

	if (WINDOW)
	{
		writeLineToWindow(text_hwnd, resultado);
	}
	else
	{
		printf(resultado);
	}
}

typedef struct PKG_NUMBER_TO_VERIFY{
	bignumber_t number;
	round_t round;
	int contador;
	int qtd_de_threads;
	id_t id_Thread_que_resolveu;
	bool_t eh_primo;
} pkg_number_to_veriry_t, * pkg_number_to_veriry_pt, numeros_primos_t, * numeros_primos_pt;

pkg_number_to_veriry_pt create_pkg_number_to_veriry(bignumber_t number, int qtd_de_threads)
{
	pkg_number_to_veriry_pt pkg = malloc(sizeof(pkg_number_to_veriry_t));
	pkg->number = number;
	pkg->round = 0;
	pkg->contador = 0;
	pkg->qtd_de_threads = qtd_de_threads;
	pkg->id_Thread_que_resolveu = -1;
	pkg->eh_primo = FALSE;
	return pkg;
}

typedef struct BUFFER_RESULTADOS {
	int current_index;
	pkg_number_to_veriry_pt* buffer; // tamanho = X/ln(x)
	int max_size_buffer; // usado para calcular o round do number_to_verify
	pthread_mutex_t* mutex;
	pthread_cond_t* cond; // usado na inserção e no get
	sem_t* sem;
}buffer_resultados_t, * buffer_resultados_pt;

buffer_resultados_pt create_buffer_resultados(int numeros_a_serem_calculados)
{
	buffer_resultados_pt b = malloc(sizeof(buffer_resultados_t));
	// https://www.clubedohardware.com.br/forums/topic/933851-como-fazer-lnx-em-c/
	b->buffer = malloc(sizeof(pkg_number_to_veriry_pt) * numeros_a_serem_calculados - 2); // pois serão excluídos os números 0 e 1, e o processamento começará do 2
	b->current_index = 0;
	for (int index = 0; index < numeros_a_serem_calculados - 2; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = numeros_a_serem_calculados - 2;
	b->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(b->mutex, NULL);
	b->cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(b->cond, NULL);
	b->sem = malloc(sizeof(sem_t));
	sem_init(b->sem, 0, 0);
	return b;
}

bignumber_t last_number_inserted = 1;

void insert_in_buffer_resultados(buffer_resultados_pt buffer_resultados, pkg_number_to_veriry_pt ntv)
{
	pthread_mutex_lock(buffer_resultados->mutex);
	buffer_resultados->buffer[((int)ntv->number) - 2] = ntv;
	printf("Número %lld foi inserido no buffer resultados\n", ntv->number);
	if (last_number_inserted + 1 != ntv->number)
	{
		last_number_inserted++;
	}
	else
	{
		last_number_inserted = ntv->number;
	}
	sem_post(buffer_resultados->sem);
	pthread_mutex_unlock(buffer_resultados->mutex);
}

/////////////////////////////////////////////////////////////////////////////////

typedef struct BUFFER_DE_PRIMOS{
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer; // usado para calcular o round do number_to_verify
}buffer_de_primos_t, * buffer_de_primos_pt;

void insert_in_buffer_internal_primos(buffer_de_primos_pt buffer_primos, pkg_number_to_veriry_pt number_to_verify, int id, HWND text_hwnd)
{
	// envia o número para a primeira thread de processamento
	buffer_primos->buffer[number_to_verify->round] = number_to_verify;
}

primo_t get_number_from_buffer_internal_primos(buffer_de_primos_pt buffer_primos, int index, int id, HWND text_hwnd)
{
	// Caso -1
	if (index >= buffer_primos->max_size_buffer) // quer dizer que o espaço no buffer acabou, sendo assim encerrando o processo
	{
		return -2;
	}
	//printf_(text_hwnd, "get_number_from_buffer_internal_primos::buffer_primos->buffer => %p\n", buffer_primos->buffer);

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
	int current_index_writing;
	int current_index_reading;
	pkg_number_to_veriry_pt* buffer;
	int max_size_buffer;
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
	sem_t* sem_espera; // fila para usar o buffer
	sem_t* sem_mutex; // imita o mutex
}buffer_IO_t, * buffer_IO_pt;

void printf_bufferIO(HWND text_hwnd, id_t id, buffer_IO_pt buffer, int index, const char* function)
{
	char buffer_string[5000];
	int i;
	int escreveu_ate = sprintf_s(buffer_string, 5000, "%s::BUFFER THREAD #%d (pos atual: %d): ", function, id, index);
	int qtd_chars = 10 * 2;
	int tamanho_total = escreveu_ate + qtd_chars;
	for (i = 0; i < 10 - 1; i++)
	{
		if (buffer->buffer[i] != NULL)
		{
			escreveu_ate += sprintf_s(buffer_string + escreveu_ate, 5000 - escreveu_ate, "%lld,", buffer->buffer[i]->number);
		}
		else
		{
			escreveu_ate += sprintf_s(buffer_string + escreveu_ate, 5000 - escreveu_ate, "N,");
		}
	}
	if (buffer->buffer[i])
	{
		escreveu_ate += sprintf_s(buffer_string + escreveu_ate, 5000 - escreveu_ate, "%lld.\n", buffer->buffer[i]->number);
	}
	else
	{
		escreveu_ate += sprintf_s(buffer_string + escreveu_ate, 5000 - escreveu_ate, "N.\n");
	}

	printf_(text_hwnd, buffer_string);
}

buffer_IO_pt create_buffer_IO(int max_size_buffer)
{
	buffer_IO_pt b = malloc(sizeof(buffer_IO_t));
	b->current_index_writing = 0;
	b->current_index_reading = 0;
	b->buffer = malloc(sizeof(pkg_number_to_veriry_t) * max_size_buffer);
	for (int index = 0; index < max_size_buffer; index++)
	{
		b->buffer[index] = NULL;
	}
	b->max_size_buffer = max_size_buffer;
	b->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(b->mutex, NULL);

	b->cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(b->cond, NULL);

	b->sem_espera = malloc(sizeof(sem_t));
	sem_init(b->sem_espera, 0, max_size_buffer);

	b->sem_mutex = malloc(sizeof(sem_t));
	sem_init(b->sem_mutex, 0, 1);
	return b;
}

typedef struct GERADORA_TERMINOU{
	pthread_mutex_t* mutex;
	bool_t terminou;
} geradora_terminou_t, * geradora_terminou_pt;

typedef struct PKG_THREAD_GERADORA{
	id_t id;
	int numeros_a_serem_gerados;
	buffer_IO_pt buffer_out;
	int qtd_de_threads;
	sem_t* end_process;

	// PARA JANELA
	HWND hwnd;
	HWND text_hwnd;
}pkg_thread_geradora_t, * pkg_thread_geradora_pt;

pkg_thread_geradora_pt create_pkg_thread_geradora(id_t id, int size_buffer, int qtd_threads, int numeros_a_serem_gerados, sem_t* sem_end_process)
{
	buffer_IO_pt buffer = create_buffer_IO(size_buffer);
	pkg_thread_geradora_pt tg = malloc(sizeof(pkg_thread_geradora_t));
	tg->id = id;
	tg->numeros_a_serem_gerados = numeros_a_serem_gerados;
	tg->qtd_de_threads = qtd_threads;
	tg->buffer_out = buffer;
	tg->end_process = sem_end_process;
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
	while (numeros_gerados < pkg->numeros_a_serem_gerados)
	{
		ntv = create_pkg_number_to_veriry(number, qtd_de_threads);
		if (DEBUG_PRINTS && PRINT_NUM_GERADO) printf_(pkg->text_hwnd, "thread_geradora::numero gerado: %d\n", number);

		/*
		printf_(thread_geradora::mutex => %p\n", buffer->mutex);

		pthread_mutex_lock(buffer->mutex);

		pthread_mutex_unlock(buffer->mutex);
		*/
		//insert_in_buffer_IO(buffer, ntv, 1, pkg->text_hwnd);

		/// *** INSERT ***

		if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI ESPERAR o semaforo 'buffer_out->sem_espera'\n", pkg->id);
		sem_wait(buffer_out->sem_espera);
		if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d ESPEROU o semaforo 'buffer_out->sem_espera'\n", pkg->id);

		bool_t position_free = FALSE;

		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI LOCKAR\n", pkg->id);
		//pthread_mutex_lock(buffer_out->mutex);
		sem_wait(buffer_out->sem_mutex);
		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d LOCKOU\n", pkg->id);

		// envia o número para a primeira thread de processamento
		pkg_number_to_veriry_pt pos_verify = buffer_out->buffer[buffer_out->current_index_writing];
		position_free = pos_verify == NULL;
		
		// SE A POSIÇÃO ESTIVER LIVRE ESCREVE E LIBERA PARA LEITURA
		// SE NÃO ESTIVER, LIBERA E TENTA AGUARDA PARA VERIFICAR OUTRA VEZ

		if (position_free)
		{
			if (DEBUG_PRINTS && PRINT_LIBERAR_POSICAO) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI LIBERAR POSICAO %d PARA LEITURA\n", pkg->id, buffer_out->current_index_writing);

			// ESCREVE NA POSIÇÃO
			buffer_out->buffer[buffer_out->current_index_writing++] = ntv;

			if (DEBUG_PRINTS && PRINT_BUFFER) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer_out, buffer_out->current_index_writing - 1, "insert_in_buffer_IO");

			// ATUALIZA INDEX DE ESCRITA PARA PRÓXIMA TENTATIVA
			buffer_out->current_index_writing = (buffer_out->current_index_writing % buffer_out->max_size_buffer);

			// LIBERA O MUTEX PARA OUTRA THREAD
			if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI DESLOCKAR\n", pkg->id);
			//pthread_mutex_unlock(buffer_out->mutex);
			sem_post(buffer_out->sem_mutex);
			if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d DESLOCKOU\n", pkg->id);

			// LIBERA SEMAFORO
			sem_post(buffer_out->sem_espera);

			number++;
			numeros_gerados++;
		}
		else
		{
			// imprimir toda lista quando o a posição não estiver livre
			if (DEBUG_PRINTS) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer_out, buffer_out->current_index_writing, "insert_in_buffer_IO");

			// LIBERA O MUTEX PARA OUTRA THREAD
			if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI DESLOCKAR\n", pkg->id);
			//pthread_mutex_unlock(buffer_out->mutex);
			sem_post(buffer_out->sem_mutex);
			if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d DESLOCKOU\n", pkg->id);

			// LIBERA SEMAFORO
			sem_post(buffer_out->sem_espera);
			
			// RECOMECA O LOOP
		}

		/// *** FIM INSERT ***
	}
	//sem_post(pkg->end_process);
	return NULL;
}

typedef struct PHA_THREAD_SIEVE_PROCESSAMENTO{
	id_t id;
	buffer_IO_pt buffer_in; // tamanho K
	buffer_IO_pt buffer_out; // tamanho K
	buffer_resultados_pt buffer_resultados;
	int size_buffer_internal; // valor do X

	geradora_terminou_pt geradora_terminou;

	// PARA JANELA
	HWND hwnd;
	HWND text_hwnd;
}pkg_thread_sieve_processamento_t, * pkg_thread_sieve_processamento_pt;

pkg_thread_sieve_processamento_pt create_pkg_thread_sieve_processamento(id_t id, buffer_IO_pt buffer_in, buffer_IO_pt buffer_out, int buffer_size_internal, buffer_resultados_pt buffer_resultados, geradora_terminou_pt g_term)
{
	pkg_thread_sieve_processamento_pt tsp = malloc(sizeof(pkg_thread_sieve_processamento_t));
	tsp->id = id;
	tsp->buffer_in = buffer_in;
	tsp->buffer_out = buffer_out;
	tsp->buffer_resultados = buffer_resultados;
	tsp->size_buffer_internal = buffer_size_internal;
	tsp->geradora_terminou = g_term;
	return tsp;
}

void update_round(pkg_number_to_veriry_pt number_to_verify, HWND text_hwnd)
{
	if (DEBUG_PRINTS && PRINT_UPDATE_ROUND) printf_(text_hwnd, "update_round::ATUALIZA ROUND do number: %d\n", number_to_verify->number);
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
	while (TRUE)
	{
		/*
			Casos possíveis:
			-1. Buffer estourado
			0. Não ter número no vetor de primos e ele ser automaticamente primo
			1. Ser primo:
				1.1: Não foi possível dividir
					=> Passa para a próxima thread e e pega o próximo do buffer
			2. Não ser primo
				2.1: Descartar o número, pegando o próximo do buffer
		*/
		// obtém número que será verificado
		//ntv = get_number_from_buffer_IO(buffer_in, pkg->id, pkg->text_hwnd); // função assincrona


		/// *** GET ***

		if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d VAI ESPERAR o semaforo 'buffer_in->sem'\n", pkg->id);
		sem_wait(buffer_in->sem_espera);
		if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d ESPEROU o semaforo 'buffer_in->sem'\n", pkg->id);

		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d VAI LOCKAR %p\n", pkg->id, buffer_in->mutex);
		//pthread_mutex_lock(buffer_in->mutex);
		sem_wait(buffer_in->sem_mutex);
		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d LOCKOU %p\n", pkg->id, buffer_in->mutex);

		pkg_number_to_veriry_pt number = buffer_in->buffer[buffer_in->current_index_reading];
		
		// SE FOR NULO ELE ESTÁ TENTANDO LER ANTES DE ALGUÉM ESCREVER
		// LIBERA O SEMAFORO, E TENTA LER OUTRA VEZ
		
		if (number == NULL) // significa que a posição está vazia, deve esperar que a posição receba alguém
		{
			//pthread_cond_wait(buffer_in->cond, buffer_in->mutex);
			sem_post(buffer_in->sem_mutex);
		}
		else
		{
			if (DEBUG_PRINTS && PRINT_LIBERAR_POSICAO) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d VAI LIBERAR A POSICAO %d PARA ESCRITA\n", pkg->id, buffer_in->current_index_reading);
			//if(PRINT_BUFFER) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer, buffer_in->current_index_reading, "get_number_from_buffer_IO");

			// Lê número
			number = buffer_in->buffer[buffer_in->current_index_reading];

			// Libera posição para escrita
			buffer_in->buffer[buffer_in->current_index_reading++] = NULL;

			if (DEBUG_PRINTS && PRINT_BUFFER) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer_in, buffer_in->current_index_reading - 1, "get_number_from_buffer_IO");

			// atualizar index
			buffer_in->current_index_reading = (buffer_in->current_index_reading % buffer_in->max_size_buffer);
			// ver possibilidade de semaforo aqui
			sem_post(buffer_in->sem_mutex);
		}

		//pthread_cond_signal(buffer_in->cond);
		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d VAI DESLOCKAR %p\n", pkg->id, buffer_in->mutex);
		//pthread_mutex_unlock(buffer_in->mutex);
		// Libera lugar na fila de espera
		sem_post(buffer_in->sem_espera);
		if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "get_number_from_buffer_IO::THREAD #%d DESLOCKOU %p\n", pkg->id, buffer_in->mutex);
		ntv = number;

		/// FIM DO GET

		// PROCESSAR()

		if(ntv != NULL)
		{
			// obtém número primo que tentará dividir o ntv
			primo_t numero_do_vetor_de_primos = get_number_from_buffer_internal_primos(buffer_primos, ntv->round, pkg->id, pkg->text_hwnd);
			// verifica se existe algum número no buffer de primos na posição ntv->round
			// numero_do_vetor_de_primos == -1; não existe número naquela posição
			// numero_do_vetor_de_primos != -1; existe número naquela posição
			// Caso -1
			// buffer não tem mais espaço, encerra o processamento de todas as threads
			if (numero_do_vetor_de_primos == -2)
			{
				if (DEBUG_PRINTS) printf_(pkg->text_hwnd, ANSI_COLOR_RED "process_number::***NAO TEM MAIS ESPACO PARA GUARDAR PRIMOS***" ANSI_COLOR_RESET);
				// aqui deve parar de rodar o código
				system("pause");
			}
			// Caso 0
			if (numero_do_vetor_de_primos == -1)
			{
				// insere número
				ntv->id_Thread_que_resolveu = pkg->id;
				ntv->eh_primo = TRUE;
				insert_in_buffer_internal_primos(buffer_primos, ntv, pkg->id, pkg->text_hwnd);
				if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "process_number::numero %d foi verificado como PRIMO na Thread #%d e ROUND #%d\n", ntv->number, pkg->id, ntv->round);
				// enviar para thread resultado
				insert_in_buffer_resultados(pkg->buffer_resultados, ntv);
			}
			else
			{
				// verificar se o ntv é divisível pelo numero_do_vetor_de_primos
				bignumber_t resultado = ntv->number % numero_do_vetor_de_primos;
				// resultado == 0, é divisível. Não é primo, descarta
				// resultado != 0, não é divisível
				// Caso 1
				if (resultado != 0)
				{
					// incrementa ntv->contador e atualiza o ntv->round
					update_round(ntv, pkg->text_hwnd);
					// passa para a próxima thread => escrver no buffer_out
					//insert_in_buffer_IO(buffer_out, ntv, pkg->id, pkg->text_hwnd);

					/// *** INSERT ***

					if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI ESPERAR o semaforo 'buffer_out->sem_espera'\n", pkg->id);
					sem_wait(buffer_out->sem_espera);
					if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d ESPEROU o semaforo 'buffer_out->sem_espera'\n", pkg->id);

					bool_t position_free = FALSE;

					if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI LOCKAR\n", pkg->id);
					//pthread_mutex_lock(buffer_out->mutex);
					sem_wait(buffer_out->sem_mutex);
					if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d LOCKOU\n", pkg->id);

					// envia o número para a primeira thread de processamento
					pkg_number_to_veriry_pt pos_verify = buffer_out->buffer[buffer_out->current_index_writing];
					position_free = pos_verify == NULL;

					// SE A POSIÇÃO ESTIVER LIVRE ESCREVE E LIBERA PARA LEITURA
					// SE NÃO ESTIVER, LIBERA E TENTA AGUARDA PARA VERIFICAR OUTRA VEZ

					if (position_free)
					{
						if (DEBUG_PRINTS && PRINT_LIBERAR_POSICAO) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI LIBERAR POSICAO %d PARA LEITURA\n", pkg->id, buffer_out->current_index_writing);

						// ESCREVE NA POSIÇÃO
						buffer_out->buffer[buffer_out->current_index_writing++] = ntv;

						if (DEBUG_PRINTS && PRINT_BUFFER) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer_out, buffer_out->current_index_writing - 1, "insert_in_buffer_IO");

						// ATUALIZA INDEX DE ESCRITA PARA PRÓXIMA TENTATIVA
						buffer_out->current_index_writing = (buffer_out->current_index_writing % buffer_out->max_size_buffer);

						// LIBERA O MUTEX PARA OUTRA THREAD
						if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI DESLOCKAR\n", pkg->id);
						//pthread_mutex_unlock(buffer_out->mutex);
						sem_post(buffer_out->sem_mutex);
						if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d DESLOCKOU\n", pkg->id);

						// LIBERA SEMAFORO
						sem_post(buffer_out->sem_espera);
					}
					else
					{
						// imprimir toda lista quando o a posição não estiver livre
						if (DEBUG_PRINTS) printf_bufferIO(pkg->text_hwnd, pkg->id, buffer_out, buffer_out->current_index_writing, "insert_in_buffer_IO");

						// LIBERA O MUTEX PARA OUTRA THREAD
						if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d VAI DESLOCKAR\n", pkg->id);
						//pthread_mutex_unlock(buffer_out->mutex);
						sem_post(buffer_out->sem_mutex);
						if (DEBUG_PRINTS && PRINT_MUTEXES) printf_(pkg->text_hwnd, "insert_in_buffer_IO::THREAD #%d DESLOCKOU\n", pkg->id);

						// LIBERA SEMAFORO
						sem_post(buffer_out->sem_espera);

						// RECOMECA O LOOP
					}

					/// *** FIM INSERT ***
				}
				else
				{
					ntv->id_Thread_que_resolveu = pkg->id;
					ntv->eh_primo = FALSE;
					if (DEBUG_PRINTS) printf_(pkg->text_hwnd, "process_number::numero %d foi verificado como NAO PRIMO na Thread #%d e ROUND #%d\n", ntv->number, pkg->id, ntv->round);
					insert_in_buffer_resultados(pkg->buffer_resultados, ntv);
				}
			}
			// FIM PROCESSAR
		}
	}
	return NULL;
}

typedef struct PKG_THREAD_RESULTADO{
	id_t id;
	buffer_resultados_pt buffer_resultados;

	// PARA JANELA
	HWND hwnd;
	HWND text_hwnd;
}pkg_thread_resultado_t, * pkg_thread_resultado_pt;

pkg_thread_resultado_pt create_pkg_thread_resultado(id_t id, int size_buffer_resultados)
{
	pkg_thread_resultado_pt ts = malloc(sizeof(pkg_thread_resultado_t));
	ts->id = id;
	ts->buffer_resultados = create_buffer_resultados(size_buffer_resultados);
	return ts;
}

void printf_buffer_resultados(HWND text_hwnd, buffer_resultados_pt buffer)
{
	#define SIZE_BUFFER_STRING 10000
	char buffer_string[SIZE_BUFFER_STRING];
	int i;
	int escreveu_ate = sprintf_s(buffer_string, SIZE_BUFFER_STRING, "thread_resultado::BUFFER RESULTADOS: ");
	int qtd_chars = buffer->max_size_buffer * 2;
	int tamanho_total = escreveu_ate + qtd_chars;
	int primeiro_index_com_NULL = -1;
	for (i = 0; i < buffer->max_size_buffer - 1; i++)
	{
		if (buffer->buffer[i] != NULL)
		{
			escreveu_ate += sprintf_s(buffer_string + escreveu_ate, SIZE_BUFFER_STRING - escreveu_ate, "%lld,", buffer->buffer[i]->number);
		}
		else
		{
			if (primeiro_index_com_NULL == -1) primeiro_index_com_NULL = i;
		}
	}
	escreveu_ate += sprintf_s(buffer_string + escreveu_ate, SIZE_BUFFER_STRING - escreveu_ate, " => FIM VETOR\n");

	printf("\n\nprimeiro_index_com_NULL: %d.\n\n", primeiro_index_com_NULL);

	printf_(text_hwnd, buffer_string);
}

void* thread_resultado(void* args)
{
	printf("Entrou na Thread de Resultados\n");
	pkg_thread_resultado_pt pkg = (pkg_thread_resultado_pt) args;
	buffer_resultados_pt buffer_resultados = pkg->buffer_resultados;
	pkg_number_to_veriry_pt ntv;
	for (int i = 0; i < buffer_resultados->max_size_buffer; i++)
	{
		pthread_mutex_lock(buffer_resultados->mutex);
		ntv = buffer_resultados->buffer[i];
		pthread_mutex_unlock(buffer_resultados->mutex);
		while(ntv == NULL)
		{
			sem_wait(buffer_resultados->sem);
			//printf_buffer_resultados(pkg->text_hwnd, buffer_resultados);
			pthread_mutex_lock(buffer_resultados->mutex);
			ntv = buffer_resultados->buffer[i];
			pthread_mutex_unlock(buffer_resultados->mutex);

		}
		//printf("\n");
		if (ntv->eh_primo)
		{
			//printf_(pkg->text_hwnd, "thread_resultado::numero %d foi verificado como PRIMO na Thread #%d e ROUND #%d\n", ntv->number, ntv->id_Thread_que_resolveu, ntv->round);
			printf("thread_resultado::numero %lld foi verificado como PRIMO na Thread #%d e ROUND #%d\n", ntv->number, ntv->id_Thread_que_resolveu, ntv->round);
		}
		else
		{
			//printf_(pkg->text_hwnd, "thread_resultado::numero %d foi verificado como NAO PRIMO na Thread #%d e ROUND #%d\n", ntv->number, ntv->id_Thread_que_resolveu, ntv->round);
			printf("thread_resultado::numero %lld foi verificado como NAO PRIMO na Thread #%d e ROUND #%d\n", ntv->number, ntv->id_Thread_que_resolveu, ntv->round);
		}
	}
	return NULL;
}

// FUNÇÕES PARA JANELAS

// Procedimento de janela para as janelas das threads
LRESULT CALLBACK ThreadWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Função para criar e personalizar uma janela da thread
void createThreadGeradoraWindow(pkg_thread_geradora_pt thread_args, ArgsWindow* window_args);

// Função para criar e personalizar uma janela da thread
void createThreadResultadosWindow(pkg_thread_resultado_pt thread_args, ArgsWindow* window_args);

// Função para criar e personalizar uma janela da thread
void createThreadSieveProcessamentoWindow(pkg_thread_sieve_processamento_pt thread_args, ArgsWindow* window_args);


// Função para configurar os argumentos da janela
void configure_args_window(ArgsWindow* window_args, int x, int y, int width, int height);

int main(int argc, char* argv[])
{

	pthread_t t_geradora, t_resultado;

	pthread_t* ts_sieves_processamento;
	pkg_thread_sieve_processamento_pt* pkgs_tsp;

	sem_t sem_end_process;
	sem_init(&sem_end_process, 0, 0);

	/*
		N: quantidade de primos que a devem ter na thread RESULTADO
		M : quantidade de threads de processamento
		K : tamanho dos buffers de comunicação
		X : tamanho do buffer de primos das threads de processamento
		buffer interno(que guarda os numeros primos)
	*/
	// N
	int numero_a_serem_testados = 100000;
	// M
	int qtd_de_threads_de_processamento = 2;
	// K
	int max_size_comunication_buffer = 10;
	// X
	int max_size_internal_buffer = 50;

	// PARA JANELAS

	// Obter as dimensões da tela
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// Calcular as dimensões dos espaços das threads
	int thread_width = screen_width / 2;
	int thread_height = screen_height / 2;

	// FIM PARA JANELAS

	pkg_thread_geradora_pt pkg_tg = create_pkg_thread_geradora(0, max_size_comunication_buffer, qtd_de_threads_de_processamento, numero_a_serem_testados, &sem_end_process);
	buffer_IO_pt buffer_out_geradora = pkg_tg->buffer_out;

	pkg_thread_resultado_pt pkg_tr = create_pkg_thread_resultado(1, numero_a_serem_testados);
	buffer_resultados_pt buffer_resultados_numeros_primos = pkg_tr->buffer_resultados;

	// PARA JANELAS

	ArgsWindow argsWindow_tg, argsWindow_tr;
	ArgsWindow argsWindow_tsp1, argsWindow_tsp2;

	// Configurar os argumentos das janelas das threads
	configure_args_window(&argsWindow_tg, 0, 0, thread_width, thread_height);
	configure_args_window(&argsWindow_tr, thread_width, 0, thread_width, thread_height);

	createThreadGeradoraWindow(pkg_tg, &argsWindow_tg);
	createThreadResultadosWindow(pkg_tr, &argsWindow_tr);

	// FIM PARA JANELAS

	pthread_create(&t_geradora, NULL, &thread_geradora, (void*)pkg_tg);
	pthread_create(&t_resultado, NULL, &thread_resultado, (void*)pkg_tr);

	buffer_IO_pt buffer_in_tsp = buffer_out_geradora;
	buffer_IO_pt buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);

	ts_sieves_processamento = malloc(sizeof(pthread_t) * qtd_de_threads_de_processamento);
	pkgs_tsp = malloc(sizeof(pkg_thread_sieve_processamento_pt) * qtd_de_threads_de_processamento);

	id_t id;

	geradora_terminou_pt g_term = malloc(sizeof(geradora_terminou_t));
	g_term->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(g_term->mutex, NULL);
	g_term->terminou = FALSE;

	for (id = 0; id < qtd_de_threads_de_processamento - 1; id++) // id <= total_threads - 2 para parar na última e ser criada manualmente
	{
		pkgs_tsp[id] = create_pkg_thread_sieve_processamento(id + 2, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos, g_term);

		// PARA JANELAS
		configure_args_window(&argsWindow_tsp1, 0, thread_height, thread_width, thread_height);
		createThreadSieveProcessamentoWindow(pkgs_tsp[id], &argsWindow_tsp1);
		// FIM DO PARA JANELAS

		buffer_in_tsp = buffer_out_tsp;
		buffer_out_tsp = create_buffer_IO(max_size_comunication_buffer);
	}

	buffer_out_tsp = buffer_out_geradora;
	pkgs_tsp[id] = create_pkg_thread_sieve_processamento(id + 2, buffer_in_tsp, buffer_out_tsp, max_size_internal_buffer, buffer_resultados_numeros_primos, g_term);

	// PARA JANELAS
	configure_args_window(&argsWindow_tsp2, thread_width, thread_height, thread_width, thread_height);
	createThreadSieveProcessamentoWindow(pkgs_tsp[id], &argsWindow_tsp2);
	// FIM DO PARA JANELAS

	for (id = 0; id < qtd_de_threads_de_processamento; id++)
	{
		pthread_create(&ts_sieves_processamento[id], NULL, &thread_sieve_processamento, (void*)pkgs_tsp[id]);
	}

	if (WINDOW)
	{
		// PARA JANELAS
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// FIM DO PARA JANELAS
	}

	// espera receber um sinal de que pode parar o processo
	sem_wait(&sem_end_process);
	pthread_mutex_lock(g_term->mutex);
	g_term->terminou = TRUE;
	pthread_mutex_unlock(g_term->mutex);
	printf(ANSI_COLOR_RED "Esperou o semaforo\n" ANSI_COLOR_RESET);

	for (id = 0; id < qtd_de_threads_de_processamento; id++)
	{
		pthread_cancel(ts_sieves_processamento[id]);
	}

	pthread_cancel(t_resultado);

	return 0;
}

// FUNÇÕES PARA JANELAS

// Procedimento de janela para as janelas das threads
LRESULT CALLBACK ThreadWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// Função para criar e personalizar uma janela da thread
void createThreadGeradoraWindow(pkg_thread_geradora_pt thread_args, ArgsWindow* window_args)
{
	// Criar janela para a thread
	wchar_t window_title[20];
	swprintf_s(window_title, sizeof(window_title) / sizeof(window_title[0]), L"Thread %d", thread_args->id);

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = ThreadWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = window_title;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // Fundo preto
	RegisterClass(&wc);

	if (WINDOW)
	{
		// Criar a janela da thread
		thread_args->hwnd = CreateWindowExW(0, wc.lpszClassName, window_title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			window_args->x, window_args->y, window_args->width, window_args->height,
			NULL, NULL, wc.hInstance, NULL);

		// Criar controle de edição de texto
		thread_args->text_hwnd = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, window_args->width, window_args->height,
			thread_args->hwnd, NULL, wc.hInstance, NULL);

		// Adicionar barra de rolagem ao controle de texto
		SetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE,
			GetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE) | WS_VSCROLL);

		// Mostrar a janela
		ShowWindow(thread_args->hwnd, SW_SHOW);

		// Ajustar o tamanho do controle de texto para incluir a barra de rolagem
		RECT client_rect;
		GetClientRect(thread_args->hwnd, &client_rect);
		int text_width = client_rect.right - client_rect.left;
		int text_height = client_rect.bottom - client_rect.top;
		int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
		SetWindowPos(thread_args->text_hwnd, NULL, 0, 0, text_width - scrollbar_width, text_height, SWP_NOMOVE | SWP_NOZORDER);
	}
}

// Função para criar e personalizar uma janela da thread
void createThreadResultadosWindow(pkg_thread_resultado_pt thread_args, ArgsWindow* window_args)
{
	// Criar janela para a thread
	wchar_t window_title[20];
	swprintf_s(window_title, sizeof(window_title) / sizeof(window_title[0]), L"Thread %d", thread_args->id);

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = ThreadWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = window_title;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // Fundo preto
	RegisterClass(&wc);

	if (WINDOW)
	{
		// Criar a janela da thread
		thread_args->hwnd = CreateWindowExW(0, wc.lpszClassName, window_title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			window_args->x, window_args->y, window_args->width, window_args->height,
			NULL, NULL, wc.hInstance, NULL);

		// Criar controle de edição de texto
		thread_args->text_hwnd = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, window_args->width, window_args->height,
			thread_args->hwnd, NULL, wc.hInstance, NULL);

		// Adicionar barra de rolagem ao controle de texto
		SetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE,
			GetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE) | WS_VSCROLL);

		// Mostrar a janela
		ShowWindow(thread_args->hwnd, SW_SHOW);

		// Ajustar o tamanho do controle de texto para incluir a barra de rolagem
		RECT client_rect;
		GetClientRect(thread_args->hwnd, &client_rect);
		int text_width = client_rect.right - client_rect.left;
		int text_height = client_rect.bottom - client_rect.top;
		int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
		SetWindowPos(thread_args->text_hwnd, NULL, 0, 0, text_width - scrollbar_width, text_height, SWP_NOMOVE | SWP_NOZORDER);
	}

}

// Função para criar e personalizar uma janela da thread
void createThreadSieveProcessamentoWindow(pkg_thread_sieve_processamento_pt thread_args, ArgsWindow* window_args)
{
	// Criar janela para a thread
	wchar_t window_title[20];
	swprintf_s(window_title, sizeof(window_title) / sizeof(window_title[0]), L"Thread %d", thread_args->id);

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = ThreadWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = window_title;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // Fundo preto
	RegisterClass(&wc);

	if (WINDOW)
	{
		// Criar a janela da thread
		thread_args->hwnd = CreateWindowExW(0, wc.lpszClassName, window_title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			window_args->x, window_args->y, window_args->width, window_args->height,
			NULL, NULL, wc.hInstance, NULL);

		// Criar controle de edição de texto
		thread_args->text_hwnd = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, window_args->width, window_args->height,
			thread_args->hwnd, NULL, wc.hInstance, NULL);

		// Adicionar barra de rolagem ao controle de texto
		SetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE,
			GetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE) | WS_VSCROLL);

		// Mostrar a janela
		ShowWindow(thread_args->hwnd, SW_SHOW);

		// Ajustar o tamanho do controle de texto para incluir a barra de rolagem
		RECT client_rect;
		GetClientRect(thread_args->hwnd, &client_rect);
		int text_width = client_rect.right - client_rect.left;
		int text_height = client_rect.bottom - client_rect.top;
		int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
		SetWindowPos(thread_args->text_hwnd, NULL, 0, 0, text_width - scrollbar_width, text_height, SWP_NOMOVE | SWP_NOZORDER);
	}
}

// Função para escrever uma linha na janela com limite de linhas
void writeLineToWindow(HWND text_hwnd, const char* line)
{
	// Converter linha para LPCWSTR
	wchar_t wline[256];
	swprintf_s(wline, sizeof(wline) / sizeof(wline[0]), L"%hs\n", line);

	// Verificar o número de linhas atualmente exibidas
	int line_count = (int)SendMessageW(text_hwnd, EM_GETLINECOUNT, 0, 0);

	// Se exceder o limite, remover a linha mais antiga
	if (line_count >= 100)
	{
		SendMessageW(text_hwnd, EM_SETSEL, 0, SendMessageW(text_hwnd, EM_LINEINDEX, 1, 0));
		SendMessageW(text_hwnd, EM_REPLACESEL, FALSE, (LPARAM)L"");
	}

	// Inserir a nova linha na janela
	int length = GetWindowTextLengthW(text_hwnd);
	SendMessageW(text_hwnd, EM_SETSEL, length, length);
	SendMessageW(text_hwnd, EM_REPLACESEL, FALSE, (LPARAM)wline);

	// Rolar a janela para exibir a última linha
	SendMessageW(text_hwnd, EM_LINESCROLL, 0, 1);
}

// Função para configurar os argumentos da janela
void configure_args_window(ArgsWindow* window_args, int x, int y, int width, int height)
{
	window_args->x = x;
	window_args->y = y;
	window_args->width = width;
	window_args->height = height;
}