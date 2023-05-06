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
		-> o pombo mora (começa) no site_A
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

typedef int mensagens_t, *mensagens_pt;
typedef int id_t;

typedef struct pombo_t{
	mensagens_pt mensagens; // variável compartilhada entre os sites A e B
	int limite;
}pombo_t, *pombo_pt;

pombo_pt create_pombo(int limite_mensagens, mensagens_pt m)
{
	pombo_pt p = malloc(sizeof(pombo_pt));
	p->limite = limite_mensagens;
	p->mensagens = m;
	return NULL;
}

typedef struct pkg_site_t{
	id_t id;
	pombo_pt pombo;
}pkg_site_t, *pkg_site_pt, pkg_site_A_t, pkg_site_B_t, *pkg_site_A_pt, *pkg_site_B_pt;

pkg_site_pt create_pkg_site(id_t id, pombo_pt p)
{
	pkg_site_pt site = malloc(sizeof(pkg_site_t));
	site->id = id;
	site->pombo = p;
	return site;
}

pkg_site_A_pt create_pkg_site_A(id_t id, pombo_pt p)
{
	return create_pkg_site(id, p);
}

void *site_A(void *args)
{
	printf("Thread A foi criada\n");
	printf("Thread A será encerrada\n");
	return NULL;
}

pkg_site_B_pt create_pkg_site_B(id_t id, pombo_pt p)
{
	return create_pkg_site(id, p);
}

void* site_B(void* args)
{
	printf("Thread B foi criada\n");
	printf("Thread B será encerrada\n");
	return NULL;
}

int main()
{
	mensagens_t mensagens = 0;
	pombo_pt pombo = create_pombo(20, &mensagens);

	id_t id_A = 0, id_B = 1;

	pkg_site_A_pt pkg_A = create_pkg_site_A(id_A, pombo);
	pkg_site_B_pt pkg_B = create_pkg_site_B(id_B, pombo);

	pthread_t tsite_A, tsite_B;

	pthread_create(&tsite_A, NULL, &site_A, (void*)pkg_A);
	pthread_create(&tsite_B, NULL, &site_B, (void*)pkg_B);

	pthread_join(tsite_A, NULL);
	printf("Thread A foi encerrada\n");
	pthread_join(tsite_B, NULL);
	printf("Thread B foi encerrada\n");

	return 0;
}