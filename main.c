#include <stdio.h>
#include <unistd.h>

#define MAX_SIZE 1024

int main(void)
{
    while(1) {
        printf("shell> ");
        char* input = malloc(sizeof(char) * MAX_SIZE);
        char* output = fgets(input, MAX_SIZE, stdin);
        printf(output);
    }
}
