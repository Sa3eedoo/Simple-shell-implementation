#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CHAR 100
#define MAX_WORD 10

// one command may contain MAX CHAR = 100 character
char command[MAX_CHAR];

// one command can have 10 word
char *args[MAX_WORD];

void print_blue()
{
    printf("\033[0;34m");
}

void print_green()
{
    printf("\033[0;32m");
}

void print_red()
{
    printf("\033[1;31m");
}

void print_reset()
{
    printf("\033[0m");
}

void get_command()
{
    // get username, device name and dir
    char *username = getenv("USER");
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char device_name[100];
    char dir[1024] = "";

    char *ptr;
    ptr = strtok(cwd, "/");
    int i = 0;
    while (ptr != NULL)
    {
        if (i == 1)
        {
            strcpy(device_name, ptr);
        }
        if (i > 1)
        {
            strcat(dir, "/");
            strcat(dir, ptr);
        }
        ptr = strtok(NULL, "/");
        i++;
    }

    print_green();
    printf("%s@%s", username, device_name);
    print_reset();
    printf(":");
    print_blue();
    printf("~%s", dir);
    print_reset();
    printf("$ ");

    // get command from user
    fgets(command, MAX_CHAR, stdin);

    // remove unnecessary new line
    if ((strlen(command) > 0) && (command[strlen(command) - 1] == '\n'))
        command[strlen(command) - 1] = '\0';
}

int convert_command()
{
    // split command to arguments
    char *ptr;
    ptr = strtok(command, " ");
    int i = 0;
    while (ptr != NULL)
    {
        args[i] = ptr;
        ptr = strtok(NULL, " ");
        i++;
    }

    // check for &
    if (!strcmp("&", args[i - 1]))
    {
        args[i - 1] = NULL;
        args[i] = "NOT NULL";
    }
    else
    {
        args[i] = NULL;
    }
    return i;
}

void log_file()
{
    FILE *fptr;
    fptr = fopen("log.txt", "a");
    if (fptr == NULL)
    {
        print_red();
        printf("ERROR: Log file not found or being used.");
        print_reset();
    }
    else
        fprintf(fptr, "Child process was terminated.\n");
    fclose(fptr);
}

int main()
{
    // signal handler
    signal(SIGCHLD, log_file);

    while (1)
    {
        // get command from user
        get_command();

        // check for exit
        if (!strcmp("exit", command))
            break;

        // skip empty command
        if (!strcmp("", command))
            continue;

        // split command in args[]
        int i = convert_command();

        // check for cd
        if (!strcmp("cd", args[0]))
        {
            int flag = chdir(args[1]);
            if (flag < 0)
            {
                print_red();
                printf("ERROR: No such file or directory\n");
                print_reset();
            }
            continue;
        }

        // forking and execution
        pid_t pid = fork();
        // if forking failed
        if (pid == -1)
        {
            print_red();
            printf("ERROR: Child creation failed\n");
            print_reset();
        }

        // child process executes
        else if (pid == 0)
        {
            int flag = execvp(args[0], args);
            if (flag < 0)
            {
                print_red();
                printf("ERROR: Exec failed\n");
                print_reset();
                exit(0);
            }
        }

        // parent process waits if there is not &
        else
        {
            if (args[i] == NULL)
                waitpid(pid, NULL, 0);
        }
    }
    return 0;
}