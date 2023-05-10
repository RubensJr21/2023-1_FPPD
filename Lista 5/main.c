// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma warning(disable:6011)

/*
	Structs:
		-> site_A; OK
		-> site_B; OK
		-> pombo; OK
		-> mensagens; OK
		-> post-it; +- OK
		-> mochila do pombo; OK
	Condições:
		-> precisa ter 20 mensagens para enviar
	Estados:
		-> o pombo "mora"/"começa" no site_A
		-> o pombo dorme até que tenha o número de mensagens (20) para enviar
		-> quando voltar ele dorme até ter o número de mensagens (20) para enviar
	Regras:
		-> o pombo só pode levar exatamente 20 mensagens por vez
		-> caso o pombo tenha partido o usuário deve esperer que o pombo volte para colar na mochila do pombo
	Funcionamento:
		-> as mensagens são escritas em um post-it pelos usuários
		-> cada usuário, quando tem uma mensagem pronta, cola sua mensagem na mochila do pombo
		-> o vigésimo usuário deve acordar o pombo caso ele esteja dormindo
		-> ENTREGA DAS MENSAGENS: o pombo quando chegar no site_B substitui a quantidade de mensagem de 20 para 0
	OBS:
		-> Cada usuário tem seu bloquinho inesgotável de post-it e continuamente prepara uma mensagem e a leva ao pombo
		-> Usando semáforos, modele o processo pombo e o processo usuário, lembrando que existem muitos usuários e apenas um pombo.
		-> Identifique regiões críticas na vida do usuário e do pombo destacando através de comentários nessas regiões do seu código.
			-> nos casos de concorrência, tipo com mutex, semáforo
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define TRUE 1
#define FALSE 0

typedef int mensagens_t, *mensagens_pt;
typedef int id_t;

typedef struct pombo{
	mensagens_pt mensagens; // variável compartilhada entre os sites A e B
	int limite;
	sem_t* sem_pode_escrever;
	sem_t* sem_pode_entregar;
	pthread_mutex_t* mutex;
}pombo_t, *pombo_pt;

pombo_pt create_pombo(int limite_mensagens, mensagens_pt m, sem_t* s_pode_escrever, sem_t* s_pode_entregar, pthread_mutex_t* mutex)
{
	pombo_pt p = malloc(sizeof(pombo_t));
	p->limite = limite_mensagens;
	p->mensagens = m;
	p->sem_pode_escrever = s_pode_escrever;
	p->sem_pode_entregar = s_pode_entregar;
	p->mutex = mutex;
	return p;
}

typedef struct{
	id_t id;
	pombo_pt pombo;
}pkg_usuario_t, *pkg_usuario_pt;

pkg_usuario_pt create_pkg_usuario(id_t id, pombo_pt p)
{
	pkg_usuario_pt site = malloc(sizeof(pkg_usuario_t));
	site->id = id;
	site->pombo = p;
	return site;
}

void *t_pombo(void* args)
{
	pombo_pt pombo = (pombo_pt) args;
	mensagens_pt p_mensagens = pombo->mensagens;
	while (TRUE)
	{
		// Nessa parte a thread pombo espera até que ele poxa executar o fluxo de "entrega"
		sem_wait(pombo->sem_pode_entregar);
		// Nesse ponto é garantido o acesso exclusivo à área de memória que é compartilhada (pombo->mensagens)
		pthread_mutex_lock(pombo->mutex);
		for (; (*p_mensagens) > 0; (*p_mensagens)--)
		{
			printf("pombo entregando mensagem de numero: %d\n", (pombo->limite - (*p_mensagens)) + 1);
			// Esse semaforo é usado para sinalizar para os usuários que eles podem voltar a escrever
			sem_post(pombo->sem_pode_escrever);
		}
		// Nesse ponto é liberado o acesso da área de memória que é compartilhada (pombo->mensagens) para as outras threads (usuário)
		pthread_mutex_unlock(pombo->mutex);
	}
}

void* t_usuario(void* args)
{
	pkg_usuario_pt usuario = (pkg_usuario_pt)args;
	pombo_pt pombo = usuario->pombo;
	while (TRUE)
	{
		// Na primeira iteração existem 20 vagas a serem ocupadas, todas as threads tentam, mas apenas 20 threads usuários conseguem o acesso
		sem_wait(pombo->sem_pode_escrever);
		// Nesse ponto é garantido o acesso exclusivo à área de memória que é compartilhada (pombo->mensagens)
		pthread_mutex_lock(pombo->mutex);
		(*(pombo->mensagens))++;
		printf("usuario %d escrevendo mensagem de numero %d no pombo\n", usuario->id, (*(pombo->mensagens)));
		if (pombo->limite == (*(pombo->mensagens)))
		{
			// Esse semaforo informa à thread pombo que ele já pode comerçar o fluxo de "entrega" das mensagens
			sem_post(pombo->sem_pode_entregar);
		}
		// Nesse ponto é liberado o acesso da área de memória que é compartilhada (pombo->mensagens) para as outras threads (usuário)
		pthread_mutex_unlock(pombo->mutex);
	}
}

#define LIMITE_MENSAGENS 20
#define QTD_USUARIOS 50

int main()
{
	sem_t sem_pode_escrever;
	sem_init(&sem_pode_escrever, 0, LIMITE_MENSAGENS);
	sem_t sem_pode_entregar;
	sem_init(&sem_pode_entregar, 0, 0);
	pthread_mutex_t mutex_pombo;
	pthread_mutex_init(&mutex_pombo, NULL);
	mensagens_t mensagens = 0;
	
	pombo_pt pombo = create_pombo(LIMITE_MENSAGENS, &mensagens, &sem_pode_escrever, &sem_pode_entregar, &mutex_pombo);

	pthread_t thread_pombo;
	pthread_create(&thread_pombo, NULL, &t_pombo, (void*)pombo);

	pthread_t* usuarios = malloc(sizeof(pthread_t) * QTD_USUARIOS);
	pkg_usuario_t** pkgs_usuarios = malloc(sizeof(pkg_usuario_t *) * QTD_USUARIOS);;
	int i;
	for (i = 0; i < QTD_USUARIOS; i++)
	{
		pkgs_usuarios[i] = create_pkg_usuario(i + 1, pombo);
	}

	for (i = 0; i < QTD_USUARIOS; i++)
	{
		pthread_create(&usuarios[i], NULL, &t_usuario, (void*)pkgs_usuarios[i]);
		
	}

	pthread_join(thread_pombo, NULL);
	for (i = 0; i < QTD_USUARIOS; i++)
	{
		pthread_join(usuarios[i], NULL);
	}
	return 0;
}