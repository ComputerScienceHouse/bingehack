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

int
main( int argc, char *argv[] )
{
	char name[256];
	struct u_stat_t u_stat;
	struct sockaddr_in addr, from;
	socklen_t fromlen;
	u_int yes = 1;

	int s = socket(PF_INET, SOCK_DGRAM, 0);
	if( s < 0 ) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	gethostname(name, sizeof(name));

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

	signal(SIGINT, int_handler);

	while(!done) {
		fd_set rfds;
		struct timeval tv, *tvp;

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

				struct u_stat_t *stats = &u_stat;
				const char alignments[3] = "cnl";
				char *sline;
				if( asprintf(&sline, "%s,%c,%c,%c,%s,%d,%d,%d,%d,%d,%d,%d,%ld,%d,%s\n",
					 stats->plname,
					 stats->race, stats->gender, alignments[stats->align + 1],
					 stats->class, stats->hp, stats->hpmax, stats->ulevel,
					 stats->ac, stats->prayers, stats->wishes, stats->deaths,
					 stats->moves, stats->dlevel, stats->dungeon_or_death) == -1 ) {
					perror("asprintf");
					exit(EXIT_FAILURE);
				}
				printf(sline);
				free(sline);
			}
		}

	}

	exit(EXIT_SUCCESS);
}
