//	to compile:
//	g++ -Wall -o OurHouseServer OurHouseServer.cpp mysql.cpp tcp.cpp events.cpp `mysql_config --cflags --libs`


#include "OurHouseServer.h"

MYSQL *dbConn=0;
//		store information about the 0 to 256 devices in the system
struct device{
	char 		state;
	int		connectionIdx;
	char		setStateTo;
	time_t	stateChangeRequestTime;
};
struct device devices[1000];
char *msg;

int isMessageType(char *buff, const char *msgType);
int getDeviceId(char *msg, int startPos);



int main(int argc, char **argv){

	int i;
	//	initialise the devices table
	for (i = 0; i < 999; i++){
		devices[i].state = '0';
		devices[i].setStateTo = '0';
		devices[i].connectionIdx = -1;
		devices[i].stateChangeRequestTime = 0;
	}
	int deviceId;	
	int maxDeviceId = 0;
	time_t timeNow;
	char buf[100];

	//	open a database connection
	dbConn = openDb();

	//	load up scheduled events
	schedEvents events(dbConn);

	//	start the tcp listener
	tcpConnections connections;

	//	main loop
	for(;;){

		if (connections.check()){

			msg = connections.message();

			//	SD Store Data
			if (isMessageType(msg,"SD")){
				connections.sendMsg((char *)"WR");
				writeObservationToDb(dbConn, msg);
			}

			//	RD Register Device
			if (isMessageType(msg,"RD")){
				deviceId = getDeviceId(msg, 2);
				devices[deviceId].connectionIdx = connections.connectionId();
				connections.sendMsg((char *)"DR");
				if (deviceId > maxDeviceId)
					maxDeviceId = deviceId;				
				writeLog("Register device id %d on socket %d\n",deviceId,connections.connectionId());
			}	
				
			//	RS Report State
			if (isMessageType(msg,"RS")){
				deviceId = getDeviceId(msg, 2);
				devices[deviceId].state = msg[5];
				if ((devices[deviceId].setStateTo == devices[deviceId].state) &&
					 (devices[deviceId].stateChangeRequestTime != 0)){
					devices[deviceId].stateChangeRequestTime = 0;
				}
				connections.sendMsg((char *)"WR");
			}

			//	QS Query State
			if (isMessageType(msg,"QS")){
				deviceId = getDeviceId(msg, 2);
				sprintf(buf, "RS%03d%c", deviceId, devices[deviceId].state);
				connections.sendMsg(buf);
			}

			//	RE	Reload events
			if (isMessageType(msg,"RE")){
				connections.sendMsg((char *)"WR");
				connections.closeConnection();
				events.reload(dbConn);
				writeLog("Scheduled events reload\n");
			}

			//	ES	External State change - a state change called from another device
			if (isMessageType(msg,"ES")){
				connections.sendMsg((char *)"WR");
				deviceId = getDeviceId(msg, 2);
				devices[deviceId].setStateTo = msg[5];
				devices[deviceId].stateChangeRequestTime = 0;
				writeLog("ES msg deviceId %u set state to %c\n", deviceId, devices[deviceId].setStateTo);
			}

			//	DS	Device initiated state change - a state change originating from the device, 
			//	probs from a physical button or some such
			if (isMessageType(msg,"DS")){
				connections.sendMsg((char *)"WR");
				deviceId = getDeviceId(msg, 2);
				devices[deviceId].setStateTo = msg[5];
				devices[deviceId].state = msg[5];
				writeLog("DS msg deviceId %u state set to %c\n", deviceId, devices[deviceId].setStateTo);
			}

			//	HB Heart Beat
			if (isMessageType(msg,"HB")){
				connections.sendMsg((char *)"HB");
			}

		}

		//	check for scheduled events
		time(&timeNow);
		if (timeNow > events.nextDateTime()){
			deviceId = events.nextEvent()->deviceId();
			devices[deviceId].setStateTo = events.nextEvent()->setStateTo();
			devices[deviceId].stateChangeRequestTime = 0;
			events.clickOverToNextEvent();
		}

		//	check if we need to send any change state messages
		for (i = 1; i <= maxDeviceId; i++){		
			if (devices[i].state != devices[i].setStateTo){
				if ((devices[i].connectionIdx != -1) &&
				  	 ((timeNow - devices[i].stateChangeRequestTime) > 5)){
					//	send a change state message
					sprintf(buf,"CS%03d%c",i,devices[i].setStateTo);
					connections.sendMsg(devices[i].connectionIdx, buf);
					devices[i].stateChangeRequestTime = timeNow;
					writeLog("Change State device id %u, state %c\n",deviceId, devices[i].setStateTo);
				}
			}
		}

	}

	mysql_close(dbConn);
	return 0;
}

int isMessageType(char *buff, const char *msgType){
	if ((buff[0] == msgType[0]) && (buff[1] == msgType[1]))
		return 1;
	else
		return 0;
}

int getDeviceId(char *msg, int startPos){
	char buf[4];
	int i;
	for (i = 0; i < 3; i++){
		buf[i] = msg[startPos + i];
	}
	buf[i] = 0;
	i = atoi(buf);
	return i;
}
