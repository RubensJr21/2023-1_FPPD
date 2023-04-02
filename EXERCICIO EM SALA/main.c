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

typedef struct {
    int id;
    unsigned long retorno;
    unsigned long* recurso_compartilhado;
    pthread_mutex_t* mutex;
} pkg_pthread_starvation;

void pkg_pthread_starvation_init(pkg_pthread_starvation* pkg, int id, unsigned long* recurso_comp, pthread_mutex_t* mutex) {
    pkg->id = id;
    pkg->retorno = 0;
    pkg->recurso_compartilhado = recurso_comp;
    pkg->mutex = mutex;
}

void* produtor_Starvation(void* argv) {
    pkg_pthread_starvation* pkg = (pkg_pthread_starvation*)argv;
    unsigned long long contador = 0;
    while (1) {
        //printf("Execucao n-%lld do produtor => Endereco mutex: %x\n", ++contador, pkg->mutex);
        pthread_mutex_lock(pkg->mutex);
        unsigned long ultimo_valor_recurso = ++(*(pkg->recurso_compartilhado));
        pthread_mutex_unlock(pkg->mutex);
        //printf("Depois da producao valor de rescurso_compartilhado: %ld\n", ultimo_valor_recurso);
    }
    return NULL;
}

void* consumidor_Starvation(void* argv) {
    pkg_pthread_starvation* pkg = (pkg_pthread_starvation*)argv;
    unsigned long long contador = 0;
    while (1) {
        //printf("consumidor #%d :: execucao n-%lld => Endereco mutex: %x\n", ++contador, pkg->id, pkg->mutex);
        pthread_mutex_lock(pkg->mutex);
        unsigned long ultimo_valor_recurso = --(*(pkg->recurso_compartilhado));
        pkg->retorno++;
        pthread_mutex_unlock(pkg->mutex);
        //printf("ultimo valor de rescurso_compartilhado: %ld\n", ultimo_valor_recurso);
    }
    return argv;
}
void* with_starvation(void* argv) {
    int* qtd_consumidores = (int*)argv;
    printf("qtd=%d\n", *qtd_consumidores);
    unsigned long recurso_compartilhado = 0;
    // mutex
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    printf("mutex\n");

    // produtor
    pthread_t* produtor = (pthread_t*)malloc(sizeof(pthread_t));
    pkg_pthread_starvation* pkg_produtor = (pkg_pthread_starvation*)malloc(sizeof(pkg_pthread_starvation));
    //printf("produtor=>malloc\n");
    pkg_pthread_starvation_init(pkg_produtor, -1, &recurso_compartilhado, &mutex);

    pthread_create(produtor, NULL, &produtor_Starvation, pkg_produtor);

    // consumidores
    //pthread_t consumidores[qtd_consumidores];
    pkg_pthread_starvation* pkg = (pkg_pthread_starvation*)malloc(sizeof(pkg_pthread_starvation) * (*qtd_consumidores));
    if (pkg == NULL) {
        return NULL;
    }
    pthread_t* consumidores = (pthread_t*)malloc(sizeof(pthread_t) * (*qtd_consumidores));
    if (consumidores == NULL) {
        return NULL;
    }
    for (int i = 0; i < (*qtd_consumidores); i++) {
        pkg_pthread_starvation_init(&pkg[i], i, &recurso_compartilhado, &mutex);
        //printf("endereco do consumidor #%d: %x\n", i, &(consumidores[i]));
        pthread_create(&consumidores[i], NULL, &consumidor_Starvation, (void*)&pkg[i]);
    }

    for (int i = 0; i < (*qtd_consumidores); i++)
    {
        pkg_pthread_starvation* pkg = NULL;
        pthread_join(consumidores[i], (void*)&pkg);
        printf("consumidor #%d produziu: %ld\n", pkg->id, pkg->retorno);
    }
    return NULL;
}

// 1 produtor e N consumidores
int main() {
    pthread_t* thread = NULL;
    int qtd_consumidores = 4;
    pthread_create(thread, NULL, &with_starvation, (void*)&qtd_consumidores);
    printf("Sleep(5000)...\n");
    Sleep(5000);
    printf("fechando programa\n");
    return 0;
}