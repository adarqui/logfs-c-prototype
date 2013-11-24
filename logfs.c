/*
  gcc -Wall logfs.c `pkg-config fuse --cflags --libs` -o logfs
*/

#include "logfs.h"

static int logfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	XDEBUG("logfs_getattr: %s %p\n", path, stbuf);

	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
	}
	else {
		stbuf->st_mode = S_IFREG | 0666;
	}
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = 0;
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return res;
}

static int logfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	XDEBUG("logfs_readdir: %s %p %jd\n", path, buf, offset);

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	return 0;
}


static int logfs_truncate(const char *path, off_t size)
{
	(void) size;

	XDEBUG("logfs_truncate: %s %jd\n", path, size);

	return 0;
}


static int logfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	XDEBUG("logfs_create: %s %i\n", path, mode);

	return 0;
}


static int logfs_open(const char *path, struct fuse_file_info *fi)
{

	XDEBUG("logfs_open: %s\n", path);

	return 0;
}

static int logfs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	(void) fi;

	XDEBUG("logfs_read: %s %p %jd %jd\n", path, buf, size, offset);

	return 0;
}


static int logfs_write(const char *path, const char *buf, size_t size,
	off_t offset, struct fuse_file_info *fi) {
	int n;
	char * msg = NULL;

	XDEBUG("logfs_write: %s %p[%.*s] %jd %jd\n", path, buf, (int)size, buf, size, offset);

	n = asprintf(&msg, "%lu:%s:%s:%.*s", info.seq, info.pfx, path, size, buf);

	n = zmq_send(info.pub, msg, n, 0);
	if(n < 0) {
		XDEBUG("logfs_write: %s %i\n", strerror(errno), n);
	}

	if(msg)
		free(msg);

	info.seq++;

	return size;
}

void *logfs_init(struct fuse_conn_info *fi) {
	init_zmq();
	return NULL;
}


static struct fuse_operations logfs_oper = {
	.getattr	= logfs_getattr,
	.readdir	= logfs_readdir,
	.truncate	= logfs_truncate,
	.open		= logfs_open,
	.create 	= logfs_create,
	.read		= logfs_read,
	.write		= logfs_write,
	.init 		= logfs_init,
};


int init_zmq(void) {
	int n, rcvhwm = 30000;

	info.ctx = zmq_ctx_new();
	info.pub = zmq_socket(info.ctx, ZMQ_PUB);

	n = zmq_connect(info.pub, info.con);
	if(n<0) {
		XDEBUG("init_zmq: %s\n", strerror(errno));
		exit(-1);
	}
	XDEBUG("init_zmq: %i %s\n", n, info.con);

	zmq_setsockopt(info.pub, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));	

	return 0;
}

void fini(void) {
	if(info.pub)
		zmq_close(info.pub);
	if(info.ctx);
		zmq_ctx_destroy(info.ctx);
	return;
}

void init_signal_handlers(void) {
}

void init_debug_fp(void) {
#if defined(DEBUG)
	debug_fp = fopen("/tmp/logfs.log", "a+");
	if(!debug_fp) exit(-1);
#endif
}

void init_env(void) {
	if(getenv("UPSTREAM")) {
		info.con = getenv("UPSTREAM");
	} else {
		XDEBUG("init_env: Specify UPSTREAM\n");
		exit(-1);
	}

	if(getenv("PREFIX")) {
		info.pfx = getenv("PREFIX");
		asprintf(&info.pfx, "log %s", info.pfx);
	} else {
		info.pfx = "log null";
	}

	return;
}

int main(int argc, char *argv[])
{
	init_env();

#if defined(DEBUG)
	init_debug_fp();
#endif

	return fuse_main(argc, argv, &logfs_oper, NULL);
}
