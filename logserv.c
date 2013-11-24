#include <zmq.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <time.h>

extern int errno;

typedef struct INFO {
	char * logdir;
	void * ctx;
	void * sub;
	int fd;
} info_t;

info_t info;

void hup_handler(int);
void init_log(void);
void usage(char *);

void hup_handler(int num) {
	init_log();
}

void init_log(void) {
	time_t t;
	struct tm *tm;
	char name[32];

	memset(name,0,sizeof(name));
	time(&t);
	tm = localtime(&t);
	strftime(name, sizeof(name)-1, "%F.%H:%M:%S", tm);

	close(info.fd);
	info.fd = open(name, O_WRONLY|O_CREAT, S_IWUSR|S_IRUSR|S_IXUSR|S_IRGRP|S_IROTH);
	if(info.fd < 0) {
		exit(-1);
	}

	return;
}

void usage(char * s) {
	printf(
		"Error: %s\n"
		"\n"
		"Usage: ./logserv <directory>\n"
	, s);
	exit(-1);
}

int main (int argc, char *argv[])
{
	char buf[10024];

	if(argc < 2)
		usage("Provide a directory.");

	info.logdir = argv[1];

	if(access(info.logdir, W_OK) < 0)
		usage("Directory doesn't exist.");

	daemon(0,0);

	umask(0);
	chdir(info.logdir);
	init_log();
	signal(SIGHUP, hup_handler);

    info.ctx = zmq_ctx_new ();
    info.sub = zmq_socket (info.ctx, ZMQ_SUB);

    int rc = zmq_bind (info.sub, "tcp://*:1010"), n;
    assert (rc == 0);

    zmq_setsockopt(info.sub, ZMQ_SUBSCRIBE, "log", 1);

    while (1) {
        n = zmq_recv (info.sub, buf, sizeof(buf)-1, 0);
        dprintf (info.fd, "%.*s\n", n, buf);
    }
    return 0;
}
