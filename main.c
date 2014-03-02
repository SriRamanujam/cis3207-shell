#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_SIZE 1024

void parse_input(char *input, int forking);
char **str_split(char *a_str, const char a_delim);
char **build_argv(char *input);
void process_command(char *input);
void process_input(char *input);
void process_output(char *input);
void process_pipe(char *input);
void trim_whitespace(char *in, char *out);


void process_input(char *input)
{
    char **splitInArgs = str_split(input, '<'); // str_split is internal function!
    trim_whitespace(splitInArgs[1], splitInArgs[1]);
    int newstdin = open(splitInArgs[1], O_RDONLY);
    char **argv = build_argv(splitInArgs[0]); // build_argv is internal function!
    if (fork() == 0)
    {
        dup2(newstdin, 0);
        execvp(argv[0], argv);
    }
    else
    {
        close(newstdin);
        int status = 0;
        wait(&status);
    }
    return;
}

void process_output(char *input)
{
    char **splitOutArgs = str_split(input, '>');
    trim_whitespace(splitOutArgs[1], splitOutArgs[1]);
    int newstdout = open(splitOutArgs[1], O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    char **argv = build_argv(splitOutArgs[0]);
    if (fork() == 0)
    {
        dup2(newstdout, 1);
        execvp(argv[0], argv);
    }
    else
    {
        close(newstdout);
        int status = 0;
        wait(&status);
    }
    return;
}

void process_pipe(char *input)
{
    char **splitPipeArgs = str_split(input, '|');
    char **left = build_argv(splitPipeArgs[0]);
    char **right = build_argv(splitPipeArgs[1]);

    /* Pipe initialization */
    int thePipe[2];
    pipe(thePipe);

    /* Forking for the pipe */
    int PID = fork();
    if (PID > 0)
    {
        /* This is the parent process after forking for left-side child */
        close(thePipe[1]); // close pipe in before forking to prevent hangs

        /* Executing fork for right-side child process */
        int PID2 = fork();
        if (PID2 > 0)
        {
            /* This is the parent process after forking for right-side child */
            close(thePipe[0]); // close pipe out because it's no longer needed

            int status2 = 0;
            wait(&status2); // wait on right-side child to exit
        }
        else
        {
            /* This is the right-side child's process */
            dup2(thePipe[0], 0); // remake stdin to pipe
            execvp(right[0], right);
        }
        close(thePipe[0]); // after right-side child exits, close pipe out for parent

        int status = 0;
        wait(&status); // wait on left-side child to exit
    }
    else
    {
        /* This is the left-side child's process */
        close(thePipe[0]); // close pipe out since it is not needed here
        dup2(thePipe[1], 1); // remake stdout to pipe in
        execvp(left[0], left);
    }
    return;
}

void process_command(char *input)
{
    char **argv = build_argv(input); // build_argv is internal function!
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


void trim_whitespace(char *in, char *out)
{
    char *end = out;
    char c;

    /* Trim leading whitespace */
    while (*in && isspace(*in))
    {
        ++in;
    }

    /* Copy the rest of the string and remember last non-whitespace char */
    while (*in)
    {
        c = *(out++) = *(in++); // copy character over

        if (!isspace(c)) // if not a whitespace, it COULD be the last character, so set out just in case
        {
            end = out;
        }
    }

    *end = '\0'; // once out of the loop, set terminator
}

void parse_input(char *input, int forking)
{

    size_t len = strlen(input) - 1;
    if (input[len] == '\n')
    {
        input[len] = '\0';
    }
    if (strstr(input, "<") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                process_input(input);
            }
            else
            {
                return;
            }
        }
        else
        {
            process_input(input);
        }
    }
    else if (strstr(input, ">") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                process_output(input);
            }
            else
            {
                return;
            }
        }
        else
        {
            process_output(input);
        }
    }
    else if (strstr(input, "|") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                process_pipe(input);
            }
            else
            {
                return;
            }
        }
        else
        {
            process_pipe(input);
        }
    }
    else
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                process_command(input);
            }
            else
            {
                return;
            }
        }
        else
        {
            process_command(input);
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

    while (token != NULL)
    {
        argv[index] = (char*) malloc(sizeof(char) * strlen(token) + 1); // malloc() appropriate size block for token string, and put ptr in argv[index]
        strncpy(argv[index], token, strlen(token) + 1); // copy token into block pointed to by argv[index]
        index++; // increment index by 1 in preparation for next iteration
        token = strtok(NULL, " "); // move ptr ahead to next token
        argv = (char**) realloc(argv, sizeof(char*) * (index+1)); // change size of argv for next iteration
    }
    argv[index] = NULL; // null-terminate argv like execvp() expects
    free(copy);
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
            output[len-2] = '\0';
        }
        parse_input(output, forking);
    }
    printf("Program done!");
    return 0;
}
