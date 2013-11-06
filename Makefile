FLAGS=  # -DDEBUG

all:
	gcc -Wall -O3 logfs.c base64.c $(FLAG) `pkg-config fuse --cflags --libs` -o logfs

clean:
	rm -f logfs

test:
	mkdir -p /tmp/logfs 2>/dev/null
	rm -f /tmp/upstream.sock ; nc -lU /tmp/upstream.sock &
	UPSTREAM=/tmp/upstream.sock ./logfs -o allow_other /tmp/logfs
	echo "hi" > /tmp/logfs/file.log
	umount /tmp/logfs

test2:
	echo "replacing /var/log: this is dirty, you should do this manually with '-d' specified to ./logfs"
	UPSTREAM=/tmp/upstream.sock ./logfs -o nonempty -o allow_other /var/log/
	pkill -HUP -f syslog
	rm -f /tmp/upstream.sock ; nc -lU /tmp/upstream.sock
