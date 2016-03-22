#include "sock.h"

#define MAX_CLIENTS 30

int main() {
	struct addrinfo hint, *result;
    struct sockaddr remote;
    struct timeval timeout = {0, 0};
    int res, master_socket, sd, client_sockets[MAX_CLIENTS], max_sd;
	unsigned int addrlen;
    fd_set readfds;

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

    memset(client_sockets, 0, sizeof(client_sockets));

    addrlen = sizeof(struct sockaddr);
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
          
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        
        res = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ( res < 0) {
            perror("error : func select error");
            exit(1);
        }

        if (FD_ISSET(master_socket, &readfds)) {
            
            sd = accept(master_socket, &remote, &addrlen);
            if (sd < 0) {
                perror("error : cannot accept client socket\n");
                exit(1);
            }
            
            //strcpy(buf, "Welcome Client");
            //write(sd, buf, strlen(buf));

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                     client_sockets[i] = sd;
                     break;
                }
            }
        }
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                res = read(sd, buf, sizeof(buf));
                
                if (res) {
                    printf("receive client data : %s\n", buf);
					deal(buf, file);	
                    res = write(sd, file, strlen(file));
                    printf("write <hello client> to client\n");
                }
				printf("close socket\n");
				client_sockets[i] = 0;
				close(sd);
            }
        }

    }
    return 0;
}

int getFirstLine(char *str, char *line) {	
	int i = 0;
	for(i = 0; i < 100 && *(str+i) != '\n'; i++) {
		*(line+i) = *(str+i);
	}
	*(line+i) = '\0';
	return 1;
}

int getPath(char *line, char *path) {
	int i = 0;
	for(i = 0; i < 100 && *(line+i) != ' '; i++);
	i++;
	int j = 0;
	for(; i < 100 && *(line+i) != ' '; i++, j++) {
		*(path + j) = *(line + i); 
	}
	*(path+j) = '\0';
	return 1;
}

int parseHttp(char *http, char *file) {
	char line[100], path[100];
	getFirstLine(http, line);
	getPath(line, path);
	getcwd(file, 100);
	strcat(file, path);
	return 1;
}

int setTitle(int code, char *msg, char *line) {
	sprintf(line, "HTTP/1.1 %d %s\r\n", code, msg);
	return 1;
}

int setHeader(char *key, char * value, char *line) {
	sprintf(line, "%s: %s\r\n", key, value);
	return 1;
}

int fileexists(char *filename) {
	return (access(filename, 0) == 0);
}

void deal(char * http, char *buf) {
	char file[1024]; 
	char line[100];
	parseHttp(http, file);
	buf[0] = '\0';
	if(fileexists(file)) {
		setTitle(200, "OK", line);
		strcat(buf, line);
		setHeader("Content-Type", "text/html", line);
		strcat(buf, line);
		strcat(buf, "\r\n");
		FILE* fp;
		char fbuf[1024];
		fp = fopen(file, "r");
		fread(fbuf, 1024, 1, fp);
		fclose(fp);
		strcat(buf, fbuf);
		strcat(buf, "\r\n");
		strcat(buf, "\r\n");
	}
	else {
		setTitle(404, "OUCH, FILE NOT FOUND", line);
		strcat(buf, line);
		strcat(buf, "\r\n");
		strcat(buf, "\r\n");
	}
}
