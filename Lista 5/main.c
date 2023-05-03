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

typedef struct{
	int* mensagens; // vari�vel compartilhada entre os sites A e B
}pombo_t, *pombo_pt;


typedef struct site_t{
	int id;
	pombo_pt pombo
}site_A_t, site_B_t, *site_A_pt, *site_B_pt;

typedef struct {
	int id;
	pombo_pt pombo;
}pkg_site_A_t, pkg_site_B_t, *pkg_site_A_pt, *pkg_site_B_pt;

struct site* create_site(int id, pombo_pt p)
{
	struct site* s = NULL;
	return s;
}

site_A_pt create_site_A(int id, pombo_pt p)
{
	return create_site(id, p);
}

site_B_pt create_site_B(int id, pombo_pt p)
{
	return create_site(id, p);
}

int main()
{
	return 0;
}