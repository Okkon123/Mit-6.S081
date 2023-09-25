#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, char *argv[])
{
    char buf[2048];
    char *bufStart = buf, *bufEnd = buf;
    char *args[32];
    char **argsStart = args;
    for (int i = 1; i < argc; i++)
    {
        *argsStart = argv[i];
        argsStart++;
    }
    char **argsCur = argsStart;
    while (read(0, bufEnd, 1) != 0)
    {
        if (*bufEnd == ' ')
        {
            *bufEnd = '\0';
            *argsCur = bufStart;
            argsCur++;
            bufStart = bufEnd + 1;
        }
        else if (*bufEnd == '\n')
        {
            *bufEnd = '\0';
            *argsCur = bufStart;
            argsCur++;
            *argsCur = 0;
            if (fork() == 0)
            {
                exec(argv[1], args);
                exit(0);
            }
            else
            {
                wait(0);
            }
            argsCur = argsStart;
            bufStart = buf;
            bufEnd = buf;
        }
        bufEnd++;
    }
    if (argsCur != argsStart)
    {
        *bufEnd = '\0';
        *argsCur = bufStart;
        argsCur++;
        *argsCur = 0;
        if (fork() == 0)
        {
            exec(argv[1], args);
        }
        else
        {
            wait(0);
        }
    }
    exit(0);
}
