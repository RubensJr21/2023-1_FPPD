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

#define QTD_CONSUMIDORES 4

// with_starvation

typedef struct {
    int id;
    unsigned long retorno;
    long* recurso_compartilhado;
} pkg_pthread_with_starvation;

void pkg_pthread_with_starvation_init(pkg_pthread_with_starvation* pkg, int id, long* recurso_comp) {
    pkg->id = id;
    pkg->retorno = 0;
    pkg->recurso_compartilhado = recurso_comp;
}

void* produtor_starvation(void* argv) {
    pkg_pthread_with_starvation* pkg = (pkg_pthread_with_starvation*)argv;
    while (1) {
        (*(pkg->recurso_compartilhado))++;
        pkg->retorno++;
    }
    return NULL;
}

void* consumidor_starvation(void* argv) {
    pkg_pthread_with_starvation* pkg = (pkg_pthread_with_starvation*)argv;
    while (1) {
        (*(pkg->recurso_compartilhado))--;
        pkg->retorno++;
    }
    return NULL;
}

typedef struct
{
    long* recurso_compartilhado;
}arg_with_starvation;

void* with_starvation(void* arg) {
    arg_with_starvation* arg_w_s = (arg_with_starvation*)arg;
    long* recurso_compartilhado = arg_w_s->recurso_compartilhado;
    //long recurso_compartilhado = 0;

    // produtor
    pthread_t produtor;
    pkg_pthread_with_starvation* pkg_produtor = (pkg_pthread_with_starvation*)malloc(sizeof(pkg_pthread_with_starvation));
    pkg_pthread_with_starvation_init(pkg_produtor, -1, recurso_compartilhado);

    pthread_create(&produtor, NULL, &produtor_starvation, pkg_produtor);

    // consumidores
    pkg_pthread_with_starvation pkgs_consumidores[QTD_CONSUMIDORES];
    //pkg_pthread_with_starvation* pkgs_consumidores = (pkg_pthread_with_starvation*)malloc(sizeof(pkg_pthread_with_starvation) * qtd_consumidores);
    if (pkgs_consumidores == NULL) {
        return NULL;
    }
    pthread_t consumidores[QTD_CONSUMIDORES];
    //pthread_t* consumidores = (pthread_t*)malloc(sizeof(pthread_t) * qtd_consumidores);
    if (consumidores == NULL) {
        return NULL;
    }
    for (int i = 0; i < QTD_CONSUMIDORES; i++) {
        pkg_pthread_with_starvation_init(&pkgs_consumidores[i], i, recurso_compartilhado);
        pthread_create(&consumidores[i], NULL, &consumidor_starvation, (void*)&pkgs_consumidores[i]);
    }

    pkg_pthread_with_starvation* pkg_aux = NULL;

    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pkg_aux = &pkgs_consumidores[i];
        printf("consumidor #%d consumiu: %ld\n", pkg_aux->id, pkg_aux->retorno);
    }

    pthread_cancel(produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pthread_cancel(consumidores[i]);
    }

    free(pkg_produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        free(&pkgs_consumidores[i]);
    }

    return NULL;
}
// ends with_starvation

// without_starvation
typedef struct {
    int id;
    unsigned long retorno;
    long* recurso_compartilhado;
    pthread_mutex_t* mutex;
} pkg_pthread_without_starvation;

void pkg_pthread_without_starvation_init(pkg_pthread_without_starvation* pkg, int id, long* recurso_comp, pthread_mutex_t* mutex) {
    pkg->id = id;
    pkg->retorno = 0;
    pkg->recurso_compartilhado = recurso_comp;
    pkg->mutex = mutex;
}

void* produtor_without_starvation(void* argv) {
    pkg_pthread_without_starvation* pkg = (pkg_pthread_without_starvation*)argv;
    while (1) {
        pthread_mutex_lock(pkg->mutex);
        (*(pkg->recurso_compartilhado))++;
        pkg->retorno++;
        pthread_mutex_unlock(pkg->mutex);
    }
    return NULL;
}

void* consumidor_without_starvation(void* argv) {
    pkg_pthread_without_starvation* pkg = (pkg_pthread_without_starvation*)argv;
    while (1) {
        pthread_mutex_lock(pkg->mutex);
        //printf("recurso: %ld\n", *(pkg->recurso_compartilhado));
        (*(pkg->recurso_compartilhado))--;
        pkg->retorno++;
        pthread_mutex_unlock(pkg->mutex);
    }
    return NULL;
}

typedef struct
{
    long* recurso_compartilhado;
    pthread_mutex_t* mutex;
}arg_without_starvation;

void* without_starvation(void* arg) {

    arg_without_starvation* arg_wt_s = (arg_without_starvation*)arg;
    pthread_mutex_t* mutex = arg_wt_s->mutex;
    long* recurso_compartilhado = arg_wt_s->recurso_compartilhado;

    // produtor
    pthread_t* produtor = (pthread_t*)malloc(sizeof(pthread_t));
    if (!produtor)
    {
        return NULL;
    }
    pkg_pthread_without_starvation* pkg_produtor = (pkg_pthread_without_starvation*)malloc(sizeof(pkg_pthread_without_starvation));
    pkg_pthread_without_starvation_init(pkg_produtor, -1, recurso_compartilhado, mutex);

    pthread_create(produtor, NULL, &produtor_without_starvation, pkg_produtor);

    // consumidores
    pkg_pthread_without_starvation pkgs_consumidores[QTD_CONSUMIDORES];
    if (pkgs_consumidores == NULL) {
        return NULL;
    }
    pthread_t consumidores[QTD_CONSUMIDORES];
    if (consumidores == NULL) {
        return NULL;
    }
    for (int i = 0; i < QTD_CONSUMIDORES; i++) {
        pkg_pthread_without_starvation_init(&pkgs_consumidores[i], i, recurso_compartilhado, mutex);
        pthread_create(&consumidores[i], NULL, &consumidor_without_starvation, (void*)&pkgs_consumidores[i]);
    }

    pkg_pthread_without_starvation* pkg_aux = NULL;

    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pkg_aux = &pkgs_consumidores[i];
        printf("consumidor #%d consumiu: %ld\n", pkg_aux->id, pkg_aux->retorno);
    }

    pthread_cancel(*produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pthread_cancel(consumidores[i]);
    }

    free(pkg_produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        free(&pkgs_consumidores[i]);
    }

    return NULL;
}
// ends without_starvation

// with_limit
typedef struct {
    int id;
    unsigned long retorno;
    long* recurso_compartilhado;
    pthread_mutex_t* mutex;
    sem_t* pode_acabar;
} pkg_produtor_pthread_with_limit;

void pkg_produtor_pthread_with_limit_init(pkg_produtor_pthread_with_limit* pkg, int id, long* recurso_comp, pthread_mutex_t* mutex, sem_t* pode_acabar) {
    pkg->id = id;
    pkg->retorno = 0;
    pkg->recurso_compartilhado = recurso_comp;
    pkg->mutex = mutex;
    pkg->pode_acabar = pode_acabar;
}

void* produtor_with_limit(void* argv) {
    pkg_produtor_pthread_with_limit* pkg = (pkg_produtor_pthread_with_limit*)argv;
    int contador = 0;
    while (contador++ < 5000) {
        pthread_mutex_lock(pkg->mutex);
        (*(pkg->recurso_compartilhado))++;
        pkg->retorno++;
        pthread_mutex_unlock(pkg->mutex);
    }
    printf("produtor produziu 5000 unidades. Sinalizando...\n");
    sem_post(pkg->pode_acabar);
    return NULL;
}

typedef struct {
    int id;
    unsigned long retorno;
    long* recurso_compartilhado;
    pthread_mutex_t* mutex;
} pkg_consumidor_pthread_with_limit;

void pkg_consumidor_pthread_with_limit_init(pkg_consumidor_pthread_with_limit* pkg, int id, long* recurso_comp, pthread_mutex_t* mutex) {
    pkg->id = id;
    pkg->retorno = 0;
    pkg->recurso_compartilhado = recurso_comp;
    pkg->mutex = mutex;
}

void* consumidor_with_limit(void* argv) {
    pkg_consumidor_pthread_with_limit* pkg = (pkg_consumidor_pthread_with_limit*)argv;
    while (1) {
        pthread_mutex_lock(pkg->mutex);
        (*(pkg->recurso_compartilhado))--;
        pkg->retorno++;
        pthread_mutex_unlock(pkg->mutex);
    }
    return NULL;
}

typedef struct
{
    long* recurso_compartilhado;
    pthread_mutex_t* mutex;
    sem_t* pode_acabar;
}arg_with_limit;

void* with_limit(void* arg) {

    arg_with_limit* arg_w_l = (arg_with_limit*)arg;
    pthread_mutex_t* mutex = arg_w_l->mutex;
    long* recurso_compartilhado = arg_w_l->recurso_compartilhado;

    // produtor
    pthread_t* produtor = (pthread_t*)malloc(sizeof(pthread_t));
    if (!produtor)
    {
        return NULL;
    }
    pkg_produtor_pthread_with_limit* pkg_produtor = (pkg_produtor_pthread_with_limit*)malloc(sizeof(pkg_produtor_pthread_with_limit));
    pkg_produtor_pthread_with_limit_init(pkg_produtor, -1, recurso_compartilhado, mutex, arg_w_l->pode_acabar);

    pthread_create(produtor, NULL, &produtor_with_limit, pkg_produtor);

    // consumidores
    pkg_consumidor_pthread_with_limit pkgs_consumidores[QTD_CONSUMIDORES];
    if (pkgs_consumidores == NULL) {
        return NULL;
    }
    pthread_t consumidores[QTD_CONSUMIDORES];
    if (consumidores == NULL) {
        return NULL;
    }
    for (int i = 0; i < QTD_CONSUMIDORES; i++) {
        pkg_consumidor_pthread_with_limit_init(&pkgs_consumidores[i], i, recurso_compartilhado, mutex);
        pthread_create(&consumidores[i], NULL, &consumidor_with_limit, (void*)&pkgs_consumidores[i]);
    }

    pkg_consumidor_pthread_with_limit* pkg_aux = NULL;

    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pkg_aux = &pkgs_consumidores[i];
        printf("consumidor #%d consumiu: %ld\n", pkg_aux->id, pkg_aux->retorno);
    }

    pthread_cancel(*produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        pthread_cancel(consumidores[i]);
    }

    free(pkg_produtor);
    for (int i = 0; i < QTD_CONSUMIDORES; i++)
    {
        free(&pkgs_consumidores[i]);
    }

    return NULL;
}
// ends with_limit

// 1 produtor e N consumidores
//int main() {
//    pthread_t thread;
//    long recurso_compartilhado = 0;
//
//    arg_with_starvation* arg = (arg_with_starvation*)malloc(sizeof(arg_with_starvation));
//    arg->recurso_compartilhado = &recurso_compartilhado;
//    pthread_create(&thread, NULL, &with_starvation, (void*) arg);
//
//    const int miligundos = 5000;
//    Sleep(miligundos);
//
//    pthread_cancel(thread);
//    free(arg);
//
//    return 0;
//}

//int main() {
//    pthread_t thread;
//    long recurso_compartilhado = 0;
//
//    // mutex
//    pthread_mutex_t mutex;
//    pthread_mutex_init(&mutex, NULL);
//
//    arg_without_starvation* arg = (arg_without_starvation*)malloc(sizeof(arg_without_starvation));
//    arg->mutex = &mutex;
//    arg->recurso_compartilhado = &recurso_compartilhado;
//
//    pthread_create(&thread, NULL, &without_starvation, (void *) arg);
//
//    const int miligundos = 1000;
//    Sleep(miligundos);
//
//    pthread_cancel(thread);
//    pthread_mutex_destroy(&mutex);
//    free(arg);
//
//    return 0;
//}

int main() {
    pthread_t thread;
    long recurso_compartilhado = 0;

    // mutex
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    sem_t pode_acabar;
    sem_init(&pode_acabar, 0, 0);

    arg_with_limit* arg = (arg_with_limit*)malloc(sizeof(arg_with_limit));
    if (!arg)
    {
        return NULL;
    }
    arg->mutex = &mutex;
    arg->recurso_compartilhado = &recurso_compartilhado;
    arg->pode_acabar = &pode_acabar;

    pthread_create(&thread, NULL, &with_limit, (void*)arg);

    sem_wait(&pode_acabar);
    printf("Esperou...\n");

    pthread_cancel(thread);
    sem_close(&pode_acabar);
    pthread_mutex_destroy(&mutex);
    free(arg);

    return 0;
}