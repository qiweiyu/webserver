#ifndef _HTTP_H
#define _HTTP_H

#include <string.h>
#include "util/qString.h"
#include "util/qEvent.h"
#include "util/qDict.h"

typedef enum httpMethod {
	GET,
	POST,
	UNKNOWN,
} httpMethod;

typedef enum contentType {
	html,
	htm,
	jpg,
	bmp,
	gif	
} contentType;

typedef struct httpClient {
	int fd;
	httpMethod method;
	qString *url;
	qString *version;
	qString *host;
	int	port;
	qDict *headers;
	int statusCode;
	contentType type;
	int	length;
	qString *readBuff;
	qString *writeBuff;
} httpClient;

httpClient *createHttpClient(qEvent *event, int fd);
void freeHttpClient(httpClient *client);

void readFromClient(qEvent *event, int fd, httpClient *client);
void writeToClient(qEvent *event, int fd, httpClient *client);

#endif
