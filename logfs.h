/*
  gcc -Wall logfs.c `pkg-config fuse --cflags --libs` -o logfs
*/

#ifndef LOGFS_H
#define LOGFS_H

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "base64.h"

/*
 * Some dirty globals
 */
int unfd = -1;
char * upstream_path = "/var/run/mpassd.sock";
FILE * debug_fp;
#if defined(DEBUG)
#define XDEBUG(msg,...) fprintf(debug_fp,msg,##__VA_ARGS__); fflush(debug_fp)
#else
#define XDEBUG(msg,...) { }
#endif

static int logfs_getattr(const char *, struct stat *);
static int logfs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
static int logfs_truncate(const char *, off_t);
static int logfs_create(const char *, mode_t, struct fuse_file_info *);
static int logfs_open(const char *, struct fuse_file_info *);
static int logfs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
static int logfs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);

int clear_socket(void);
void init_env(void);
void init_debug_fp(void);
void init_signal_handlers(void);
void init_socket(int);

int main(int, char **);

#endif
