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

    while(token != NULL) {
        for(int i = 0; i < 5; i++) {
            if(strcmp(token, keywords[i]) == 0) {
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
    int fd_first_to_second[2];
    int fd_second_to_third[2];
    int input_file_size;

    if (pipe(fd_first_to_second) < 0) {
        printf("Error");
        exit(-1);
    }
    if (pipe(fd_second_to_third) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }

    pid_t pid_1, pid_2, pid_3; // дочерние процессы

    pid_1 = fork(); 

    if (pid_1 < 0) { 
        printf("can\'t fork\n");
        exit(-1);
    } else if (pid_1 == 0) { // дочерний 1 
        printf("child #1\n");
        int input_file = open(input_file_name, O_RDONLY); // открываем файл на чтение
        if (stat(input_file_name, &st) == 0) {
            input_file_size = st.st_size;
        }
        char buffer[input_file_size+1];
        buffer[input_file_size] = '\0';
        read(input_file, buffer, input_file_size); // читаем файл
        write(fd_first_to_second[1], buffer, strlen(buffer) + 1); // пишем в канал

        close(input_file); // закрывваем канал
    } else { // родитель
        pid_2 = fork();
        if (pid_2 < 0) {  
            printf("can\'t fork\n");
        } else if (pid_2 == 0) { // дочерний 2
            printf("child #2\n");
            close(fd_first_to_second[1]); 
            read(fd_first_to_second[0], buffer, size);
            count_words(buffer);
            write(fd_second_to_third[1], buffer, strlen(buffer) + 1);
            close(fd_second_to_third[1]); // закрываем канал
        } else {  // родитель
            pid_3 = fork();

            if (pid_3 < 0) {  
                printf("can\'t fork\n");
            } else if (pid_3 == 0) { // дочерний 3
                int output_file = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644); // use O_TRUNC to truncate the output file if it already exists

                read(fd_second_to_third[0], buffer, size);
                char *str = buffer;
                int bytes_written = write(output_file, str, strlen(str));
                close(output_file);
            } else { // родитель
                close(fd_first_to_second[0]);
                close(fd_first_to_second[1]);
                close(fd_second_to_third[0]);
                close(fd_second_to_third[1]); // закрываем каналы
            }
        } 
    }

    return 0;
}