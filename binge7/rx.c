#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <signal.h>
#include <stdbool.h>
#include "list.h"

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

struct player_t {
  struct u_stat_t stats;
  struct timeval updated_at;
};

bool done = false;
bool any_changed = false;

static void
int_handler( int sig )
{
  done = true;
}

list *players;

static int
cmp_plname( struct u_stat_t *a, struct player_t *b )
{
  return strncmp(a->plname, b->stats.plname, PL_NSIZ);
}

#define STATUS_ACTIVE   0
#define STATUS_SAVED    1
#define STATUS_INACTIVE 2

#define PLAYER_PLNAME  0
#define PLAYER_ITEMS   1
#define PLAYER_RACE    2
#define PLAYER_GENDER  3
#define PLAYER_ALIGN   4
#define PLAYER_CLASS   5
#define PLAYER_HP      6
#define PLAYER_MAXHP   7
#define PLAYER_ULEVEL  8
#define PLAYER_AC      9
#define PLAYER_PRAYERS 10
#define PLAYER_WISHES  11
#define PLAYER_DEATHS  12
#define PLAYER_MOVES   13
#define PLAYER_DLEVEL  14
#define PLAYER_DUNGEON 15
#define PLAYER_MAX     16

static int player_sorting = PLAYER_PLNAME;
static int player_direction = 1;

static int
cmp_player( struct player_t *pa, struct player_t *pb )
{
  struct u_stat_t *a = &pa->stats;
  struct u_stat_t *b = &pb->stats;

  switch(player_sorting) {
  default:
  case PLAYER_PLNAME: return cmp_plname(a, pb);
  case PLAYER_ITEMS: return b->have - a->have;
  case PLAYER_RACE: return b->race - a->race;
  case PLAYER_GENDER: return b->gender - a->gender;
  case PLAYER_ALIGN: return b->align - a->align;
  case PLAYER_CLASS: return strncmp(a->class, b->class, 3);
  case PLAYER_HP: return b->hp - a->hp;
  case PLAYER_MAXHP: return b->hpmax - a->hpmax;
  case PLAYER_ULEVEL: return b->ulevel - a->ulevel;
  case PLAYER_AC: return a->ac - b->ac;
  case PLAYER_PRAYERS: return b->prayers - a->prayers;
  case PLAYER_WISHES: return b->wishes - a->wishes;
  case PLAYER_DEATHS: return b->deaths - a->deaths;
  case PLAYER_MOVES: return b->moves - a->moves;
  case PLAYER_DLEVEL: return b->dlevel - a->dlevel;
  case PLAYER_DUNGEON: return strcmp(a->dungeon_or_death, b->dungeon_or_death);
  }
}

static void
update( struct u_stat_t *u_stat )
{
  struct player_t *existing = list_goto(players, u_stat, (compare) cmp_plname);
  if( !existing ) {
    existing = (struct player_t *) malloc(sizeof(struct player_t));
    list_unshift(players, existing);
  }
  memcpy(&existing->stats, u_stat, sizeof(struct u_stat_t));
  gettimeofday(&existing->updated_at, NULL);
}

int
main( int argc, char *argv[] )
{
  char name[256];
  struct u_stat_t u_stat;
  struct sockaddr_in addr, from;
  socklen_t fromlen;
  struct hostent *hent;
  u_int yes = 1;
  bool first_time = true;
  int i;

  int s = socket(PF_INET, SOCK_DGRAM, 0);
  if( s < 0 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  gethostname(name, sizeof(name));

  hent = gethostbyname(name);

  if( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  if( setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  const struct in_addr localhost_addr = {
      .s_addr = htonl(INADDR_LOOPBACK)
  };
  if( setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &localhost_addr, sizeof(localhost_addr)) == -1 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  const struct ip_mreq mreq = {
    .imr_multiaddr = {
      .s_addr = inet_addr("225.0.0.37")
    },
    .imr_interface = {
      .s_addr = htonl(INADDR_LOOPBACK)
    }
  };
  if( setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 ) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(12345);

  if( bind(s, (struct sockaddr *) &addr, sizeof(addr)) != 0 ) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  players = list_new();
  list_set_free(players, (free_function) free, NULL);

  signal(SIGINT, int_handler);

  while(!done) {
    fd_set rfds;
    int indl, indr;
    char *sline;
    struct timeval tv, *tvp;

    /* shitty */

    list_sort(players, (compare) cmp_player);
    if( player_direction < 0 )
      list_reverse(players);
    switch(player_sorting) {
    default:
    case PLAYER_PLNAME: indl = 11; indr = 16; break;
    case PLAYER_ITEMS: indl = 16; indr = 22; break;
    case PLAYER_RACE: indl = -1; indr = -1;
      break;
    case PLAYER_GENDER: indl = -1; indr = -1;
      break;
    case PLAYER_ALIGN: indl = -1; indr = -1;
      break;
    case PLAYER_CLASS: indl = -1; indr = -1;
      break;
    case PLAYER_HP: indl = 32; indr = 36; break;
    case PLAYER_MAXHP: indl = 36; indr = 40; break;
    case PLAYER_ULEVEL: indl = 40; indr = 43; break;
    case PLAYER_AC: indl = 44; indr = 47; break;
    case PLAYER_PRAYERS: indl = 48; indr = 51; break;
    case PLAYER_WISHES: indl = 51; indr = 54; break;
    case PLAYER_DEATHS: indl = 54; indr = 57; break;
    case PLAYER_MOVES: indl = 57; indr = 63; break;
    case PLAYER_DLEVEL: indl = 63; indr = 66; break;
    case PLAYER_DUNGEON: indl = 66; indr = 79; break;
    }
    if( indl >= 0 ) {
    }
    if( indr >= 0 ) {
    }
	//MAGIC NUMBERS
    sline = (char *) malloc(400);

    struct timeval now;
    gettimeofday(&now, NULL);
	//MAGIC NUMBERS++
    for( i = 0; i < list_count(players); i++ ) {
      const char alignments[3] = "cnl";
      struct player_t *player = list_goto_index(players, i);
      struct u_stat_t *stats = &player->stats;
	// (MAGIC_NUMBER++)++
      snprintf(sline, 400,
         "%s,%c,%c,%c,%s,%d,%d,%d,%d,%d,%d,%d,%ld,%d,%s\n",
	 stats->plname,
         stats->race, stats->gender, alignments[stats->align + 1],
         stats->class, stats->hp, stats->hpmax, stats->ulevel,
         stats->ac, stats->prayers, stats->wishes, stats->deaths,
         stats->moves, stats->dlevel, stats->dungeon_or_death);
    }
    printf(sline);
    fflush(stdout);
    if( first_time ) {
      int bsock;
      struct sockaddr_in other;
      first_time = false;
      fprintf(stderr, sline);
      free(sline);
      bsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if( bsock == -1 ) {
        perror("socket");
        exit(EXIT_FAILURE);
      }

      memset(&other, 0, sizeof(other));
      other.sin_family = AF_INET;
      other.sin_port = htons(12346);
      if( inet_aton("127.0.0.1", &other.sin_addr) == 0 ) {
        fprintf(stderr, "inet_aton failed\n");
        exit(EXIT_FAILURE);
      }

      if( sendto(bsock, &yes, sizeof(yes), 0,
             (struct sockaddr *) &other, sizeof(other)) == -1 ) {
        perror("sendto");
	exit(EXIT_FAILURE);
      }

      shutdown(bsock, SHUT_RDWR);
      close(bsock);
    }
    else{
	free(sline);
    }

    FD_ZERO(&rfds);
    FD_SET(s, &rfds);
    FD_SET(0, &rfds);

    if( any_changed ) {
      tv.tv_sec = 0;
      tv.tv_usec = 100000;
      tvp = &tv;
    } else {
      tvp = NULL;
    }

    switch( select(s + 1, &rfds, NULL, NULL, tvp) ) {
    case 0: continue; /* timeout... shouldn't happen */
    case -1:
      if( errno == EINTR || errno == ETIMEDOUT )
        continue;
      perror("select");
      exit(EXIT_FAILURE);
    default:
      if( FD_ISSET(s, &rfds) ) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if( recvfrom(s, &u_stat, sizeof(u_stat), 0, (struct sockaddr *) &from, &fromlen) < 0 ) {
          perror("recvfrom");
          exit(EXIT_FAILURE);
        }

        update(&u_stat);
      }
    }

  }


  list_destroy(players);

  exit(EXIT_SUCCESS);
}
