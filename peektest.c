#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>


void bail(const char *msg)
{
	perror(msg);
	exit(1);
}


int main()
{
	int server, client, err;
	socklen_t len;
	struct sockaddr_in addr;
	char buf[0x20];
	struct msghdr hdr;
	struct iovec iov;

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	inet_aton("127.0.0.1", &addr.sin_addr);

	bzero(&hdr, sizeof (hdr));
	bzero(&iov, sizeof (iov));
	bzero(buf, sizeof (buf));

	server = socket(AF_INET, SOCK_DGRAM, 0);
	if (server < 0) {
		bail("socket(server)");
	}
	err = bind(server, (struct sockaddr *)&addr, sizeof (addr));
	if (err < 0) {
		bail("bind()");
	}

	len = sizeof (addr);
	err = getsockname(server, (struct sockaddr *)&addr, &len);
	if (err < 0) {
		bail("getsockname()");
	}
  printf("Using port %d\n", addr.sin_port);

	client = socket(AF_INET, SOCK_DGRAM, 0);
	if (client < 0) {
		bail("socket(client)");
	}

  strncpy(buf, "ABCD", 4);
	err = sendto(client, buf, sizeof(buf), 0, (struct sockaddr *)&addr, sizeof (addr));
	if (err < 0) {
		bail("sendto()");
	}

	errno = 0;
	iov.iov_base = &buf;
	iov.iov_len = 2;
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_flags = 0;
	bzero(buf, sizeof (buf));

	printf("peek buflen: 0x%02zx\n", iov.iov_len);
	err = recvmsg(server, &hdr, MSG_PEEK);
	if (err < 0)
		perror("recvmsg(server, ..., MSG_PEEK");
	printf("peek len: 0x%02x errno: %d flags: 0x%02x\n", err, errno, hdr.msg_flags);
	printf("peek read: [%-4s]\n", buf);

	iov.iov_base = &buf;
	iov.iov_len = 0x20;
	bzero(buf, sizeof (buf));
	printf("read buflen: 0x%02zx\n", iov.iov_len);
	err = recvmsg(server, &hdr, 0);
	if (err < 0) {
		perror("recvmsg(server, ..., 0)");
	}
	printf("recv len: 0x%02x errno: %d flags: 0x%02x\n", err, errno, hdr.msg_flags);
	printf("recv read: [%-4s]\n", buf);

	close(client);
	close(server);
}
