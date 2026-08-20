#ifndef PROCESSOS_H
#define PROCESSOS_H

#define MAX_TASKS 100
#define MAX_ARGS 50
#define MAX_JOBS 100

typedef struct {
    char nome[50];
    char programa[256];
    char *argumentos[MAX_ARGS];
    int quantidadeArgumentos;
} Task;

#endif