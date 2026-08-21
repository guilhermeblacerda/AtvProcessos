#include "processos.h"

void inicializar_orquestrador(Orquestrador *orquestrador) {
    orquestrador->quantidade_tarefas = 0;
    orquestrador->quantidade_trabalhos = 0;
    orquestrador->proximo_identificador_trabalho = 1;
    strcpy(orquestrador->diretorio_trabalho, ".");
}

void limpar_string(char *texto) {
    char *origem = texto;
    char *destino = texto;
    int espaco_anterior = 0;
    
    while (*origem == ' ') {
        origem++;
    }
    
    while (*origem) {
        if (*origem == ' ') {
            if (!espaco_anterior) {
                *destino++ = ' ';
                espaco_anterior = 1;
            }
        } else {
            *destino++ = *origem;
            espaco_anterior = 0;
        }
        origem++;
    }
    *destino = '\0';
    
    int comprimento = strlen(texto);
    if (comprimento > 0 && texto[comprimento-1] == ' ') {
        texto[comprimento-1] = '\0';
    }
}

int contar_argumentos(char *argumentos[]) {
    int contador = 0;
    while (argumentos[contador] != NULL) {
        contador++;
    }
    return contador;
}

Tarefa* encontrar_tarefa(Orquestrador *orquestrador, char *nome) {
    for (int i = 0; i < orquestrador->quantidade_tarefas; i++) {
        if (strcmp(orquestrador->tarefas[i].nome, nome) == 0) {
            return &orquestrador->tarefas[i];
        }
    }
    return NULL;
}

void processar_comando(Orquestrador *orquestrador, char *linha) {
    limpar_string(linha);
    
    if (strlen(linha) == 0) {
        return;
    }
    
    char *argumentos[MAXIMO_ARGUMENTOS];
    int quantidade_argumentos = 0;
    char *token = strtok(linha, " ");
    
    while (token != NULL && quantidade_argumentos < MAXIMO_ARGUMENTOS - 1) {
        argumentos[quantidade_argumentos++] = token;
        token = strtok(NULL, " ");
    }
    argumentos[quantidade_argumentos] = NULL;
    
    if (quantidade_argumentos == 0) {
        return;
    }
    
    if (strcmp(argumentos[0], "exit") == 0) {
        exit(0);
    } else {
        printf("Comando desconhecido: %s\n", argumentos[0]);
    }
}

int main(int argc, char *argv[]) {
    Orquestrador orquestrador;
    inicializar_orquestrador(&orquestrador);
    
    if (argc > 2) {
        printf("Uso: %s [workflowFile]\n", argv[0]);
        return 1;
    }
    
    if (argc == 1) {
        char linha[TAMANHO_MAXIMO_LINHA];
        while (1) {
            printf("processflow> ");
            if (fgets(linha, sizeof(linha), stdin) == NULL) {
                printf("\n");
                break;
            }
            linha[strcspn(linha, "\n")] = '\0';
            
            char *copia_linha = strdup(linha);
            processar_comando(&orquestrador, copia_linha);
            free(copia_linha);
        }
    }
    
    return 0;
}