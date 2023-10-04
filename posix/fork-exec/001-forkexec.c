/// Basic fork-exec

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGC_BUFFER_SIZE 128

static int streq(const char *a, const char *b) {
  return strcmp(a, b) == 0;
}

static int streqAlt(const char *a, const char *b, const char *c) {
  return streq(a, b) || streq(a, c);
}

static void printUsage() {
  fprintf(stderr, "Usage: [options] executable [args...]\n");
}

int main(int argc, char **argv) {
  // Skip the name of the process itself
  if (argc > 0) {
    argc--;
    argv++;
  }

  // options
  int verbose = 0;
  int searchPath = 1;
  while (argc > 0) {
    if (streqAlt(argv[0], "--use-path", "-p")) {
      searchPath = 1;
    } else if (streq(argv[0], "--no-path")) {
      searchPath = 0;
    } else if (streqAlt(argv[0], "--verbose", "-v")) {
      verbose = 1;
    } else {
      break;
    }
    argc--;
    argv++;
  }
  if (argc <= 0) {
    printUsage();
    return 1;
  }
  int childArgc = argc;
  char *childArgvBuffer[ARGC_BUFFER_SIZE];
  char **childArgv = childArgvBuffer;
  if (childArgc >= ARGC_BUFFER_SIZE) {
    childArgv = (char **)malloc(sizeof(char *) * (childArgc + 1));
  }
  memcpy(childArgv, argv, sizeof(char *) * childArgc);
  childArgv[childArgc] = NULL;

  if (argc <= 1) {
    printUsage();
    fprintf(stderr, "Usage: ./a.out executable [args...]\n");
    if (searchPath) {
      execvp(childArgv[0], childArgv);
    }
    return 1;
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    if (searchPath) {
      execvp(childArgv[0], childArgv);
    } else {
      execv(childArgv[0], childArgv);
    }
    fprintf(stderr, "exec failed\n");
    return 127;
  }

  // Parent process
  if (verbose) {
    printf("VERBOSE: pid = %d\n", (int)pid);
  }
  int waitStatus;
  pid_t waitReturn = waitpid(pid, &waitStatus, 0);
  while (waitReturn != pid) {
    if (waitReturn == (pid_t)-1) {
      if (errno == EINTR) {
        // Interrupted due to signal to the parent/calling process
        fprintf(stderr, "parent process interrupted\n");
        return 1;
      } else {
        // Some other error, unclear what the error is
        fprintf(stderr, "waitpid error, errno = %d", (int)errno);
        return 1;
      }
    }
    // State was changed, but not finished? idk when this would happen
    // If WNOHANG flag was set, then waitpid can return 0. But
    // docs don't really mention any other scenario:
    // https://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
    fprintf(stderr,
            "WARN: waitpid returned %d, errno = %d\n",
            (int)waitReturn,
            (int)errno);
    waitReturn = waitpid(pid, &waitStatus, 0);
  }
  if (WIFEXITED(waitStatus)) {
    // child process exited normally
    if (verbose) {
      printf("VERBOSE: child process finished normally, exit code = %d\n",
             WEXITSTATUS(waitStatus));
    }
    return WEXITSTATUS(waitStatus);
  }
  if (WIFSIGNALED(waitStatus)) {
    // child process killed by uncaught signal
    if (verbose) {
      printf("VERBOSE: child process killed by signal, signal = %d\n",
             WTERMSIG(waitStatus));
    }
    return 127;
  }
  return 0;
}
