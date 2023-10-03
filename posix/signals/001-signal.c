// A program that just waits for a signal
// then prints some info about the signal just received
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void signalHandler(int signal) {
  printf("signalHandler: signal = %d\n", signal);
}

int main(int argc, const char **argv) {
  struct sigaction action;
  action.sa_handler = signalHandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  // DESCRIPTION
  // pause() causes the calling process (or thread) to sleep until a
  //      signal is delivered that either terminates the process or causes
  //      the invocation of a signal-catching function.
  //
  // RETURN VALUE
  // pause() returns only when a signal was caught and the signal-
  //      catching function returned.  In this case, pause() returns -1,
  //      and errno is set to EINTR.
  //      https://man7.org/linux/man-pages/man2/pause.2.html
  printf("PAUSING...\n");
  int pauseReturn = pause();
  printf("pauseReturn = %d, errno = %d\n", pauseReturn, (int)errno);
  printf("  (EINTR = %d)\n", EINTR);
  return 0;
}
