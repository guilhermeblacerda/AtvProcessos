#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define TAMANHO_MAXIMO_LINHA 1024
#define MAXIMO_ARGUMENTOS 64
#define MAXIMO_TAREFAS 100
#define MAXIMO_TRABALHOS 100

typedef struct {
    char *nome;
    char *programa;
    char **argumentos;
    int quantidade_argumentos;
    char *arquivo_entrada;
    char *arquivo_saida;
    char *arquivo_anexar;
} Tarefa;

typedef struct {
    int identificador;
    pid_t pid_processo;
    char *nome_tarefa;
    int esta_rodando;
} Trabalho;

typedef struct {
    Tarefa tarefas[MAXIMO_TAREFAS];
    int quantidade_tarefas;
    Trabalho trabalhos[MAXIMO_TRABALHOS];
    int quantidade_trabalhos;
    int proximo_identificador_trabalho;
    char diretorio_trabalho[256];
} Orquestrador;

#endif