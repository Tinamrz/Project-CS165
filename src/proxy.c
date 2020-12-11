#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tls.h>
#include <unistd.h>
#include "murmur3.h"
//your application name -port portnumber -servername:serverportnumber
static void usage()
{
	extern char * __progname;
	fprintf(stderr, "usage: %s portnumber serverportnumber\n", __progname);
	exit(1);
}

static void kidhandler(int signum) {
	/* signal handler for SIGCHLD */
	waitpid(WAIT_ANY, NULL, WNOHANG);
}


int main(int argc,  char *argv[])
{
	struct sockaddr_in sockname, client;
	char buffer[80], *ep;
	struct sigaction sa;
	int sd;
	socklen_t clientlen;
	u_short port;
	pid_t pid;
	u_long p;

	/*
	 * first, figure out what port we will listen on - it should
	 * be our first parameter.
	 */

	if (argc != 3)
		usage();
		errno = 0;
        p = strtoul(argv[1], &ep, 10);
        if (*argv[1] == '\0' || *ep != '\0') {
		/* parameter wasn't a number, or was empty */
		fprintf(stderr, "%s - not a number\n", argv[1]);
		usage();
	}
        if ((errno == ERANGE && p == ULONG_MAX) || (p > USHRT_MAX)) {
		/* It's a number, but it either can't fit in an unsigned
		 * long, or is too big for an unsigned short
		 */
		fprintf(stderr, "%s - value out of range\n", argv[1]);
		usage();
	}
	/* now safe to do this */
	port = p;
	//murmur hashing
	uint32_t hash[4];                // Output for the hash
	uint32_t seed = 42;              // Seed value for hash
	MurmurHash3_x86_32(strcat(argv[4],proxyname), strlen(strcat(argv[4],proxyname)), seed, hash);
	
	
	// Kramer: Similarly to the server code, we need to set up a tls. The proxy acts as a middle-man in-between 
	// the server and the client, acting as a "server of the server" if you will.
	
	struct tls_config *serverConfigTLS = tls_config_new();
	struct tls_config *clientConfigTLS = NULL;
	
	// Kramer: We need to make two TLS connections - one for the server and one for the client.
	
