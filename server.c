#include "sock.h"
#include "http.h"

void listenClient(qEvent *event, int fd, void *client) {
	int sd = accept(fd, NULL, NULL);	
	createHttpClient(event, sd);
}

int main() {
	struct addrinfo hint, *result;
    struct sockaddr remote;
    struct timeval timeout = {0, 0};
    int res, master_socket;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;
    hint.ai_flags    = AI_PASSIVE;

    res = getaddrinfo(NULL, "8088", &hint, &result);
    if (res != 0) {
        perror("error : cannot get socket address!\n");
        exit(1);
    }

    master_socket = socket(result->ai_family, result->ai_socktype,
                           result->ai_protocol);
    if (master_socket == -1) {
        perror("error : cannot get socket file descriptor!\n");
        exit(1);
    }
	int option = 1;
	setsockopt ( master_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );
			
    res = bind(master_socket, result->ai_addr, result->ai_addrlen);
    if (res == -1) {
        perror("error : cannot bind the socket with the given address!\n");
        exit(1);
    }

    res = listen(master_socket, SOMAXCONN);
    if (res == -1) {
        perror("error : cannot listen at the given socket!\n");
        exit(1);
    }
		
	qEvent *event = qCreateEvent();
	qCreateFileReadEvent(event, master_socket, (eventProc*)listenClient, NULL);
	qEventLoop(event);
    return 0;
}
