/* ************************************************************************** *
 * Projekt: POS #2 - Shell
 * Autor:   Daniel Klimaj; xklima22@stud.fit.vutbr.cz
 * Datum:   2017-04-09
 * Subor:   shell.c
 * ************************************************************************** */

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "utils.h"
#include "monitor.h"
#include "str_vector.h"
#include "process.h"

#define STDIN  0
#define STDOUT 1

#define RUN  100
#define READ 101

struct Monitor monitor;

void init_sigaction(struct sigaction *sa, int signum, void (*handler)(int));
void on_sigint(int signum);
void on_sigchld(int signum);
void *th_rt_read(void *t);
void *th_rt_run(void *t);

int main()
{
  pthread_t th_read, th_run;
  struct sigaction sa_sigint, sa_sigchld;

  if(!mt_init(&monitor))
    return -1;

  init_sigaction(&sa_sigint, SIGINT, on_sigint);
  init_sigaction(&sa_sigchld, SIGCHLD, on_sigchld);

  pthread_create(&th_read, NULL, th_rt_read, (void *) &monitor);
  pthread_create(&th_run, NULL, th_rt_run, (void *) &monitor);

  pthread_join(th_read, NULL);
  pthread_join(th_run, NULL);

  mt_destroy(&monitor);

  return 0;
}

void init_sigaction(struct sigaction *sa, int signum, void (*handler)(int))
{
  sa->sa_handler = handler;
  sa->sa_flags = 0;
  sigemptyset(&sa->sa_mask);
  sigaction(signum, sa, NULL);
}

void on_sigint(int signum)
{
  mt_kill_running_pid(&monitor, signum);
}

void on_sigchld(int __attribute__((unused)) signum)
{
  printf("SIGCHLD\n");
}

void *th_rt_read(void *t)
{
  struct Monitor *m = (struct Monitor *) t;
  int rc;
  char buff[MT_CMDLEN+1]; // +1 aby sa zaistilo spravne ukoncenie retazca

  while(mt_running(m))
  {
    printf("$ ");
    fflush(stdout);
    memset(buff, 0, MT_CMDLEN+1);
    rc = read(STDIN, buff, MT_CMDLEN);
    if(rc == MT_CMDLEN && buff[rc-1] != '\n') // Maximum je 512 znakov + Enter
    {
      fprintf(stderr, "Chyba: Prikaz prilis dlhy\n");
      while(getchar() != '\n') {} // precitaj prikaz az do konca
      buff[0] = '\0';
    }
    buff[rc-1] = '\0';

    if(strcmp(buff, "exit") == 0)
    {
      mt_shutdown(m);
      mt_signal(m, READ);
      break;
    }
    else if(strcmp(buff, "") == 0)
    {
      continue;
    }
    else
    {
      mt_set_cmd(m, str_trim(buff));
      mt_signal(m, READ);
      mt_wait(m, READ);
    }
  }
  pthread_exit(NULL);
}

void *th_rt_run(void *t)
{
  struct Monitor *m = (struct Monitor *) t;
  struct Process p;

  while(mt_running(m))
  {
    mt_wait(m, RUN);

    if(!mt_running(m))
      break;

    p_init(&p, mt_get_cmd(m));
    p_print(&p);
    printf("Valid process?: %s\n", str_bool(p_is_valid(&p)));
    p_destroy(&p);
    mt_signal(m, RUN);
  }
  pthread_exit(NULL);
}
