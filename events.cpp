#include "OurHouseServer.h"

//	***** schedEvent::schedEvent(...) *****
schedEvent::schedEvent(int id, int deviceId, int timeOfDay, time_t oneOffDateTime, char scheduledDaysOfWeek[7], char setStateTo){
	m_id = id;
	m_deviceId = deviceId;
	m_timeOfDay = timeOfDay;
	m_oneOffDateTime = oneOffDateTime;
	for (int i=0; i<7; i++){
		m_scheduledDaysOfWeek[i] = scheduledDaysOfWeek[i];
	}
	m_setStateTo = setStateTo;

	//	set the event date time to the next event from the time now
	clickOverToNextEvent();
}

//	***** schedEvent::~schedEvent() *****
schedEvent::~schedEvent(){
//	free(m_nodeName);
}

//	***** schedEvent::daysTillNextScheduleDay() *****
int schedEvent::daysTillNextScheduleDay(int wday){
	int j = 0;
	while(m_scheduledDaysOfWeek[wday] == '0'){
		wday += 1;
      if (wday > 6) wday = 0;
		j++;
	}
	return j;
}

//	***** schedEvent::clickOverToNextEvent() *****  	calc the date and time of the next event
time_t schedEvent::clickOverToNextEvent(){

	time_t timeNow = time(NULL);

	//	check if it's a one off event
	if (m_oneOffDateTime != 0){
		if (m_oneOffDateTime < timeNow)
			m_eventDateTime = 1999999999;
		else
			m_eventDateTime = m_oneOffDateTime;
		return m_eventDateTime;
	}

	//	get the current date calendar date and zero out the time
	struct tm *tmDate = localtime(&timeNow);
	tmDate->tm_hour = tmDate->tm_min = tmDate->tm_sec = 0;

	//	get the days to the next scheduled date (might be in the past already)
	tmDate->tm_mday += daysTillNextScheduleDay(tmDate->tm_wday);
	//	add the time
	m_eventDateTime = mktime(tmDate) + m_timeOfDay;

	//	check if the date is in the past
	if (m_eventDateTime < timeNow){
		tmDate->tm_wday += 1;
		if (tmDate->tm_wday > 6) tmDate->tm_wday = 0;
		tmDate->tm_mday += 1 + daysTillNextScheduleDay(tmDate->tm_wday);
		m_eventDateTime = mktime(tmDate) + m_timeOfDay;
	}

	return m_eventDateTime;
}



//	***** schedEvents::schedEvents(MYSQL *dbConn) *****
schedEvents::schedEvents(MYSQL *dbConn){
	m_eventCount = 0;
	reload(dbConn);
}

//	***** schedEvents::nextDateTime()  *****
time_t schedEvents::nextDateTime(){
	if (m_eventCount == 0){
		return 0;
	}
	else{
		return m_events[0]->eventDateTime();
	}
}

//	*****	schedEvent * schedEvents::nextEvent() *****
schedEvent * schedEvents::nextEvent(){
	return m_events[0];
}

//	***** schedEvents::clickOverToNextEvent() *****
time_t schedEvents::clickOverToNextEvent(){

	if (m_eventCount == 0){
		return 0;
	}
	
	m_events[0]->clickOverToNextEvent();
	sortEvents();
	return m_events[0]->eventDateTime();
}
	

// ***** schedEvents::sortEvents() *****
void schedEvents::sortEvents(){
	int i, j;
	schedEvent *temp;

	for (i = 1; i < m_eventCount; i++) {
		j = i;
		temp = m_events[j];
		while (j > 0 && (m_events[j-1]->eventDateTime() > temp->eventDateTime())) {
			m_events[j] = m_events[j-1];
			j--;
		}
		m_events[j] = temp;
	}
}


// ***** schedEvents::~schedEvents() *****
schedEvents::~schedEvents(){
	for (int i = 0; i < m_eventCount; i++){
		delete m_events[i];
	}
}

//	*****	schedEvents::reload()	*****
void schedEvents::reload(MYSQL *dbConn){

	//	delete any previously loaded events
	for (int i = 0; i < m_eventCount; i++){
		delete m_events[i];
	}

	//	load up events from the database
	m_eventCount=0;

	if (mysql_query(dbConn, "SELECT id, deviceId, timeOfDay, Unix_Timestamp(oneOffDateTime), scheduledWeekDays, setStateTo FROM ScheduledEvents WHERE (oneOffDateTime = 0 OR oneOffDateTime > now())")){
      writeLog("%s\n", mysql_error(dbConn));
	}
  
	MYSQL_RES *result = mysql_store_result(dbConn);
	if (result == NULL){
		return;
	}

	MYSQL_ROW row;
	int i=0;
	schedEvent *newEvent;

	while ((row = mysql_fetch_row(result))){
/*		schedDow[0] = (unsigned char)atoi(row[5]);
		schedDow[1] = (unsigned char)atoi(row[6]);
		schedDow[2] = (unsigned char)atoi(row[7]);
		schedDow[3] = (unsigned char)atoi(row[8]);
		schedDow[4] = (unsigned char)atoi(row[9]);
		schedDow[5] = (unsigned char)atoi(row[10]);
		schedDow[6] = (unsigned char)atoi(row[11]);*/
		newEvent = new schedEvent(atoi(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4], *row[5]);
		m_events[i] = newEvent;
		i++;
	}
	m_eventCount=i;
	sortEvents();  
	mysql_free_result(result);
}

