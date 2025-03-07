#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC "/bin/bash"
#define FIND_EXEC "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC "/bin/grep"
#define SORT_EXEC "/bin/sort"
#define HEAD_EXEC "/usr/bin/head"

int main(int argc, char *argv[])
{
    int status;
    pid_t pid_1, pid_2, pid_3, pid_4;

    // creating 3 pipes for IPC
    int p1[2], p2[2], p3[2]; 
    pipe(p1);
    pipe(p2);
    pipe(p3);

    // Checking if input parameters DIR, STR, and NUM_FILES are correct
    if (argc != 4)
    {
        printf("usage: finder DIR STR NUM_FILES\n");
        exit(0);
    }

    // STEP 1: Fork for the "find" command
    pid_1 = fork();
    if (pid_1 == 0)
    {
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s %s -name '*.[ch]'", FIND_EXEC, argv[1]);
        dup2(p1[1], STDOUT_FILENO);
        close(p1[0]);
        close(p2[0]);
        close(p2[1]);
        close(p3[0]);
        close(p3[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, NULL};
        execv(BASH_EXEC, myArgs);
        perror("Exec failed for find");
        exit(EXIT_FAILURE);
    }

    // STEP 2: Fork for the "grep" command
    pid_2 = fork();
    if (pid_2 == 0)
    {
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]);
        dup2(p1[0], STDIN_FILENO);
        dup2(p2[1], STDOUT_FILENO);
        close(p1[1]);
        close(p2[0]);
        close(p3[0]);
        close(p3[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, NULL};
        execv(BASH_EXEC, myArgs);
        perror("Exec failed for grep");
        exit(EXIT_FAILURE);
    }

    // STEP 3: Fork for the "sort" command
    pid_3 = fork();
    if (pid_3 == 0)
    {
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC);
        dup2(p2[0], STDIN_FILENO);
        dup2(p3[1], STDOUT_FILENO);
        close(p1[0]);
        close(p1[1]);
        close(p2[1]);
        close(p3[0]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, NULL};
        execv(BASH_EXEC, myArgs);
        perror("Exec failed for sort");
        exit(EXIT_FAILURE);
    }

    // STEP 4: Fork for the "head" command
    pid_4 = fork();
    if (pid_4 == 0)
    {
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s --lines=%s", HEAD_EXEC, argv[3]);
        dup2(p3[0], STDIN_FILENO);
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        close(p3[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, NULL};
        execv(BASH_EXEC, myArgs);
        perror("Exec failed for head");
        exit(EXIT_FAILURE);
    }

    // Close all pipe ends in the parent process
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    close(p3[0]);
    close(p3[1]);

    // Wait for all child processes to finish
    if ((waitpid(pid_1, &status, 0)) == -1)
    {
        perror("Error waiting for process 1");
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_2, &status, 0)) == -1)
    {
        perror("Error waiting for process 2");
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_3, &status, 0)) == -1)
    {
        perror("Error waiting for process 3");
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_4, &status, 0)) == -1)
    {
        perror("Error waiting for process 4");
        return EXIT_FAILURE;
    }

    return 0;
}
