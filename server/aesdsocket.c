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
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define error_exit -1

#define data_file "/var/tmp/aesdsocketdata"
#define buffer_size 22000

pthread_mutex_t lock;
volatile bool operation = true;
pthread_t timer_task;

struct socket_info {
  int fd, out, peer;
  pthread_t thread;
  char buffer[buffer_size];
  bool finished_execution;
  SLIST_ENTRY(socket_info) entries;
};

SLIST_HEAD(socket_list, socket_info) head;

void *socket_worker(void *arg) {
  struct socket_info *info = arg;
  int used_buffer = 0;

  syslog(LOG_DEBUG, "started worker thread with fd %d", info->fd);
  while (true) {
    int len = recv(info->fd, &info->buffer[used_buffer],
                   buffer_size - used_buffer, 0);
    if (len <= 0)
      break;

    used_buffer += len;
    syslog(LOG_DEBUG, "arrived data with len %d", len);
    if (strchr(info->buffer, '\n') != NULL) {
      pthread_mutex_lock(&lock);
      syslog(LOG_INFO, "writting");
      write(info->out, info->buffer, used_buffer);
      used_buffer = 0;
      memset(info->buffer, 0, buffer_size);
      off_t posicion_original = lseek(info->out, 0, SEEK_CUR);
      lseek(info->out, 0, SEEK_SET);

      int file_bytes = read(info->out, info->buffer, buffer_size);
      syslog(LOG_INFO, "bytes read %d", file_bytes);
      if (file_bytes > 0) {

        syslog(LOG_INFO, "writing to peer %d bytes ", file_bytes);
        write(info->peer, info->buffer, file_bytes);
      } else if (file_bytes < 0) {
        syslog(LOG_ERR, "error reading out file %s", strerror(errno));
      }
      lseek(info->out, posicion_original, SEEK_SET);

      pthread_mutex_unlock(&lock);
      memset(info->buffer, 0, buffer_size);
    }
  }
  info->finished_execution = true;
  // close(info->peer);

  return NULL;
}

static void handle_error(char *error_string) {
  if (error_string)
    syslog(LOG_ERR, "%s %s", error_string, strerror(errno));
  exit(error_exit);
}

static int open_or_create_file(void) {
   char buffer [100] = {0};
   sprintf(buffer,"rm %s",data_file);
   system(buffer);	
  int ret = open(data_file, O_RDWR | O_APPEND | O_CREAT, 0644);

  if (ret == -1) {
    handle_error("file could not be opened");
  }
  return ret;
}

int server_fd, peer_fd, out;

static void signal_handler(int number) {
  syslog(LOG_INFO, "caught signal");
  operation = false;
  shutdown(server_fd, SHUT_RDWR); 
}

void setup_signal() {
  struct sigaction sa = {0};
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
  }
       sigaction(SIGTERM, &sa, NULL);

}

void handle_termination_request(void) {
  struct socket_info *np;
  while (!SLIST_EMPTY(&head)) {
    np = SLIST_FIRST(&head);
    SLIST_REMOVE_HEAD(&head, entries);
    
    shutdown(np->peer, SHUT_RDWR);
    pthread_join(np->thread, NULL);
    
    close(np->fd);
    free(np); // Crucial para Valgrind
  }
}


void *timer_thread(void *arg) {

  char outstr[200];
  time_t t;
  struct tm *tmp;

  while (operation) {

    t = time(NULL);
    tmp = localtime(&t);

    if (strftime(outstr, sizeof(outstr), "timestamp:%a, %d %b %Y %T %z \n",
                 tmp) != 0) {
      pthread_mutex_lock(&lock);

      write(out, outstr, strlen(outstr));

      pthread_mutex_unlock(&lock);
    }

    printf("Result string is \"%s\"\n", outstr);

    sleep(10);
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog("aesdsocket", LOG_PERROR, LOG_USER);
  int opt = 1, daemon = 0;
  SLIST_INIT(&head);
  pthread_mutex_init(&lock, NULL);

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

  memset(&server_addr, 0, sizeof(server_addr));

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
      exit(EXIT_SUCCESS);
  }

  if (listen(server_fd, 10) == -1)
    handle_error("listen failed");

  /* Now we can accept incoming connections one
     at a time using accept(2) */
  socklen_t peer_addr_size;
  peer_addr_size = sizeof(peer_addr);

  out = open_or_create_file();

  pthread_create(&timer_task, NULL, timer_thread, NULL);
  while (operation) {

    peer_fd = accept(server_fd, (struct sockaddr *)&peer_addr, &peer_addr_size);
    if (peer_fd == -1)
        break;
    syslog(LOG_INFO, "Accepted connection from %s",
           inet_ntoa(peer_addr.sin_addr));

    syslog(LOG_DEBUG, "accepted fd %d", peer_fd);

    struct socket_info *node = malloc(sizeof(struct socket_info));
    if (!node) {
      syslog(LOG_ERR, "out of memory !");
      break;
    }
    node->fd = peer_fd;
    node->finished_execution = false;
    node->out = out;
    node->peer = peer_fd;

    SLIST_INSERT_HEAD(&head, node, entries);

    pthread_create(&node->thread, NULL, socket_worker, node);

    struct socket_info *np = SLIST_FIRST(&head);
    struct socket_info *next;

    while (np != NULL) {
      next =
          SLIST_NEXT(np, entries);

      if (np->finished_execution) {
        pthread_join(np->thread, NULL);

        SLIST_REMOVE(&head, np, socket_info, entries);

        close(np->fd);
        free(np);

        syslog(LOG_INFO, "Memory and thread cleaned up");
      }
      np = next; 
    }
  }

  syslog(LOG_INFO, "finish");
  handle_termination_request();
  pthread_join(timer_task, NULL);
  pthread_mutex_destroy(&lock);
  remove(data_file);
  close(out);
  close(server_fd);
  return 0;
}
