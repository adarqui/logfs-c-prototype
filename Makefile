FLAGS=  # -DDEBUG

all:
	gcc -Wall -O3 logfs.c base64.c -lzmq -lpthread $(FLAG) `pkg-config fuse --cflags --libs` -o logfs
	gcc logserv.c -lzmq -o logserv
	gcc test-logserv.c -lzmq -o test-logserv

clean:
	rm -f logfs

mount:
	mkdir -p /tmp/logfs 2>/dev/null
#	UPSTREAM="tcp://127.0.0.1:1010" ./logfs -o allow_other /tmp/logfs
	UPSTREAM=tcp://localhost:1010 ./logfs -f -o allow_other -o nonempty -o auto_unmount -o intr /tmp/logfs

unmount:
	umount /tmp/logfs

test:
	mkdir -p /tmp/logfs 2>/dev/null
	UPSTREAM="tcp://127.0.0.1:1010" ./logfs -o allow_other /tmp/logfs
	echo "hi" > /tmp/logfs/file.log
	umount /tmp/logfs

test2:
	echo "replacing /var/log: this is dirty, you should do this manually with '-d' specified to ./logfs"
	UPSTREAM="tcp://127.0.0.1:1010" ./logfs -o nonempty -o allow_other /var/log/
	pkill -HUP -f syslog
