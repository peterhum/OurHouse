//	*******Using select() for I/O multiplexing
//	http://www.tenouk.com/Module41.html

#include "OurHouseServer.h"

tcpConnections::tcpConnections(){
	
	int i;
	int yes = 1;

	//	lastUsed is an array of time_t the same size as fdmax
	for (i=0; i<100; i++)
		lastUsed[i]=0;

	writeLog("OurHouseServer begin start up.\n");

	//	clear the master and temp sets
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//	get the listener
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		writeLog("Server-socket() error\n");
		exit(1);
	}
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		writeLog("Server-setsockopt() error address already in use\n");
		exit(1);
	}

   //	bind
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);
	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1){
		writeLog("Server-bind() error\n");
		exit(1);
	}

	//	listen
	if(listen(listener, 10) == -1){
		writeLog("Server-listen() error\n");
		exit(1);
	}

	//	add the listener to the master set
	FD_SET(listener, &master);
	//	keep track of the biggest file descriptor. so far, it's this one
	fdmax = listener;

	writeLog("OurHouseServer start up OK.\n");
}

//	*****	tcpConnections::check()  *****
int tcpConnections::check(){

	int i;
	unsigned int addrlen;
	time_t timeNow;

	//	copy fd list
	read_fds = master;
	//	set select timeout to one second
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	//	timed out select, waiting for data to arrive
	i = select(fdmax+1, &read_fds, NULL, NULL, &timeout);
	if (i == -1){
		writeLog("Server-select() error\n");
		return 0;
	}
	if (i == 0) return 0;

	nbytes = 0;

	//	run through the existing connections looking for data to be read
	for(i = 0; i <= fdmax; i++){
		if(FD_ISSET(i, &read_fds)){
			//	something arrived
			if(i == listener){
				//	handle new connections
				addrlen = sizeof(clientaddr);
				if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1){
					writeLog("Server-accept() error\n");
				}
				else{
					FD_SET(newfd, &master); 	//	add to master set
					if(newfd > fdmax){
						fdmax = newfd;				//	keep track of the maximum
					}
					time(&(lastUsed[newfd]));	//	initialise the last used time
					//	don't log php connections. 16777343 is the unsigned long equivalent of 127.0.0.1
					if (clientaddr.sin_addr.s_addr != 16777343)
						writeLog("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
					if(send(newfd, "CN", 3, 0) == -1)
						writeLog("Send() error, socket %d\n", newfd);
				}
 			}
			else{
				//	handle data from an existing connection
				if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0){
					//	got error or connection closed by client
					//	for some reason if on raspberry the php socket_close call results in an error
					//	this should only happen from localhost
					//	16777343 is the unsigned long equivalent of 127.0.0.1
					if(nbytes == 0){
						//	connection closed
						writeLog("Socket %d hung up\n", i);
					}
					else{
						if (clientaddr.sin_addr.s_addr != 16777343)
							writeLog("Recv() error, socket %d\n", i);
					}
					//	close it
					close(i);
					//	remove from master set
					FD_CLR(i, &master);
				}
				else{
					//	we got some data from a client
					//	end the buf
					buf[nbytes]=0;
					m_connectionId = i;
					//	note the time of the last use
					time(&(lastUsed[i]));
					return 1;
				}
			}
		}
		//	check if it's more than 60 minutes since last use. If so, close it
		time(&timeNow);
		if ((lastUsed[i] != 0) && (difftime(timeNow, lastUsed[i]) > 3600.0) && (i > listener)){
			writeLog("Socket %d timed out and was closed\n", i);
			close(i);
			FD_CLR(i, &master);
			lastUsed[i]=0;
		}
	}

	return 0;
}


//	*****	tcpConnections::message()  *****
char * tcpConnections::message(){
	return buf;
}


//	*****	tcpConnections::connectionId()  *****
int tcpConnections::connectionId(){
	return m_connectionId;
}

//	*****	tcpConnections::sendMsg()  *****
void tcpConnections::sendMsg(char *msg){
	//	send msg to current connection idx
	if(send(m_connectionId, msg, strlen(msg), 0) == -1)
		writeLog("Send() error, socket %i",m_connectionId);
}
	

//	*****	tcpConnections::sendMsg()  *****
void tcpConnections::sendMsg(int connectionId, char *msg){
	//	send msg to current connection idx
	if(send(connectionId, msg, strlen(msg), 0) == -1)
		writeLog("Send() error, socket %i", connectionId);
}

void tcpConnections::closeConnection(){
	//	closes the current connection
	close(m_connectionId);
	//	remove from master set
	FD_CLR(m_connectionId, &master);
	m_connectionId = -1;
}

