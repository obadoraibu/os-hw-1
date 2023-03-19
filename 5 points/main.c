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
char *count_words(char *str) {
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
    char *output = malloc(size);
    output[0] = '\0';
    for (int i = 0; i < 5; i++) {
        char symbol[10];
        snprintf(symbol, sizeof(symbol), "%s:%d\n", keywords[i], counter[i]);
        strcat(output, symbol);
    }
    memset(str, 0, size);
    strcpy(str, output);
    free(output);
    return str;
}

int main(int argc, char *argv[]) {
    int fd;
    size_t size_n;
    char buffer[size]; //буффер для работы с каналами
    if (argc != 3) {
        printf("Not enough args");
        return 0;
    }
    char *input_file_name = argv[1];
    char *output_file_name = argv[2];
    int input_file_size;
    // Создаем два именованных канала
    
    mknod("first_to_second", S_IFIFO | 0666, 0);
    mknod("second_to_third", S_IFIFO | 0666, 0);
    pid_t pid_1, pid_2, pid_3; // дочерние процессы
    pid_1 = fork();
    if (pid_1 < 0) {
        perror("Error forking pid_1");
        exit(EXIT_FAILURE);
    } else if (pid_1 == 0) { // дочерний 1
        printf("child #1\n");
        struct stat st;
        int input_file = open(input_file_name, O_RDONLY); // открываем файл на чтение
        if (fstat(input_file, &st) == -1) {
            perror("fstat");
            exit(EXIT_FAILURE);
        }
        off_t input_file_size = st.st_size;
        char buf[input_file_size+1];
        buf[input_file_size] = '\0'; // локальный буфер для чтения из файла
        ssize_t bytes_read = read(input_file, buf, input_file_size);  // читаем из файла
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (close(input_file) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        int fd = open("first_to_second", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // открываем канал на запись
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        ssize_t bytes_written = write(fd, buf, input_file_size); // пишем
        if (bytes_written == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (bytes_written != input_file_size) {
            fprintf(stderr, "partial write");
            exit(EXIT_FAILURE);
        }   
        if (close(fd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    } else { // родитель
        pid_2 = fork();
        if (pid_2 < 0) {
            perror("Error forking pid_2");
        } else if (pid_2 == 0) { // дочерний 2
        printf("child #2\n");
            if((fd = open("first_to_second", O_RDONLY)) < 0){  // открываем канал на чтение
                printf("Can\'t open FIFO for reading\n");
                exit(-1);
            }
            size_n = read(fd, buffer, size);
            printf("%s\n", buffer);
            if (size_n < 0) {
                printf("Can\'t read string from FIFO\n");
                exit(-1);
            }
            if(close(fd) < 0){
                printf("Reader: Can\'t close FIFO\n"); exit(-1);
            }
            char *str = buffer;
            str = count_words(buffer);

            if((fd = open("second_to_third", O_WRONLY | O_CREAT)) < 0){  // открываем канал на запись
                printf("Can\'t open FIFO for writting\n");
                exit(-1);
            }
            size_n = write(fd, str, strlen(str) + 1);
            if(size_n != strlen(str) + 1){
                exit(-1);
            }
            if(close(fd) < 0){  // закрытие именованного канала
                printf("Writer: Can\'t close FIFO\n"); 
                exit(-1);
            }
        } else { // родитель
            pid_3 = fork();
            
            if (pid_3 == 0) { // дочерний 3
                printf("child process #3\n");
                int output_file = open(output_file_name, O_WRONLY | O_CREAT, 0644);  // открываем файл на запись
                if((fd = open("second_to_third", O_RDONLY)) < 0){
                    printf("Can\'t open FIFO for reading\n");
                    exit(-1);
                }
                size_n = read(fd, buffer, size);
                char *str = buffer;
                if (size_n < 0) {
                    printf("Can\'t read string from FIFO\n");
                    exit(-1);
                }
                if(close(fd) < 0){
                    printf("Reader: Can\'t close FIFO\n"); exit(-1);
                }
                write(output_file, str, strlen(str));
                
                close(output_file);
            } else {  
                wait(NULL);
                printf("Конец программы\n");
            }
        }
    }
}