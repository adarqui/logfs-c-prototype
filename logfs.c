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
	filler(buf, "hi", NULL, 0);

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

	char * base64Data = NULL, * base64Path = NULL;
	ssize_t n = -1;

	XDEBUG("logfs_write: %s %p[%.*s] %jd %jd\n", path, buf, (int)size, buf, size, offset);

	if(unfd < 0) return size;

	Curl_base64_encode(buf, size, &base64Data);
	Curl_base64_encode(path, strlen(path), &base64Path);

	if(base64Data && base64Path) {
		n = dprintf(unfd, "{ \"mod\" : \"logfs\", \"path\" : \"%s\", \"message\" : \"%s\" }\n", base64Path, base64Data);
	}

	if(base64Data) free(base64Data);
	if(base64Path) free(base64Path);

	if(n < 0) {
		init_socket(0);
		return -1;
	}

	return size;
}


static struct fuse_operations logfs_oper = {
	.getattr	= logfs_getattr,
	.readdir	= logfs_readdir,
	.truncate	= logfs_truncate,
	.open		= logfs_open,
	.create 	= logfs_create,
	.read		= logfs_read,
	.write		= logfs_write
};


int clear_socket(void) {
	if(unfd >= 0) {
		close(unfd);
		unfd = -1;
	}

	alarm(1);

	return -1;
}

void init_socket(int num) {
	struct sockaddr_un sun;
	int r;

	XDEBUG("init_socket: entered\n");

	if(unfd >= 0) {
		close(unfd);
		unfd = -1;
	}

	memset(&sun,0,sizeof(sun));
	unfd = socket(AF_UNIX,SOCK_STREAM,0);
	if(unfd < 0) {
		clear_socket();
		return;
	}

	alarm(0);

	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, upstream_path, sizeof(sun.sun_path)-1);

	r = connect(unfd, (struct sockaddr *)&sun, sizeof(sun));
	if(r < 0) {
		clear_socket();
		return;
	}

	return;
}

void init_signal_handlers(void) {
	signal(SIGALRM, init_socket);
}

void init_debug_fp(void) {
	debug_fp = fopen("/tmp/logfs.log", "a+");
	if(!debug_fp) exit(-1);
}

void init_env(void) {
	if(getenv("UPSTREAM")) {
		upstream_path = getenv("UPSTREAM");
	}
	return;
}

int main(int argc, char *argv[])
{

	init_env();

#if defined(DEBUG)
	init_debug_fp();
#endif

	init_signal_handlers();
	init_socket(0);

	return fuse_main(argc, argv, &logfs_oper, NULL);
}
