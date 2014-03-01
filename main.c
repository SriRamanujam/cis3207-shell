#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>

#define MAX_SIZE 1024

void parse_input(char *input, int forking);
char **str_split(char *a_str, const char a_delim);
char **build_argv(char *input);

void parse_input(char *input, int forking)
{

    size_t len = strlen(input) - 1;
    if (input[len] == '\n')
    {
        input[len] = '\0';
    }
    if (strstr(input, "<") != NULL)
    {
        printf("found a <\n");
        char **splitInArgs = str_split(input, '<'); // str_split is internal function!
        int newstdin = open(splitInArgs[1], O_RDONLY);
        dup2(newstdin, 0);
        char **argv = build_argv(splitInArgs[0]); // build_argv is internal function!
        if (forking == 0)
        {
            execvp(argv[0], argv);
        }
        else
        {
            if (fork() == 0)
            {
                execvp(argv[0], argv);
            }
            else
            {
                int status = 0;
                wait(&status);
            }
        }
    }
    else if (strstr(input, ">") != NULL)
    {
        printf("found a >\n");
        char **splitOutArgs = str_split(input, '>');
        int newstdout = open(splitOutArgs[1], O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
        dup2(newstdout, 1);
        char **argv = build_argv(splitOutArgs[0]);
        if (forking == 0)
        {
            execvp(argv[0], argv);
        }
        else
        {
            if (fork() == 0)
            {
                execvp(argv[0], argv);
            }
            else
            {
                int status = 0;
                wait(&status);
            }
        }
    }
    else if (strstr(input, "|") != NULL) {
         printf("found a |\n");
         char **splitPipeArgs = str_split(input, '|');
         char **left = build_argv(splitPipeArgs[0]);
         char **right = build_argv(splitPipeArgs[1]);
         /* Pipe initialization */
         int thePipe[2];
         pipe(thePipe);
         /* Forking for the pipe */
         int pid = fork();
         if (pid > 0)
         {
             dup2(thePipe[1], 1);
             execvp(left[0], left);
         }
         else if (pid == 0)
         {
             dup2(thePipe[0], 0);
             execvp(right[0], right);
         }
         else
         {
             printf("Something has gone very wrong\n");
         }
    }
    else
    {
        char **argv = build_argv((char*) input); // build_argv is internal function!
        if (fork() == 0)
        {
            execvp(argv[0], argv);
        }
        else
        {
            int status = 0;
            wait(&status);
        }
    }
    return;
}

/* This code works fine */
char **build_argv(char *input)
{
    int index = 0;

    char **argv = (char**) malloc(sizeof(char*));
    char *copy = (char*) malloc(sizeof(char) * (strlen(input) + 1));

    strncpy(copy, input, strlen(input) + 1);
    char *token = strtok(copy, " ");

    while (token != NULL) {
        argv[index] = (char*) malloc(sizeof(char) * strlen(token) + 1);
        strncpy(argv[index], token, strlen(token) + 1);
        index++;
        token = strtok(NULL, " ");
        argv = (char**) realloc(argv, sizeof(char*) * (index+1));
    }
    argv[index] = NULL;
    return argv;
}

/* This hasn't been tested, look into it */
char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *lastComma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            lastComma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += lastComma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**) malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int main(void)
{
    int forking;
    while(1)
    {
        forking = 0;
        printf("shell> ");
        char *input = (char*) malloc(sizeof(char) * MAX_SIZE);
        char *output = fgets(input, MAX_SIZE, stdin);
        if (feof(stdin) || (strcmp(output, "exit\n") == 0))
        {
            printf("Exiting!\n");
            free(input);
            exit(0);
        }
        int len = strlen(output);
        const char *lastTwoChars = &output[len-2];
        if (strcmp(lastTwoChars, "&\n") == 0)
        {
            forking = 1;
        }
        parse_input(output, forking);
        //printf(output);
    }
    printf("Program done!");
    return 0;
}
