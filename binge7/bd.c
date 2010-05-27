#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

#include "list.h"

typedef unsigned char bool;

#define false 0
#define true  1

#define PL_NSIZ 32

struct u_stat_t {
  char race;
  char gender;
  char align;
  char have;
  short hp, hpmax;
  short pow, powmax;
  short ac;
  short ulevel;
  short dlevel;
  short wishes;
  short prayers;
  short deaths;
  char class[3];
  char status;
  long moves; /* 8 bytes */
  char plname[PL_NSIZ];
  char dungeon_or_death[1024 - 32];
};

bool done = false;

static void
int_handler( int sig )
{
  signal(SIGINT, int_handler);

  done = true;
}

list *players;

static int
cmp_plname( struct u_stat_t *a, struct u_stat_t *b )
{
  return strncmp(a->plname, b->plname, PL_NSIZ);
}

int
main( int argc, char *argv[] )
{
  list *players;
  struct u_stat_t u_stat;
  int s;
  int mcast_socket;
  int recv_socket;
  struct sockaddr_in addr;
  struct sockaddr_in mcast_addr;
  struct sockaddr_in recv_addr;
  struct ip_mreqn mreq;
  u_int yes = 1;

  /* mcast receive */
  s = socket(PF_INET, SOCK_DGRAM, 0);
  if( s < 0 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

#ifdef __FreeBSD__
  if( setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
#endif

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(12345);

  if( bind(s, (struct sockaddr *) &addr, sizeof(addr)) != 0 ) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  mreq.imr_multiaddr.s_addr = inet_addr("225.0.0.37");
  mreq.imr_address.s_addr = htonl(INADDR_LOOPBACK);
  mreq.imr_ifindex = 0;

  if( setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                 &mreq, sizeof(struct ip_mreq)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  /* mcast send */
  mcast_socket = socket(PF_INET, SOCK_DGRAM, 0);
  if( mcast_socket < 0 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&mcast_addr, 0, sizeof(mcast_addr));
  mcast_addr.sin_family = AF_INET;
  mcast_addr.sin_addr.s_addr = inet_addr("225.0.0.37");
  mcast_addr.sin_port = htons(12345);

  /* notify receive */
  recv_socket = socket(PF_INET, SOCK_DGRAM, 0);
  if( recv_socket < 0 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  recv_addr.sin_port = htons(12346);
  if( bind(recv_socket,
           (struct sockaddr *) &recv_addr, sizeof(recv_addr)) == -1 ) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  players = list_new();
  list_set_free(players, (free_function) free, NULL);

  signal(SIGINT, int_handler);

  while(!done) {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(s, &rfds);
    FD_SET(recv_socket, &rfds);

    int ret = select((s > recv_socket ? s : recv_socket) + 1,
                     &rfds, NULL, NULL, NULL);

    if( ret == -1 ) {
      if( errno != EINTR && errno != ETIMEDOUT ) {
        perror("select");
        exit(EXIT_FAILURE);
      }
      continue;
    }

    if( FD_ISSET(0, &rfds) ) {
      struct u_stat_t *p;
      char buf[12];
      read(0, buf, sizeof(buf));
      printf( "Sending all\n" );
      list_reset(players);
      while( (p = list_next(players)) ) {
        if( sendto(mcast_socket, p, sizeof(u_stat), 0,
                   (struct sockaddr *) &mcast_addr,
                   (socklen_t) sizeof(mcast_addr)) == -1 ) {
          perror("sendto");
          exit(EXIT_FAILURE);
        }
      }
    }

    /* u_stat notification */
    if( FD_ISSET(s, &rfds) ) {
      struct u_stat_t *p;
      recvfrom(s, &u_stat, sizeof(u_stat), 0, NULL, 0);
      p = list_goto(players, &u_stat, (compare) cmp_plname);
      if( p == NULL ) {
        p = (struct u_stat_t *) malloc(sizeof(struct u_stat_t));
        list_unshift(players, p);
      }
      memcpy(p, &u_stat, sizeof(u_stat));
    }

    if( FD_ISSET(recv_socket, &rfds) ) {
      struct u_stat_t *p;
      recvfrom(recv_socket, &u_stat, sizeof(u_stat), 0, NULL, 0);
      /* send all stats held */
      list_reset(players);
      while( (p = list_next(players)) ) {
        if( sendto(mcast_socket, p, sizeof(u_stat), 0,
               (struct sockaddr *) &mcast_addr,
               (socklen_t) sizeof(mcast_addr)) == -1 ) {
          perror("sendto");
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  list_destroy(players);

  shutdown(s, SHUT_RDWR);
  shutdown(recv_socket, SHUT_RDWR);
  shutdown(mcast_socket, SHUT_RDWR);

  exit(EXIT_SUCCESS);
}
