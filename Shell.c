#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>


FILE *batchFile;
FILE *logFile;
int numOfArgs=0;
int background=0;
char newStr1[600];
char newStr2[600];
char history[1000][10000];
int numberOfPaths=0;
int numberOfCommands=0;
static char* paths[512];
char**command;
int command_Found=0;
int numberOfChildFinished=0;
char path[4096];



void split(char *str)
{
//    printf("str In splite = |%s|\n",str);
    numOfArgs=0;
    char * spliter;
    char* commandSplit[512];
    spliter = strtok (str," ");
    int i=0;
    while (spliter != NULL)
    {
//        if(spliter=='\n') continue;
        commandSplit[i++]=spliter;
        spliter = strtok (NULL, " \n");
        numOfArgs++;
    }
    command=commandSplit;
//    printf("command[0] = |%d|\n",command[0]);
//    printf("command[1] = |%d|\n",command[1]);
}


int executeCommand(char*str)
{
//    printf("str In execute = |%s|\n",str);

    command_Found=0;
    strcpy(history[numberOfCommands],str);
    numberOfCommands++;
    int i=0;
    background=0;
    command_Found=0;
    for(i=0; i<strlen(str); i++)
    {
        if(str[i]=='&')
        {
            background=1;
            str[i]=' ';
        }

        //Backspace \a, Form feed \f, Carriage return \r, Horizontal tab \t, Vertical tab \v
        if(str[i]=='\t'||str[i]=='\r'||str[i]=='\a'||str[i]=='\b'||str[i]=='\f'||str[i]=='\v')
        {
            str[i]=' ';
        }
    }

    if(strlen(str)>=512)
    {
        printf("Invalid too large command\n");
        return 2; //continue
    }
    if(strcmp(str,"\n")==0)return 2; //continue
    int foundEqual=0;
    for(i=0; i<strlen(str); i++)
    {
        if(str[i]=='=')
        {
            foundEqual=1;
            break;
        }

    }

    char * spliter;
    char* expression[512];
    if(foundEqual)
    {
        spliter = strtok (str," =\n");
        int i=0;
        while (spliter != NULL)
        {
//            if(spliter=='\n') continue;
            expression[i++]=spliter;
            spliter = strtok (NULL, " \n=");
        }
        setenv(expression[0],expression[1],1);
        return 2; //continue
    }

    numOfArgs=1;
    int i2=0;
    int length=strlen(str);
    while(str[i2]==' ')i2++;
    while(i2<length)
    {
        if(str[i2]==' ' && str[i2+1]!=' ' && str[i2]!='\0' && i2+1<length && str[i2+1]!='\n')
        {
            numOfArgs++;
        }
        i2++;
    }

    split(str);
    if(command[0][0]=='#') return 2; //continue

    char type[50];
    strcpy(type,command[0]);
    int j=0;
    while(j<50)
    {
        if(type[j]=='\n')
        {
            type[j]='\0';
            break;
        }
        j++;
    }

    command[numOfArgs]=NULL;
    pid_t pid;
    if(strcmp("exit",type)==0 || strcmp("Ctrl-D",type)==0)
    {
        printf("Goodbye!\n");
        return 0;//terminate
    }
    else if(strcmp(type,"history")==0)
    {
        i=0;
        for(; i<numberOfCommands; i++)
        {
            printf("%s",history[i]);
        }
        return 2; //continue
    }
    char *program=command[0];
    j=0;
    while(j<50)
    {
        if(program[j]=='\n')
        {
            program[j]='\0';
            break;
        }
        j++;
    }

    if(strcmp(program,"cd")!=0)
    {
        for(i=0; i<numberOfPaths; i++)
        {
//            printf("paths[%d] = |%s|\n",i,paths[i]);
            strcpy(type,paths[i]);
            strcat(type,"/");
            strcat(type,command[0]);
            int j=0;
            while(j<50)
            {
                if(type[j]=='\n')
                {
                    type[j]='\0';
                    break;
                }
                j++;
            }
            int result=access(type,X_OK);
            if(result==0)
            {
                pid=fork();
                command_Found=1;
                break;
            }
        }
        if(command[0][0]=='/')
        {
            strcpy(type,command[0]);
            command_Found=1;
            pid=fork();
        }
    }
    else if(strcmp(program,"cd")==0)
    {

        chdir(command[1]);
        return 22; //continue
    }
    if(strcmp(command[0],"echo")==0)
    {
        command_Found=1;
        int i=0;
        int j=0;
        while(i<strlen(command[1]))
        {

            char c=command[1][i];
            if(c!='"' && c!='\'')
                newStr1[j++]=c;
            i++;
        }
        newStr1[i++]='\0';
        command[1]=newStr1;


        i=0;
        j=0;
        while(i<strlen(command[numOfArgs-1]))
        {
            char c=command[numOfArgs-1][i];
            if(c!='"' && c!='\'')
                newStr2[j++]=c;
            i++;
        }
        newStr2[i++]='\0';
        command[numOfArgs-1]=newStr2;

        i=1;
        for(; i<numOfArgs; i++)
        {
            if(command[i][0]=='$')
            {
                char *env;

                int n=0;
                char * spliter;
                char* splited[50];
                spliter = strtok (command[i]," $\n");
                int i=0;
                while (spliter != NULL)
                {
                    if(spliter=='\n') continue;
                    splited[i++]=spliter;
                    spliter = strtok (NULL, " $\n");
                    n++;
                }
                env=splited[0];
                command[i]=getenv(env);
//                printf("env = |%s|\n",env);
//                printf("command[%d] = |%s|\n",i,command[i]);
            }
        }
    }
    int status;
//    printf("commandFound = %d\n",command_Found);

    if(command_Found)
    {
        if(pid!=0)
        {
            if(!background)
            {
                waitpid(pid, &status, 0);
            }
        }
        else
        {
//        printf("type = |%s|\n",type);
//        printf("command[0] = |%d|\n",command[0]);
//        printf("command[1] = |%d|\n",command[1]);
            if(type[strlen(type)-1]=='/'||type[strlen(type)-1]=='\\')return 0;
            execv(type, command);
            perror("execv");
            return 2;
        }
    }
    else
    {
        fprintf(stderr, "Command not found!\n");
    }
}

void getEnvPaths()
{
    strcpy(path,getenv("PATH"));
//    if(path)
//        printf("The current path is: %s\n", path);
    char * spliter;
    spliter = strtok (path,":");
    numberOfPaths=0;
    while (spliter != NULL)
    {
//        if(()spliter=='\n') continue;
        paths[numberOfPaths++]=spliter;
        spliter = strtok (NULL, ":");
    }
//    int i=0;
//    for(; i<numberOfPaths; i++)
//    {
//        printf("path %d = |%s|\n",i,paths[i]);
//    }
}


void runInteractiveMode()
{
    while(1)
    {
        printf("Shell>");
        char str[600];
        if (fgets(str, sizeof(str), stdin) == NULL||strcmp(str, "exit\n")==0)
            exit(0);
        background=0;
        int result=executeCommand(&str);
        if(result==2)
        {
            continue;
        }
        else if(result==0)
        {
            return;
        }
    }
}


void readFromFile(char*pathOfFile)
{
    batchFile=fopen(pathOfFile,"r");
    if (batchFile == NULL)
    {
        printf("Can not open this file!\n");
        return -1;
    }

    char str [600]; /* or other suitable maximum line size */
    while ( fgets ( str, sizeof str, batchFile ) != NULL && !feof(batchFile)) /* read a line */
    {
        if (str == NULL||strcmp(str, "exit\n")==0 || str ==EOF)
            return 0;
        printf("%s",str);
        int result=executeCommand(&str);
        if(result==2)
        {
            continue;
        }
        else if(result==0)
        {
            return;
        }
    }
}



void createLogFile()
{
    logFile = fopen("log.txt", "w");
}

void addLog(int signal)
{
//    logFile = fopen("log.txt", "a");
    fprintf(logFile,"A child process %d is finished.\n",numberOfChildFinished);
    numberOfChildFinished++;
//    fclose(logFile);
}


int main(int argc,char*argv[])
{
    signal (SIGCHLD, addLog);
//    argc=2;
//    argv[1]="/home/remon/Desktop/testLab1.sh";
    createLogFile();

    numberOfCommands=0;
    getEnvPaths();
    if(argc>1)
    {
        readFromFile(argv[1]);
    }
    else
    {
        runInteractiveMode();
    }
    fclose(logFile);
    return 0;
}

