#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_SIZE 1024

int main(void)
{
    int IS_FORKING = 0;
    while(1) {
        printf("shell> ");
        char* input = malloc(sizeof(char) * MAX_SIZE);
        const char* output = fgets(input, MAX_SIZE, stdin);
        int len = strlen(output);
        const char* lastTwoChars = &output[len-2];
        if (feof(stdin) || (strcmp(output, "exit\n") == 0)) {
            printf("Exiting!\n");
            exit(0);
        }
        if (strcmp(lastTwoChars, "&\n") == 0) {
            IS_FORKING = 1;
        }
        printf("%d\n", IS_FORKING); // replace with parse_input call
    }
}
