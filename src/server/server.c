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

// Kramer: Adding the TLS Library on line 13.
// Alphabetical order makes Kramer happy.

static void usage()
{
	extern char * __progname;
	fprintf(stderr, "usage: %s portnumber\n", __progname);
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

	if (argc != 2)
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
	
	// Kramer: We aren't required to send this. Disable it for now.
	/* the message we send the client
	strlcpy(buffer,
	    "What is the air speed velocity of a coconut laden swallow?\n",
	    sizeof(buffer));
	*/
	
	// Kramer: Now that our port is secured, we can establish the TLS connection.
	// Bob Beck's libTLS tutorial: struct tls_config : a tls configuration, used to configure tls contexts. Config includes things 
	// like what certificate and key to use as well as validation options.
	struct tls_config *configurationTLS = tls_config_new();
	
	// Project Description: You may assume that all proxies already have the CA’s root certificate (root.pem) required to authenticate the server.
	// Kramer: Does that mean we don't have to send it to them? I guess we should set up the CA root cert just in-case?
	tls_config_set_ca_file(configurationTLS, "root.pem");
	
	// Bob Beck's libTLS tutorial: Optionally Call tls_config_set_cert_file to add your own certificate.
	// A server will normally do this. Clients may not if they are connecting without client authentication.
	
	tls_config_set_cert_file(configurationTLS, "server.crt");
	
	// Bob Beck's libTLS tutorial: Optionally Call tls_config_set_key_file to add your certificate key.
	// A server will normally do this. Clients may not if they are connecting without client authentication.
	
	tls_config_set_key_file(configurationTLS, "server.key");
	
	// Bob Beck's libTLS tutorial: Once this is done you have a configuration set up to potentially initiate
	// or receive TLS connections. to make use of that configuration you need to get yourself a tls context using
	// tls_server to set up a server context.
	
	struct tls *serverTLS = tls_server();
	
	// Bob Beck's libTLS tutorial: Once you have this you apply a configuration to a context using
	// tls_configure to take your server or client context, and apply the configuration to it.
	
	tls_configure(serverTLS, configurationTLS);
	
	// Bob Beck's libTLS tutorial: Now you're actually ready make TLS connections.
	// Kramer: Yay!

	memset(&sockname, 0, sizeof(sockname));
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(port);
	sockname.sin_addr.s_addr = htonl(INADDR_ANY);
	sd=socket(AF_INET,SOCK_STREAM,0);
	if ( sd == -1)
		err(1, "socket failed");

	if (bind(sd, (struct sockaddr *) &sockname, sizeof(sockname)) == -1)
		err(1, "bind failed");

	if (listen(sd,3) == -1)
		err(1, "listen failed");

	/*
	 * we're now bound, and listening for connections on "sd" -
	 * each call to "accept" will return us a descriptor talking to
	 * a connected client
	 */


	/*
	 * first, let's make sure we can have children without leaving
	 * zombies around when they die - we can do this by catching
	 * SIGCHLD.
	 */
	sa.sa_handler = kidhandler;
        sigemptyset(&sa.sa_mask);
	/*
	 * we want to allow system calls like accept to be restarted if they
	 * get interrupted by a SIGCHLD
	 */
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1)
                err(1, "sigaction failed");

	/*
	 * finally - the main loop.  accept connections and deal with 'em
	 */
	printf("Server up and listening for connections on port %u\n", port);
	for(;;) {
		int clientsd;
		clientlen = sizeof(&client);
		clientsd = accept(sd, (struct sockaddr *)&client, &clientlen);
		if (clientsd == -1)
			err(1, "accept failed");
		/*
		 * We fork child to deal with each connection, this way more
		 * than one client can connect to us and get served at any one
		 * time.
		 */

		pid = fork();
		if (pid == -1)
		     err(1, "fork failed");

		if(pid == 0) {
			ssize_t written, w;
			
			// Bob Beck's libTLS tutorial: In a server, after you call accept, you call
			// tls_accept_socket to associate a tls context to your accepted socket.
			
			// Kramer: I think this is the right place to implement?
			
			// Kramer: Implementation needs a client TLS connection, so put a null one in for now?
			
			struct tls *clientTLS = NULL;
			
			tls_accept_socket(serverTLS, &clientTLS, clientsd);
			
			// Bob Beck's libTLS tutorial: Sending and receiving of data is done with tls_read and tls_write.
			// They are designed to be similar in use, and familiar to programmers that have experience 
			// with the normal POSIX read and write system calls.
			
			/*
			 * write the message to the client, being sure to
			 * handle a short write, or being interrupted by
			 * a signal before we could write anything.
			 */
			w = 0;
			written = 0;
			while (written < strlen(buffer)) {
				w = write(clientsd, buffer + written,
				    strlen(buffer) - written);
				if (w == -1) {
					if (errno != EINTR)
						err(1, "write failed");
				}
				else
					written += w;
			}
			close(clientsd);
			exit(0);
		}
		close(clientsd);
	}
}
