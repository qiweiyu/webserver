#include <unistd.h>
#include "http.h"
#include "util/qIo.h"
#include "util/qDict.h"

httpClient *createHttpClient(qEvent *event, int fd) {
	httpClient *client = qCalloc(1, sizeof(httpClient));	
	client->fd = fd;
	qCreateFileReadEvent(event, fd, (eventProc*)readFromClient, (void *)client);
	return client;
}

void freeHttpClient(httpClient *client) {
	if(client->url) {
		qFreeString(client->url);
	}
	if(client->version) {
		qFreeString(client->version);
	}
	if(client->host) {
		qFreeString(client->host);
	}
	if(client->headers) {
		qFreeDict(client->headers);
	}
	if(client->readBuff) {
		qFreeString(client->readBuff);
	}
	if(client->writeBuff) {
		qFreeString(client->writeBuff);
	}
	qFree(client);
}

static void parseHeader(char *key, char *value, char *line) {
	char *pos = strstr(line, ":");
	if(pos == NULL) return ;
	int i = 0;
	for(i = 0; i < (pos-line); i++) {
		char c = *(line+i);
		if(c >= 'A' && c <= 'Z') {
			c = c-'A'+'a';
		}
		*(key+i) = c;
	}
	*(key+i) = '\0';
	while(*(pos+1) == ' ') {
		pos++;
	}
	for(i = 0; *(pos+i) != '\0'; pos++) {
		*(value+i) = *(pos+i);
	}
	*(value+i) = '\0';
}

static qString *parseHttpHeaderHost(char *str, int *port) {
	char *pos = strstr(str, ":");
	qString *host;
	if(pos == NULL) {
		host = qCreateString(str);
		*port = 80;
	}
	else {
		int p = 0;
		char *stop = pos;
		while(++pos && *pos >= '0' && *pos <= '9') {
			p = p*10+(*pos)-'0';
		}
		if(p == 0) p = 80;
		*port = p;
		*stop = '\0';
		host = qCreateString(str);
		*stop = ':';
	}
	return host;
}

static void parseHttpRequest(httpClient *client) {
	qLinkList *lines = qExplodeString(client->readBuff, "\r\n");
	qLinkList *firstLine = qExplodeString(lines->head->value, " ");
	if(strcmp(firstLine->head->value, "GET") == 0) {
		client->method = GET;
	}
	else if(strcmp(firstLine->head->value, "POST") == 0) {
		client->method = POST;
	}
	else {
		client->method = UNKNOWN;
	}
	client->url = qCreateString(firstLine->head->next->value);
	client->version = qCreateString(firstLine->head->next->next->value);

	qLinkListNode *node = lines->head->next;
	char key[4000], value[4000];
	while(node) {
		parseHeader(key, value, node->value);
		qAddValueToDictByStrKey(client->headers, key, qCreateString(value));
	}
	qFreeLinkList(lines);
	qFreeLinkList(firstLine);
	qString *host = qFindValueFromDictByStrKey(client->headers, "host");
	if(host) {
		client->host = parseHttpHeaderHost(host, &(client->port));
	}
}

static void dealHttpRequest(httpClient *client) {
	char *buff = "HTTP/1.1 200 OK\r\nServer: QWY\r\nContent-Type:text/html\r\n\r\n";
	qCatString(client->writeBuff, buff);
}

void readFromClient(qEvent *event, int fd, httpClient *client) {
	client->readBuff = qReadInBuff(client->fd);
	parseHttpRequest(client);
	dealHttpRequest(client);
	qCreateFileWriteEvent(event, fd, (eventProc*)writeToClient, (void *)client);
	qRmFileReadEvent(event, fd);
}

void writeToClient(qEvent *event, int fd, httpClient *client) {
	int retval, size = strlen(client->writeBuff);
	char *buff = client->writeBuff;
	for(; size > 0;) {
		retval = write(fd, buff, size);
		if(retval < 0) break;
		size = size-retval;
	}
	qRmFileWriteEvent(event, fd);
	close(fd);
}
