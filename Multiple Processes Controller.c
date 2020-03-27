#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
int pr_limit, pr_count, status, systemerr, waitNum, pid;
char * string, line[100], *Excargv[4];

//decrease process count when the child died
void waitTermination(int status, int childPid){
  if (WIFEXITED (status)) {
    pr_count--;
    printf("Child with pid %d has been normally terminated with status: %d\n", childPid, WIFEXITED(status));
  }
  if (WIFSIGNALED(status)) {
    pr_count--;
    printf("Child with pid %d has been killed by signal with status: %d\n", childPid, WIFSIGNALED(status));
  }
  if(WIFSTOPPED(status)) {
    pr_count--;
    printf("Child with pid %d has been stopped by signal with status: %d\n", childPid, WIFSTOPPED(status));
  }
  if (WIFCONTINUED(status)) {
    pr_count--;
    printf("Child with pid %d continue with status: %d\n", childPid, WIFCONTINUED(status));
  }
}


int main(int argc, char* argv[]) {
  if (argc!=2) {
    printf ("Missing argument or argument too much.\n");
    exit(1);
  }

  pr_limit = atoi(argv[1]);

  //can not go over 10 processes
  if (pr_limit==0 || pr_limit>100) {
    printf("Invalid input.\n");
    exit(1);
  }

  printf("Please enter the command.\n");

  while ((string=fgets(line, 100, stdin)) !=NULL) {
    //when entering a new line without content
    if (string[0]=='\n') {
      printf("Please re-enter command.\n");
      continue;
    }
    //display the string content
    printf("You have enter: %s\n", string);
    //creating processes
    pid= fork();

    //parent process
    if(pid>0) {

      //check if child are dead or not
      if (pr_count!=0) {
        for (int i=0; i<pr_count; i++){
          if((pid=waitpid(-1, &status, WNOHANG))>0)
            waitTermination(status, pid);
        }
      }
      pr_count++;

      //check for current process count
      printf("Current process count= %d\n", pr_count);
      printf("\n");

      //when reach the pr limit
      if (pr_count == pr_limit) {
        printf("You have reached the limit, waiting for the child to terminated.\n");
        printf("\n");
        //wait for a child to finish
        pid = wait(&status);
        if (pid==-1)
          perror("Wait error");
        waitTermination(status, pid);
      }
    }

    else if(!pid) {
      //child process
      Excargv[0]="sh";
      Excargv[1]="-c";
      Excargv[2]=string;
      Excargv[3]=NULL;
      systemerr=execv("/bin/sh", Excargv);
      if(systemerr==-1) {
        perror("System error");
      }
      exit(3);
    }

    else if(pid==-1) {
      //error
      perror("Fork error");
    }
  }

  //if the process count is already 0 then just exit
  if (pr_count==0) {
    printf("\n");
    printf("You have exited the program.\n");
    exit(0);
  }

  //check if the child process is dead or not
  if ((pid=waitpid(-1, &status, WNOHANG))>0) {
    waitTermination(status, pid);
    //if only 1 process then just exit
    if (pr_count==0) {
      printf("\n");
      printf("You have exited the program.\n");
      exit(0);
    }
    //otherwise wait for the process to be complete
    else {
      printf("\n");
      printf("You have exited the program, but still have to wait for %d process to complete!\n", pr_count);
      while ((pid=wait(&status))>0){
        waitTermination(status, pid);
      }
      printf("\n");
      printf("You have exited the program.\n");
      exit(0);
    }
  }
  //if the child process is still running then wait for it to complete
  else {
    printf("\n");
    printf("You have exited the program, but still have to wait for %d process to complete!\n", pr_count);
    while ((pid=wait(&status))>0){
      waitTermination(status, pid);
    }
    printf("\n");
    printf("You have exited the program.\n");
    exit(0);
  }
    return 0;
}
