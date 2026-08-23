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
    }else if (strcmp(argumentos[0], "input") == 0) {
        comando_entrada(orquestrador, argumentos);
    } else if (strcmp(argumentos[0], "output") == 0) {
        comando_saida(orquestrador, argumentos);
    } else if (strcmp(argumentos[0], "append") == 0) {
        comando_anexar(orquestrador, argumentos);
    }else if (strcmp(argumentos[0], "start") == 0) {
    comando_iniciar_fundo(orquestrador, argumentos);
    } else if (strcmp(argumentos[0], "jobs") == 0) {
        comando_listar_trabalhos(orquestrador);
    } else if (strcmp(argumentos[0], "wait") == 0) {
    comando_esperar_trabalho(orquestrador, argumentos);
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
        
        if (tarefa->arquivo_entrada) {
            int descritor = open(tarefa->arquivo_entrada, O_RDONLY);
            if (descritor < 0) {
                perror("Erro ao abrir arquivo de entrada");
                exit(1);
            }
            dup2(descritor, STDIN_FILENO);
            close(descritor);
        }
        
        if (tarefa->arquivo_saida) {
            int descritor = open(tarefa->arquivo_saida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (descritor < 0) {
                perror("Erro ao abrir arquivo de saída");
                exit(1);
            }
            dup2(descritor, STDOUT_FILENO);
            close(descritor);
        }
        
        if (tarefa->arquivo_anexar) {
            int descritor = open(tarefa->arquivo_anexar, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (descritor < 0) {
                perror("Erro ao abrir arquivo para anexar");
                exit(1);
            }
            dup2(descritor, STDOUT_FILENO);
            close(descritor);
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
    } else if (strcmp(argumentos[1], "pipe") == 0) {
        comando_executar_tubulacao(orquestrador, argumentos, 2);
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

void comando_entrada(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 3) {
        printf("Erro: Uso correto: input <tarefa> <arquivo>\n");
        return;
    }
    
    Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[1]);
    if (tarefa == NULL) {
        printf("Erro: Tarefa '%s' não encontrada\n", argumentos[1]);
        return;
    }
    
    if (access(argumentos[2], F_OK) != 0) {
        printf("Erro: Arquivo '%s' não existe\n", argumentos[2]);
        return;
    }
    
    tarefa->arquivo_entrada = strdup(argumentos[2]);
    printf("Entrada de '%s' redirecionada para '%s'\n", argumentos[1], argumentos[2]);
}

void comando_saida(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 3) {
        printf("Erro: Uso correto: output <tarefa> <arquivo>\n");
        return;
    }
    
    Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[1]);
    if (tarefa == NULL) {
        printf("Erro: Tarefa '%s' não encontrada\n", argumentos[1]);
        return;
    }
    
    tarefa->arquivo_saida = strdup(argumentos[2]);
    printf("Saída de '%s' redirecionada para '%s'\n", argumentos[1], argumentos[2]);
}

void comando_anexar(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 3) {
        printf("Erro: Uso correto: append <tarefa> <arquivo>\n");
        return;
    }
    
    Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[1]);
    if (tarefa == NULL) {
        printf("Erro: Tarefa '%s' não encontrada\n", argumentos[1]);
        return;
    }
    
    tarefa->arquivo_anexar = strdup(argumentos[2]);
    printf("Saída de '%s' redirecionada (append) para '%s'\n", argumentos[1], argumentos[2]);
}

void comando_executar_tubulacao(Orquestrador *orquestrador, char *argumentos[], int inicio) {
    int quantidade_tarefas = contar_argumentos(argumentos) - inicio;
    
    if (quantidade_tarefas < 2) {
        printf("Erro: Tubulação precisa de pelo menos 2 tarefas\n");
        return;
    }
    
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (encontrar_tarefa(orquestrador, argumentos[inicio + i]) == NULL) {
            printf("Erro: Tarefa '%s' não encontrada\n", argumentos[inicio + i]);
            return;
        }
    }
    
    int tubo_descritores[2];
    int tubo_descritores_anterior[2];
    pid_t pids_processos[MAXIMO_TAREFAS];
    int quantidade_processos = 0;
    
    for (int i = 0; i < quantidade_tarefas; i++) {
        Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[inicio + i]);
        
        if (i < quantidade_tarefas - 1) {
            if (pipe(tubo_descritores) < 0) {
                perror("Erro ao criar tubulação");
                return;
            }
        }
        
        pid_t pid_processo = fork();
        if (pid_processo == 0) {
            if (i > 0) {
                dup2(tubo_descritores_anterior[0], STDIN_FILENO);
                close(tubo_descritores_anterior[0]);
                close(tubo_descritores_anterior[1]);
            }
            
            if (i < quantidade_tarefas - 1) {
                dup2(tubo_descritores[1], STDOUT_FILENO);
                close(tubo_descritores[0]);
                close(tubo_descritores[1]);
            }
            
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
            
            if (i > 0) {
                close(tubo_descritores_anterior[0]);
                close(tubo_descritores_anterior[1]);
            }
            
            if (i < quantidade_tarefas - 1) {
                tubo_descritores_anterior[0] = tubo_descritores[0];
                tubo_descritores_anterior[1] = tubo_descritores[1];
            }
        }
    }
    
    for (int i = 0; i < quantidade_processos; i++) {
        waitpid(pids_processos[i], NULL, 0);
    }
}

void comando_iniciar_fundo(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 2) {
        printf("Erro: Uso correto: start <tarefa>\n");
        return;
    }
    
    Tarefa *tarefa = encontrar_tarefa(orquestrador, argumentos[1]);
    if (tarefa == NULL) {
        printf("Erro: Tarefa '%s' não encontrada\n", argumentos[1]);
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
        Trabalho *novo_trabalho = &orquestrador->trabalhos[orquestrador->quantidade_trabalhos];
        novo_trabalho->identificador = orquestrador->proximo_identificador_trabalho++;
        novo_trabalho->pid_processo = pid_processo;
        novo_trabalho->nome_tarefa = strdup(argumentos[1]);
        novo_trabalho->esta_rodando = 1;
        orquestrador->quantidade_trabalhos++;
        
        printf("[%d] %d\n", novo_trabalho->identificador, pid_processo);
    }
}

void comando_listar_trabalhos(Orquestrador *orquestrador) {
    if (orquestrador->quantidade_trabalhos == 0) {
        printf("Nenhum trabalho em execução\n");
        return;
    }
    
    for (int i = 0; i < orquestrador->quantidade_trabalhos; i++) {
        if (orquestrador->trabalhos[i].esta_rodando) {
            int status_saida;
            pid_t resultado = waitpid(orquestrador->trabalhos[i].pid_processo, &status_saida, WNOHANG);
            
            if (resultado == orquestrador->trabalhos[i].pid_processo) {
                orquestrador->trabalhos[i].esta_rodando = 0;
                printf("[%d] %s finalizado (PID: %d)\n", 
                       orquestrador->trabalhos[i].identificador, 
                       orquestrador->trabalhos[i].nome_tarefa,
                       orquestrador->trabalhos[i].pid_processo);
            } else if (resultado == 0) {
                printf("[%d] %s em execução (PID: %d)\n", 
                       orquestrador->trabalhos[i].identificador,
                       orquestrador->trabalhos[i].nome_tarefa,
                       orquestrador->trabalhos[i].pid_processo);
            }
        }
    }
}

void comando_esperar_trabalho(Orquestrador *orquestrador, char *argumentos[]) {
    int quantidade = contar_argumentos(argumentos);
    
    if (quantidade < 2) {
        printf("Erro: Uso correto: wait <identificador_trabalho>\n");
        return;
    }
    
    int identificador = atoi(argumentos[1]);
    Trabalho *trabalho_encontrado = NULL;
    
    for (int i = 0; i < orquestrador->quantidade_trabalhos; i++) {
        if (orquestrador->trabalhos[i].identificador == identificador) {
            trabalho_encontrado = &orquestrador->trabalhos[i];
            break;
        }
    }
    
    if (trabalho_encontrado == NULL || !trabalho_encontrado->esta_rodando) {
        printf("Erro: Trabalho %d não encontrado ou já finalizado\n", identificador);
        return;
    }
    
    printf("Aguardando trabalho %d...\n", identificador);
    waitpid(trabalho_encontrado->pid_processo, NULL, 0);
    trabalho_encontrado->esta_rodando = 0;
    printf("Trabalho %d finalizado\n", identificador);
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