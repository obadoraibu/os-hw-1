#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

const int size = 5000;

struct stat st;

// функция подсчета ключевых слов в строке
void count_words(char *str) {
    int counter[5] = {0, 0, 0, 0, 0};

    char *keywords[] = {"if", "else", "for", "while", "switch"};
    char *token = strtok(str, " ,.-\n\r\t(){}[];:");

    while (token != NULL) {
        for (int i = 0; i < 5; i++) {
            if (strcmp(token, keywords[i]) == 0) {
                counter[i]++;
            }
        }
        token = strtok(NULL, " ,.-\n\r\t(){}[];:");
    }

    char output[size];
    output[0] = '\0';
    for (int i = 0; i < 5; i++) {
        char symbol[10];
        sprintf(symbol, "%s:%d\n", keywords[i], counter[i]);
        strcat(output, symbol);
    }

    memset(str, 0, size);
    strcpy(str, output);
}

int main(int argc, char *argv[]) {
    char buffer[size]; //буффер для работы с каналами
    if (argc != 3) {
        printf("Not enough args");
        return 0;
    }

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    // Создаем два именованных канала (fifo)
    int first_to_second = mknod("first_to_second", S_IFIFO | 0666, 0);
    int second_to_third = mknod("second_to_third", S_IFIFO | 0666, 0);

    if (first_to_second < 0) {
        perror("Error creating first_to_second fifo");
        exit(EXIT_FAILURE);
    }
    if (second_to_third < 0) {
        perror("Error creating second_to_third fifo");
        exit(EXIT_FAILURE);
    }

    pid_t pid_1, pid_2, pid_3; // дочерние процессы

    pid_1 = fork();

    if (pid_1 < 0) {
        perror("Error forking pid_1");
        exit(EXIT_FAILURE);
    } else if (pid_1 == 0) { // дочерний 1
        printf("child #1\n");
        int input_file = open(input_file_name, O_RDONLY);
        if (stat(input_file_name, &st) == 0) {
            int input_file_size = st.st_size;
            char buffer[input_file_size + 1];
            buffer[input_file_size] = '\0';
            read(input_file, buffer, input_file_size);
            write(first_to_second, buffer, strlen(buffer) + 1);
        } else {
            perror("Error getting input file size");
            exit(EXIT_FAILURE);
        }
        close(input_file);
    } else { // родитель
        pid_2 = fork();
        if (pid_2 < 0) {
            perror("Error forking pid_2");
        } else if (pid_2 == 0) { // дочерний 2
            printf("child #2\n");
            close(first_to_second); // закрываем доступ к каналу первому дочернему процессу
            int second_to_third_output_file = open("second_to_third_output_file.txt", O_WRONLY | O_CREAT, 0666);
            dup2(second_to_third_output_file, STDOUT_FILENO); // перенаправляем stdout в файл
            close(second_to_third_output_file);
            int second_to_third_input = open("second_to_third", O_RDONLY);
            read(second_to_third_input, buffer, size);
            char *arg_list[] = {"grep", "-v", "^$", NULL}; // аргументы для команды grep
            execvp("grep", arg_list); // запускаем команду grep
            close(second_to_third_input);
        } else { // родитель
            pid_3 = fork();
            if (pid_3 < 0) {
                perror("Error forking pid_3");
            } else if (pid_3 == 0) { // дочерний 3
                printf("child #3\n");
                close(second_to_third); // закрываем доступ к каналу второму дочернему процессу
                int first_to_second_input = open("first_to_second", O_RDONLY);
                dup2(first_to_second_input, STDIN_FILENO); // перенаправляем stdin из канала
                close(first_to_second_input);
                int output_file = open(output_file_name, O_WRONLY | O_CREAT, 0666);
                dup2(output_file, STDOUT_FILENO); // перенаправляем stdout в файл
                close(output_file);
                char *arg_list[] = {"sort", NULL}; // аргументы для команды sort
                execvp("sort", arg_list); // запускаем команду sort
            } else { // родитель
                waitpid(pid_1, NULL, 0);
                waitpid(pid_2, NULL, 0);
                waitpid(pid_3, NULL, 0);
                printf("All children are done\n");
                unlink("first_to_second");
                unlink("second_to_third");
            }
        }
    }
}