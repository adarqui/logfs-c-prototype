#include <zmq.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void usage(void) {
	puts("tail -f file.log | logit localhost web-server file.log");
	puts("logit <logserv host> <environment tag> <file tag>");
	exit(-1);
}

int main (int argc, char *argv[])
{
	void *ctx = zmq_ctx_new ();
	void *pub = zmq_socket (ctx, ZMQ_PUB);
	int rc, n;
	unsigned long seq;
	char *connect_string, *env_string, *file_string;
	

	if(argc < 3) {
		usage();
	}

	asprintf(&connect_string,"tcp://%s:1010",argv[1]);
	env_string = argv[2];
	file_string = argv[3];

	rc = zmq_connect(pub, connect_string);
	assert(rc == 0);

	/*
	 * This is to wakeup the zmq connection
	 */
	n = zmq_send(pub, "log hello\n", strlen("log hello\n"), 0);
	sleep(1);

	while (1) {
		char buf [10024];
		char buf2 [10024];
		n = read(0,buf,sizeof(buf)-1);
		if(n <= 0) break;
		n = snprintf(buf2, sizeof(buf2)-1, "log %lu:%s:%s:%.*s\n", seq, env_string, file_string, n, buf);
		n = zmq_send(pub, buf2, n, 0);
		seq++;
	}
	sleep(1);
	zmq_close (pub);
	zmq_ctx_destroy (ctx);
	return 0;
}
