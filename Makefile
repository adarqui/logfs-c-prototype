all:
	gcc -Wall -O3 logfs.c base64.c -DDEBUG `pkg-config fuse --cflags --libs` -o logfs

clean:
	rm -f logfs

test:
	mkdir -p /tmp/logfs 2>/dev/null
	rm -f /tmp/upstream.sock ; nc -lU /tmp/upstream.sock &
	UPSTREAM=/tmp/upstream.sock ./logfs -o allow_other /tmp/logfs
	echo "hi" > /tmp/logfs/file.log
	umount /tmp/logfs
