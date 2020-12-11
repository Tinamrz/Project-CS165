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
#include "bloom.h"

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
	
	// For the bloom filter we are using the bloom filter library
	bloom_parameters parameters;
	parameters.projected_element_count = 30000;  //objects
	parameters.false_positive_probability = 0.01;   //probability of the false positive
	// Simple randomizer 
   	parameters.random_seed = 0xA5A5A5A5;
	if (!parameters)
   	{
      	std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
      	return 1;
   	}
	bloom_filter filter(parameters);
	

	unsigned int Cache = 0;
	i = 0;
	//we need to set up a tls. The proxy acts as a middle-man in-between the server and the client.
	if ((tls_cfg = tls_config_new()) == NULL)
		errx(1, "unable to allocate TLS config")
	if (tls_accept_socket(tls_ctx, &tls_cctx, clientsd) == -1)
	errx(1, "Acception by TLS failed %s", tls_error(tls_ctx));
	else {
	do {
	if ((i = tls_handshake(tls_cctx)) == -1)
	errx(1, "TLS handshake failed %s", tls_error(tls_ctx));
		} while(i == TLS_WANT_POLLIN || i == TLS_WANT_POLLOUT);
	}
	char buffer[200], 
	//Now it is time to get the object from the client.
	ssize_t file;
	file = tls_read(tls_cctx, buffer,200);
	//Now that we have the file name, it is time to search the bloom filter
	//for inserting the Bloom values
	filter.insert(bloom,buffer);
	if(boolCache == 1){
	printf("IN CACHE Proxy sent: contents of %s\n\n",buffer);
	}  

	
