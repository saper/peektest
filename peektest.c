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
		bail("socket()");
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

	client = socket(AF_INET, SOCK_DGRAM, 0);

	err = sendto(client, buf, 0x20, 0, (struct sockaddr *)&addr, sizeof (addr));
	if (err < 0) {
		bail("sendto()");
	}

	iov.iov_base = NULL;
	iov.iov_len = 0;
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_flags = MSG_PEEK;

	err = recvmsg(server, &hdr, MSG_PEEK|MSG_TRUNC);
	printf("peek len: %x errno: %d flags: %x\n", err, errno, hdr.msg_flags);

	iov.iov_base = &buf;
	iov.iov_len = 0x20;
	err = recvmsg(server, &hdr, 0);
	printf("recv len: %x errno: %d flags: %x\n", err, errno, hdr.msg_flags);


	close(client);
	close(server);
}
