int main(int argc, char *argv[]) {

    if (argc > 2) {
        printf("Uso: %s [workflowFile]\n", argv[0]);
        return 1;
    }

    if (argc == 1) {

        char linha[1024];

        while (1) {

            printf("processflow> ");

            if (fgets(linha, sizeof(linha), stdin) == NULL) {
                break;
            }

            linha[strcspn(linha, "\n")] = '\0';

            if (strcmp(linha, "exit") == 0) {
                break;
            }

            printf("Comando: %s\n", linha);
        }
    }

    return 0;
}