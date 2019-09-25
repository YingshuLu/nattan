#include <stdio.h>
#include <mysql/mysql.h>
#include "task/Task.h"
#include "log/Log.h" 
#include "log/Logger.h" 

using namespace nattan;

Logger logger("./test.log");


class MysqlTask : public Task {
private:
MYSQL *my;
 
public:
void run() {
		char host[20];
		char user[20];
		char pass[20];
		 
		my = mysql_init(NULL);
		 
		 
		sprintf(host,"localhost");
		sprintf(user,"root");
		sprintf(pass,"111111");
		 
		if (my == NULL ) {
				printf("Cant initalisize MySQL\n");
				return;
		}
		 
		if( mysql_real_connect (my,host,user,pass,NULL,0,NULL,0)  == NULL) {
				printf("Error cant login\n");
		} else {
				logger.info("Login correct\n");
		}

		if (mysql_query(my, "CREATE DATABASE kamdb")) 
		{
				fprintf(stderr, "%s\n", mysql_error(my));
				mysql_close(my);
				return;
		}

		logger.info("create database correct\n");
		mysql_close(my);
		return;
}

};

int main() {
	nattan::DevelopLog::switchOn();
	nattan::DevelopLog::setDebugFile("./test.log");
	nattan::DevelopLog::setErrorFile("./error.log");


	Task* ptask = new MysqlTask();
	ptask->start();
	Task::sched();
	return 0;
}
