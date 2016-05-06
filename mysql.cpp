//	mysql and log functions
#include "OurHouseServer.h"


MYSQL *openDb(){
  
	MYSQL *dbConn = mysql_init(NULL);
	if (dbConn == NULL){
		writeLog("%s\n", mysql_error(dbConn));
		exit(1);
	}

	if (mysql_real_connect(dbConn, DBSERVER, DBUSER, DBPASS, DBNAME, 0, NULL, 0) == NULL){
		writeLog("%s\n", mysql_error(dbConn));
		mysql_close(dbConn);
		exit(1);
	}

	return dbConn;  
}


void writeObservationToDb(MYSQL *dbConn, char *msg){
	//	message from the sensor sender is in following form:
	//	SDt&mmmmmmmm&tt&mmmmmm&ttt&mmmmmm 
	//	where	t is the device id – it will be a string representing a number between 1 and 999, 
	//		mmmmm is the value with decimal place in text and
	//		& indicates the the end of the value
	//	for example:
	//		SD1&28.4&182&1005.2&23&0.751245516

	char sql[200] = "INSERT INTO Observations(observedDateTime, sensorId, value) VALUES(NOW(),\0";
	int i = 2, j;

	while(msg[i] != 0){
		j = 73;
		//	start  with the sensor id
		while(msg[i] != '&'){
			sql[j++] = msg[i++];
		}
		sql[j++] = ',';
		i++;	//	move past the '&'

		//	then the value
		while((msg[i] != '&') && (msg[i] != 0)){
			sql[j++] = msg[i++];
		}
		sql[j++] = ')';
		sql[j] = 0;
		if (mysql_query(dbConn, sql)){
      	writeLog("%s\n", mysql_error(dbConn));
		}
printf("sql->%s\n",sql);
		if (msg[i] != 0)
			i++;	// move past the '&'
	}
}


void writeLog(const char *tmplate, ...)
{
	int fd = open(LOGFILEPATH,	O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IROTH);

	char buffer[200];
	buffer[0] = '\0';
	time_t currentTime = time(NULL);
	strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
	strcat(buffer,"  ");

	va_list ap;
	va_start (ap, tmplate);
  	vsprintf ((&buffer[20]), tmplate, ap);
	va_end (ap);											// clean up

	write(fd, buffer, strlen(buffer));	
	close(fd);

	return;
}

