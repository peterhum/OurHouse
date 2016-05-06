#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <mysql.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//	mysql stuff
#define DBSERVER "localhost"
#define DBUSER "website"
#define DBPASS "markit9data"
#define DBNAME "OurHouse"
MYSQL *openDb();
void writeObservationToDb(MYSQL *, char *buf);

//	log stuff
#define LOGFILEPATH "/usbdrive/OurHouseServer/OurHouseServer.log"
void writeLog(const char *, ...);

//	TCP stuff
//	port we're listening on
#define PORT 6675

//	scheduled events
class schedEvent{
	public:
		schedEvent(int id, int deviceId, int timeOfDay, time_t oneOffDateTime, char scheduledDaysOfWeek[7], char setStateTo);
		~schedEvent();
		int		deviceId() {return m_deviceId;}
		time_t	eventDateTime() {return m_eventDateTime;}
		char		setStateTo() {return m_setStateTo;}
		time_t	clickOverToNextEvent();

	private:
		int		m_id;
		int		m_deviceId;
		int		m_timeOfDay;
		time_t	m_oneOffDateTime;
		char 		m_scheduledDaysOfWeek[7];
		char 		m_setStateTo;
		time_t	m_eventDateTime;

		int daysTillNextScheduleDay(int wday);

};

class schedEvents{
	public:
		schedEvents(MYSQL *);
		~schedEvents();
		time_t 			nextDateTime();
		schedEvent * 	nextEvent();
		time_t			clickOverToNextEvent();
		void				reload(MYSQL *);
	private:
		void sortEvents();
		schedEvent *m_events[100];
		int m_eventCount;
};	

//	TCP connections
class tcpConnections{
	public:
		tcpConnections();
		int check();
		char * message();
		int connectionId();
		void sendMsg(char *msg);
		void sendMsg(int connectionId, char *msg);
		void closeConnection();
		
	private:
		fd_set master;							//	master file descriptor list
		fd_set read_fds;						//	temp file descriptor list for select()
		struct sockaddr_in serveraddr;	//	server and client addresses
		struct sockaddr_in clientaddr;
		int fdmax;								//	maximum file descriptor number
		int listener;							//	listening socket descriptor
		int newfd;								//	newly accept()ed socket descriptor
		char buf[1024];						//	buffer for client data
		int nbytes;
		int m_connectionId;					//	index of master data received on
		time_t lastUsed[256];				//	store the last used time by connection
		struct timeval timeout;	
};



