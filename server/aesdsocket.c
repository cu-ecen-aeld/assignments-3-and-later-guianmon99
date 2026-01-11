#include "stdbool.h"
#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define error_exit -1

#define data_file "/var/tmp/aesdsocketdata"
#define buffer_size 22000

typedef enum {
  socket_state_init,
  socket_state_listen,
  socket_state_connected
} socket_states_t;

volatile bool operation = true;
static void handle_error(char *error_string) {
  if (error_string)
    syslog(LOG_ERR, "%s %s", error_string, strerror(errno));
  exit(error_exit);
}

static int open_or_create_file(void) {
  int ret = open(data_file, O_WRONLY | O_APPEND | O_CREAT, 0644);

  if (ret == -1) {
    handle_error("file could not be opened");
  }
  return ret;
}

int server_fd, peer_fd, out;

static void signal_handler(int number) {
  syslog(LOG_INFO, "caught signal");
  operation = false;
}

void setup_signal() {
  struct sigaction sa;
  sa.sa_handler = signal_handler; // Your handler function
  sigemptyset(&sa.sa_mask);

  // Crucial: Do NOT include SA_RESTART in sa_flags
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
  }
}

int main(int argc, char *argv[]) {
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog("aesdsocket", LOG_PERROR, LOG_USER);
  int opt = 1, daemon = 0;
  socket_states_t state = socket_state_init;

  if (argc == 2) {
    if (strcmp(argv[1], "-d") == 0) {
      daemon = 1;
    }
  }

  setup_signal();
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    handle_error("failed to set socket option");
  }

  struct sockaddr_in server_addr, peer_addr;

  memset(&server_addr, 0,
         sizeof(server_addr)); // Ensure the struct is zeroed out

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(9000);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    handle_error("bind failed");
  }
  if (daemon) {

    pid_t pid = fork();
    if (pid < 0)
      exit(EXIT_FAILURE);
    if (pid > 0)
      exit(EXIT_SUCCESS); // Parent exits cleanly
  }
  state = socket_state_listen;

  if (listen(server_fd, 10) == -1)
    handle_error("listen failed");

  /* Now we can accept incoming connections one
     at a time using accept(2) */
  socklen_t peer_addr_size;
  peer_addr_size = sizeof(peer_addr);

  out = open_or_create_file();

  int used_buffer = 0;
  char buffer[buffer_size] = {0};

  while (operation) {

    if (state == socket_state_listen) {

      peer_fd =
          accept(server_fd, (struct sockaddr *)&peer_addr, &peer_addr_size);
      if (peer_fd == -1)
        handle_error("accept failed");

      syslog(LOG_INFO, "Accepted connection from %s",
             inet_ntoa(peer_addr.sin_addr));
      state = socket_state_connected;
    }
    int len = recv(peer_fd, &buffer[used_buffer], buffer_size - used_buffer, 0);
    if (len <= 0) {
      if (len == 0) {
        state = socket_state_listen;
      } else {

        handle_error("receive error");
      }
    }
    syslog(LOG_DEBUG, "arrived data with len %d", len);
    if (strchr(buffer, '\n') != NULL) {
      write(out, &buffer[used_buffer], len);
      used_buffer += len;
      write(peer_fd, buffer, used_buffer);
    }
  }
  syslog(LOG_INFO, "finish");
  remove(data_file);
  close(out);
  close(peer_fd);
  close(server_fd);
  return 0;
}
