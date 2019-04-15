#include <sys/types.h>
#include <stdio.h>    //printf
#include <stdlib.h>   //exit
#include <unistd.h>   //fork, chdir, sysconf
#include <signal.h>   //signal
#include <sys/stat.h> //umask
#include <syslog.h>   //syslog, openlog, closelog


void write_to_file(char *filename, char *mes)// запись в файл
{
	FILE * f = fopen(filename, "aw+");
	if(!f)
	{
		printf("fopen error! \n" );
	}
	else if(mes=="PID")
	{
		fprintf(f,"pid = %d \n",getpid());
		fclose(f);
	}
	else
	{
		fprintf(f,"%s \n",mes);
	  fclose(f);
	}
}

static int sign_flag=-1;

void myHandler(int signal) // обработчик сигналов
{
	if(signal==SIGHUP)
	{
		sign_flag=1;
		//write_to_file("/tmp/LOGFILE.log","Terminal disconnection signal is received ");
	}
	else if(signal==SIGTERM)
	{
		sign_flag=2;
		//write_to_file("/tmp/LOGFILE.log","Termination signal is received ");
	}
}


static void skeleton_daemon()
{
    pid_t pid;

    // создание нового процесса
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    // завершаем родительский
    if (pid > 0)
        exit(EXIT_SUCCESS);
    // ребенок становится сессионым лидером
    if (setsid() < 0)
        exit(EXIT_FAILURE);

		signal(SIGINT, SIG_IGN);// игнорирует сигнал прерывания (ctrl+c
    signal(SIGTSTP, SIG_IGN);//игнорирует сигнал приостановки (ctrl+z)

		signal(SIGHUP, myHandler); //обработка сигнала  об отключении терминала,	упрвляющего процессом
		signal(SIGTERM, myHandler); // обработка сигнала отлавлимаего завершения (kill())


    umask(0);
		//установка в качестве рабочего каталога корневого каталога
    chdir("/");

    // закрываем дескрипторы открытых файлов
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
}

int main()
{
    skeleton_daemon();
		write_to_file("/tmp/LOGFILE.log", "Demon is STARTED");
		write_to_file("/tmp/PIDFILE.pid","PID");
    while (1)
    {
				if(sign_flag!=-1)
				{
					if(sign_flag==1)
					{
						write_to_file("/tmp/LOGFILE.log","Terminal disconnection signal is received ");
						sign_flag=-1;
					}
					else if(sign_flag==2)
					{
						write_to_file("/tmp/LOGFILE.log","Termination signal is received ");
						sign_flag=-1;
						exit(EXIT_SUCCESS);
					}
				}

    	sleep (60); // $ cat /tmp/PIDFILE.pid
				   // $ kill -1 <pid> для SIGHUP или  $ kill <pid> для SIGTERM

    }

		write_to_file("/tmp/LOGFILE.log", "Demon is TERMINATED");

    return EXIT_SUCCESS;
}
