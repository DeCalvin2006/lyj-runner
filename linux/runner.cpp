#include <climits>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <linux/seccomp.h>
#include <linux/sysctl.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
void setup_secconf() {
  printf("Install seccomp\n");
  prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
}
// runner <program> <file> <timelimit> <memorylimit> <inputfile> <outputfile>
int pid;
void cleanUp(int sig) {
  kill(pid, SIGKILL);
  exit(0);
}
enum { EXIT_SUC = 0, EXIT_RE = 2, EXIT_TLE = 2, EXIT_MLE = 3 };
int main(int argc, char **argv) {
  // init
  int timeLimit = 0, memoryLimit = 0;
  if (argv[2]) {
    timeLimit = (atoi(argv[2]) - 1) / 1000 + 1;
  }
  if (argv[3]) {
    memoryLimit = atoi(argv[3]);
    memoryLimit *= 1024 * 1024;
  }
  cout << timeLimit << endl;
  cout << memoryLimit << endl;

  pid = fork();
  // fork
  switch (pid) {
  case -1:
    return -1;
  case 0: {
    rlimit memlim, stalim, timlim;
    if (!memoryLimit) {
      memoryLimit = INT_MAX - 1;
    }
    memlim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
    stalim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
    if (!timeLimit) {
      timeLimit = 10;
    }
    timlim = (rlimit){(rlim_t)timeLimit / 1000 - 1, (rlim_t)timeLimit / 1000};
    setrlimit(RLIMIT_AS, &memlim);
    setrlimit(RLIMIT_STACK, &stalim);
    setrlimit(RLIMIT_CPU, &timlim);
    if (argv[4])
      freopen(argv[4], "r", stdin);
    if (argv[5])
      freopen(argv[5], "w", stdout);
    if (argv[6])
      freopen(argv[6], "w", stderr);
    if (execlp("bash", "bash", "-c", argv[1], NULL) == -1)
      return 1;
  }
  default: {
    signal(SIGABRT, cleanUp);
    signal(SIGINT, cleanUp);
    signal(SIGTERM, cleanUp);
    // get usage
    int status;
    rusage usage;
    if (wait3(&status, 0, &usage) == -1) {
      cout << "WAITING_ERROR" << endl;
      return 1;
    }
    if (WIFEXITED(status)) {
      cout << "EXITED" << endl;
      if (WEXITSTATUS(status) == 1) {
        cout << "RE" << endl;
        return EXIT_RE;
      }
      cout << (usage.ru_utime.tv_sec * 1000) << endl;
      cout << (usage.ru_maxrss / 1024) << endl;
      if (WEXITSTATUS(status) != 0) {
        cout << "EXIT_NON_ZERO" << endl;
        return EXIT_RE;
      }
    }
    if (WIFSIGNALED(status)) {
      if (WTERMSIG(status) == SIGXCPU) {
        cout << "TLE" << endl;
        return EXIT_TLE;
      }
      if (WTERMSIG(status) == SIGKILL) {
        cout << "MLE" << endl;
        return EXIT_MLE;
      }
      if (WTERMSIG(status) == SIGABRT) {
        cout << "MLE" << endl;
        return EXIT_MLE;
      }
      cout << "RE" << endl;
      return EXIT_RE;
    }
    cout << "EXIT_SUC";
    return EXIT_SUC;
  }
  }
}
