// Spawns a child, then sends a signal to the child

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void childSignalHandler(int signal) {
  printf("CHILD: signal handler: signal = %d\n", signal);
}

static void printSignalLegend() {
  printf("SIGNAL VALUES\n");
  printf("  SIGHUP = %d\n", SIGHUP);
  printf("  SIGILL = %d\n", SIGILL);
  printf("  SIGINT = %d\n", SIGINT);
  printf("  SIGKILL = %d\n", SIGKILL);
  printf("  SIGPIPE = %d\n", SIGPIPE);
  printf("  SIGSEGV = %d\n", SIGSEGV);
  printf("  SIGTERM = %d\n", SIGTERM);
  printf("  SIGWINCH = %d\n", SIGWINCH);
}

void sigact(int signal, const struct sigaction *action) {
  int ret = sigaction(signal, action, NULL);
  if (ret != 0) {
    fprintf(stderr, "ERROR sigaction: singal = %d, ret = %d, errno = %d\n",
            signal, ret, errno);
    exit(127);
  } else {
    printf("sigaction signal = %d OK\n", signal);
  }
}

int main(int argc, char **argv) {
  // skip argv[0]
  if (argc > 0) {
    argc--;
    argv++;
  }

  // options
  int catchImpossibleSignals = 0;
  int signal = SIGTERM;
  while (argc > 0) {
    if (strcmp(argv[0], "--impossible-signals") == 0) {
      argc--;
      argv++;
    } else if (strcmp(argv[0], "--signal") == 0) {
      argc--;
      argv++;
      if (argc > 0) {
        signal = atoi(argv[0]);
        argc--;
        argv++;
      }
    } else {
      fprintf(stderr, "Unrecognized option '%s'", argv[0]);
      return 1;
    }
  }

  printSignalLegend();

  pid_t pid = fork();
  if (pid == 0) {
    // child process
    // Set up signal handler
    struct sigaction action;
    action.sa_handler = childSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigact(SIGHUP, &action);
    sigact(SIGILL, &action);
    sigact(SIGINT, &action);
    if (catchImpossibleSignals) {
      sigact(SIGKILL, &action);
    }
    sigact(SIGPIPE, &action);
    sigact(SIGSEGV, &action);
    sigact(SIGTERM, &action);
    sigact(SIGWINCH, &action);

    // Just wait for a signal
    printf("CHILD: pausing...\n");
    int pauseReturn = pause();
    printf("CHILD: pauseReturn = %d, errno = %d\n", pauseReturn, (int)errno);
    printf("CHILD:   (EINTR = %d)\n", EINTR);
    return 0;
  }

  // parent process
  // send a signal to the child process
  sleep(1);
  printf("PARENT: Sending signal %d to child (pid = %d)\n", signal, (int)pid);
  int killReturn = kill(pid, signal);
  if (killReturn != 0) {
    printf("PARENT: killReturn = %d, errno = %d\n", killReturn, errno);
  }

  return 0;
}
