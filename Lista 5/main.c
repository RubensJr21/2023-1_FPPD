// C�digo necess�rio para o Visual Studio n�o acusar fun��es inseguras
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
	Condi��es:
		-> precisa ter 20 mensagens para enviar
	Estados:
		-> o pombo mora (come�a) no site_A
		-> o pombo dorme at� que tenha o n�mero de mensagens (20) para enviar
		-> quando voltar ele dorme at� ter o n�mero de mensagens (20) para enviar
	Regras:
		-> o pombo s� pode levar exatamente 20 mensagens por vez
		-> caso o pombo tenha partido o usu�rio deve esperer que o pombo volte para colar na mochila do pombo
	Funcionamento:
		-> as mensagens s�o escritas em um post-it pelos usu�rios
		-> cada usu�rio, quando tem uma mensagem pronta, cola sua mensagem na mochila do pombo
		-> o vig�simo usu�rio deve acordar o pombo caso ele esteja dormindo
		-> ENTREGA DAS MENSAGENS: o pombo quando chegar no site_B substitui a quantidade de mensagem de 20 para 0
	OBS:
		-> Cada usu�rio tem seu bloquinho inesgot�vel de post-it e continuamente prepara uma mensagem e a leva ao pombo
		-> Usando sem�foros, modele o processo pombo e o processo usu�rio, lembrando que existem muitos usu�rios e apenas um pombo.
		-> Identifique regi�es cr�ticas na vida do usu�rio e do pombo destacando atrav�s de coment�rios nessas regi�es do seu c�digo.
			-> nos casos de concorr�ncia, tipo com mutex, sem�foro
*/

#include <stdio.h>

typedef int *mensagens_t;
typedef int id_t;

typedef struct{
	mensagens_t mensagens; // vari�vel compartilhada entre os sites A e B
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