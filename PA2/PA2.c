#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_THREADS 30

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int pipes[MAX_THREADS][2];

void *thread_func1(void *args)
{
    pthread_mutex_lock(&lock);
    int pipe_number = (int)args;
    int buffer_size = 1000;
    char buffer[buffer_size];
    read(pipes[pipe_number][0], buffer, buffer_size);
    printf("---- %ld\n", pthread_self());
    printf("%s", buffer);
    printf("---- %ld\n", pthread_self());
    fflush(stdout);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[])
{
    FILE *file_ptr;
    pthread_t threads[MAX_THREADS];
    int thread_status[MAX_THREADS];
    int thread_count = 0;
    file_ptr = fopen("./commands.txt", "r");
    if (file_ptr == NULL)
    {
        exit(1);
    }
    char line[256];
    int command_number = 0;
    for (; fgets(line, sizeof(line), file_ptr);)
    {
        char redirect_symbol = '-';
        char background_flag = 'n';

        for (int i = 0; i < strlen(line); i++)
        {
            if (line[i] == '<')
            {
                redirect_symbol = '<';
            }
            else if (line[i] == '>')
            {
                redirect_symbol = '>';
            }
            if (line[i] == '&')
            {
                background_flag = 'y';
            }
        }
        char *arguments[10];
        int file_flag = 0;
        int argument_count = 0;
        char *file_name;
        int on_word = 0;
        char word_length = 0;
        int file_name_length = 0;
        int i = 0;
        while (i < strlen(line) && !(line[i] == '&' || line[i] == '\n'))
        {
            if (line[i] != ' ' && line[i] != '<' && line[i] != '>')
            {
                on_word = 1;
                word_length++;
                if ((i + 1 < strlen(line) && line[i + 1] == '\n') || i + 1 == strlen(line))
                {
                    char *word = (char *)malloc(sizeof(char) * (word_length + 1));
                    memcpy(word, line + i + 1 - word_length, word_length);
                    arguments[argument_count] = strdup(word);
                    argument_count++;
                    break;
                }
            }
            else if (on_word == 1 && line[i] == ' ')
            {
                on_word = 0;
                char *word = (char *)malloc(sizeof(char) * (word_length + 1));
                memcpy(word, line + i - word_length, word_length);
                arguments[argument_count] = strdup(word);
                argument_count++;
                word_length = 0;
            }
            else if (line[i] == '<' || line[i] == '>')
            {
                int k = i + 1;
                while (k < strlen(line) && line[k] != '\n')
                {
                    if (line[k] != ' ')
                    {
                        file_flag = 1;
                        file_name_length++;
                        if (k + 1 == strlen(line))
                        {
                            break;
                        }
                    }
                    else if (file_flag == 1 && line[k] == ' ')
                    {
                        break;
                    }
                    k++;
                }
                file_name = (char *)malloc(sizeof(char) * (file_name_length + 1));
                memcpy(file_name, line + k - file_name_length, file_name_length);
                break;
            }
            i++;
        }
        arguments[argument_count] = NULL;
        pipe(pipes[command_number]);
        if (arguments[0][0] == 'w' && arguments[0][1] == 'a')
        {
            while (wait(NULL) != -1)
                ;
            for (int i = 0; i < MAX_THREADS; i++)
            {
                if (thread_status[i] == 1)
                {
                    pthread_join(threads[i], NULL);
                    thread_status[i] = -1;
                }
            }
        }
        else
        {
            int child_pid = fork();
            if (child_pid < 0)
            {
                exit(1);
            }
            else if (child_pid == 0)
            {
                if (redirect_symbol == '-')
                {
                    dup2(pipes[command_number][1], STDOUT_FILENO);
                }
                else if (redirect_symbol == '<')
                {
                    close(STDIN_FILENO);
                    int file_descriptor = open(file_name, O_RDONLY);
                    dup2(pipes[command_number][1], STDOUT_FILENO);
                }
                else
                {
                    close(STDOUT_FILENO);
                    open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                }
                execvp(arguments[0], arguments);
            }
            else
            {
                if (redirect_symbol != '>')
                {
                    pthread_create(&threads[thread_count], NULL, thread_func1, (void *)command_number);
                    thread_status[thread_count] = 1;
                }
                if (background_flag == 'n')
                {
                    waitpid(child_pid, NULL, 0);
                    if (redirect_symbol != '>')
                    {
                        pthread_join(threads[thread_count], NULL);
                        thread_status[thread_count] = -1;
                    }
                }
                thread_count++;
            }
        }
        command_number++;
    }
    while (wait(NULL) != -1)
        ;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        if (thread_status[i] == 1)
        {
            pthread_join(threads[i], NULL);
            thread_status[i] = -1;
        }
    }
    return 0;
}
