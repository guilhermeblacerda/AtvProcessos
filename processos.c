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
    } else if (strcmp(argumentos[0], "task") == 0) {
        comando_tarefa(orquestrador, argumentos);
    } else if (strcmp(argumentos[0], "run") == 0) {
    comando_executar(orquestrador, argumentos);
    }else {
        printf("Comando desconhecido: %s\n", argumentos[0]);
    }
}

void comando_tarefa(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 3) {
        printf("Erro: Uso correto: task <nome> <programa> [argumentos...]\n");
        return;
    }
    
    if (orquestrador->quantidade_tarefas >= MAXIMO_TAREFAS) {
        printf("Erro: Número máximo de tarefas atingido\n");
        return;
    }
    
    if (encontrar_tarefa(orquestrador, argumentos[1]) != NULL) {
        printf("Erro: Tarefa '%s' já está cadastrada\n", argumentos[1]);
        return;
    }
    
    Tarefa *nova_tarefa = &orquestrador->tarefas[orquestrador->quantidade_tarefas];
    nova_tarefa->nome = strdup(argumentos[1]);
    nova_tarefa->programa = strdup(argumentos[2]);
    nova_tarefa->arquivo_entrada = NULL;
    nova_tarefa->arquivo_saida = NULL;
    nova_tarefa->arquivo_anexar = NULL;
    
    nova_tarefa->quantidade_argumentos = quantidade - 3;
    nova_tarefa->argumentos = malloc((nova_tarefa->quantidade_argumentos + 2) * sizeof(char*));
    nova_tarefa->argumentos[0] = strdup(argumentos[2]);
    
    for (int i = 0; i < nova_tarefa->quantidade_argumentos; i++) {
        nova_tarefa->argumentos[i + 1] = strdup(argumentos[i + 3]);
    }
    nova_tarefa->argumentos[nova_tarefa->quantidade_argumentos + 1] = NULL;
    
    orquestrador->quantidade_tarefas++;
    printf("Tarefa '%s' cadastrada com sucesso\n", nova_tarefa->nome);
}

void executar_tarefa(Tarefa *tarefa, char *diretorio_trabalho) {
    pid_t pid_processo = fork();
    
    if (pid_processo == 0) {
        if (chdir(diretorio_trabalho) != 0) {
            perror("Erro ao mudar para diretório de trabalho");
            exit(1);
        }
        
        execvp(tarefa->programa, tarefa->argumentos);
        perror("Erro ao executar programa");
        exit(1);
        
    } else if (pid_processo < 0) {
        perror("Erro ao criar processo filho");
    } else {
        int status_saida;
        waitpid(pid_processo, &status_saida, 0);
        
        if (WIFEXITED(status_saida) && WEXITSTATUS(status_saida) != 0) {
            printf("Aviso: Processo terminou com código de saída %d\n", 
                   WEXITSTATUS(status_saida));
        }
    }
}

void comando_executar_sequencial(Orquestrador *orquestrador, char *argumentos[], int inicio) {
    int quantidade_tarefas = contar_argumentos(argumentos) - inicio;
    
    if (quantidade_tarefas < 1) {
        printf("Erro: Pelo menos uma tarefa deve ser informada\n");
        return;
    }
    
    for (int i = 0; i < quantidade_tarefas; i++) {
        Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[inicio + i]);
        if (tarefa == NULL) {
            printf("Erro: Tarefa '%s' não encontrada\n", argumentos[inicio + i]);
            return;
        }
        executar_tarefa(tarefa, orquestrador->diretorio_trabalho);
    }
}

void comando_executar(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 3) {
        printf("Erro: Uso correto: run <sequential|parallel|pipe> <tarefas...>\n");
        return;
    }
    
    if (strcmp(argumentos[1], "sequential") == 0) {
        comando_executar_sequencial(orquestrador, argumentos, 2);
    } else if (strcmp(argumentos[1], "parallel") == 0) {
        comando_executar_paralelo(orquestrador, argumentos, 2);
    } else {
        printf("Erro: Modo inválido. Use 'sequential', 'parallel' ou 'pipe'\n");
    }
}

void comando_executar_paralelo(Orquestrador *orquestrador, char *argumentos[], int inicio) {
    int quantidade_tarefas = contar_argumentos(argumentos) - inicio;
    
    if (quantidade_tarefas < 1) {
        printf("Erro: Pelo menos uma tarefa deve ser informada\n");
        return;
    }
    
    pid_t pids_processos[MAXIMO_TAREFAS];
    int quantidade_processos = 0;
    
    for (int i = 0; i < quantidade_tarefas; i++) {
        Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[inicio + i]);
        if (tarefa == NULL) {
            printf("Erro: Tarefa '%s' não encontrada\n", argumentos[inicio + i]);
            for (int j = 0; j < quantidade_processos; j++) {
                kill(pids_processos[j], SIGTERM);
            }
            return;
        }
        
        pid_t pid_processo = fork();
        if (pid_processo == 0) {
            if (chdir(orquestrador->diretorio_trabalho) != 0) {
                perror("Erro ao mudar diretório de trabalho");
                exit(1);
            }
            execvp(tarefa->programa, tarefa->argumentos);
            perror("Erro ao executar programa");
            exit(1);
        } else if (pid_processo < 0) {
            perror("Erro ao criar processo filho");
            return;
        } else {
            pids_processos[quantidade_processos++] = pid_processo;
        }
    }
    
    for (int i = 0; i < quantidade_processos; i++) {
        int status_saida;
        waitpid(pids_processos[i], &status_saida, 0);
        if (WIFEXITED(status_saida) && WEXITSTATUS(status_saida) != 0) {
            printf("Aviso: Processo %d terminou com código %d\n", 
                   pids_processos[i], WEXITSTATUS(status_saida));
        }
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