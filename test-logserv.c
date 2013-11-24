#include <zmq.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main (int argc, char *argv[])
{
	void *ctx = zmq_ctx_new ();
	void *pub = zmq_socket (ctx, ZMQ_PUB);
	int rc, n, i = 0, imax = -1, sleepval = 500;
	
	rc = zmq_connect (pub, "tcp://localhost:1010"), n, i;

	assert (rc == 0);

	if(argc > 1) {
		sleepval = atoi(argv[1]);
	}

	if(argc > 2) {
		imax = atoi(argv[2]);
	}

	if(sleepval == 0) sleepval = 500;

	while (i < imax) {
		char buf [20];
		snprintf(buf, sizeof(buf)-1, "log %i %i", getpid(), i);
		n = zmq_send(pub, buf, strlen(buf), 0);
		usleep(sleepval);
		i++;
	}
	zmq_close (pub);
	zmq_ctx_destroy (ctx);
	return 0;
}
