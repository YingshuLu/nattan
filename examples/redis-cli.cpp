#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include "task/Task.h"
#include "log/Logger.h"

using namespace nattan;

Logger logger("./debug.log");

class RedisClientTask: public Task {

public:

void run() {
    unsigned int j, isunix = 0;
    redisContext *c;
    redisReply *reply;
    const char *hostname = "127.0.0.1";

    int port = 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    if (isunix) {
        c = redisConnectUnixWithTimeout(hostname, timeout);
    } else {
        c = redisConnectWithTimeout(hostname, port, timeout);
    }
    if (c == NULL || c->err) {
        if (c) {
            logger.info("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            logger.info("Connection error: can't allocate redis context\n");
        }
        exit();
    }

    /* PING server */
    reply = redisCommand(c,"PING");
    logger.info("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key */
    reply = redisCommand(c,"SET %s %s", "foo", "hello world");
    logger.info("SET: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    logger.info("SET (binary API): %s\n", reply->str);
    freeReplyObject(reply);

    /* Try a GET and two INCR */
    reply = redisCommand(c,"GET foo");
    logger.info("GET foo: %s\n", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(c,"INCR counter");
    logger.info("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);
    /* again ... */
    reply = redisCommand(c,"INCR counter");
    logger.info("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);

    /* Create a list of numbers, from 0 to 9 */
    reply = redisCommand(c,"DEL mylist");
    freeReplyObject(reply);
    for (j = 0; j < 10; j++) {
        char buf[64];

        snprintf(buf,64,"%u",j);
        reply = redisCommand(c,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* Let's check what we have inside the list */
    reply = redisCommand(c,"LRANGE mylist 0 -1");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            logger.info("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);

	reply = redisCommand(c, "SUBSCRIBE foo");
	while(redisGetReply(c, &reply) == REDIS_OK) {
		logger.info("subcribe foo, active\n");
		logger.info("notice %s", reply->str);
		freeReplyObject(reply);
	}
    /* Disconnects and frees the context */
    redisFree(c);
}

};


int main() {
	Task* task = new RedisClientTask();
	task->start();
	Task::sched();
}
