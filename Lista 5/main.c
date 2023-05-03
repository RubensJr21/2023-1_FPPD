// Código necessário para o Visual Studio não acusar funções inseguras
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

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

#include <stdio.h>

typedef int *mensagens_t;
typedef int id_t;

typedef struct{
	mensagens_t mensagens; // variável compartilhada entre os sites A e B
	int limite;
}pombo_t, *pombo_pt;

pombo_pt create_pombo(int limite_mensagens, mensagens_t m)
{
	pombo_pt p = malloc(sizeof(pombo_pt));
	p->limite = limite_mensagens;
	p->mensagens = m;
	return NULL;
}

typedef struct site_t{
	id_t id;
	pombo_pt pombo
}site_t, *site_pt, site_A_t, site_B_t, *site_A_pt, *site_B_pt;

typedef struct {
	id_t id;
	pombo_pt pombo;
}pkg_site_A_t, pkg_site_B_t, *pkg_site_A_pt, *pkg_site_B_pt;

site_pt create_site(id_t id, pombo_pt p)
{
	site_pt site = malloc(sizeof(site_t));
	site->id = id;
	site->pombo = p;
	return site;
}

site_A_pt create_site_A(id_t id, pombo_pt p)
{
	return create_site(id, p);
}

site_B_pt create_site_B(id_t id, pombo_pt p)
{
	return create_site(id, p);
}

int main()
{
	return 0;
}