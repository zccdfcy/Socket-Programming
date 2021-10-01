#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h>
using namespace std;

#define SERVERMAINTCPPORT "33863" //define servermain's TCP port number
#define SERVERMAINUDPPORT "32863" //define servermain's UDP port number
#define SERVERAUDPPORT "30863" //define serverA's UDP port number
#define SERVERBUDPPORT "31863" //define serverB's UDP port number
#define LOCAL_HOST "127.0.0.1" //define local host address
#define MAXBUFLEN 1000
#define BACKLOG 10 // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:

void sigchld_handler(int s)
{
// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int main(){
	printf("The Main server is up and running.\n");
	////////////////////////////////  UDP creation ////////////////////////////////////////

	// create UDP socket and bind
	int sockfd;
	struct addrinfo hints, *servinfo, *p, *serverAinfo, *serverBinfo;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	
	//char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	//hints.ai_flags = AI_PASSIVE;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERMAINUDPPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
			continue;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("listener:bind");
			continue;
		}
		break;
	}
	if(p==NULL){
		fprintf(stderr,"listener:failed to bind socket\n");
		return 2;
	}
	addr_len = sizeof their_addr;
	
	///////////////////////////////	TCP creation /////////////////////////////

	// create TCP socket and bind
	int sockfd_TCP;
	struct addrinfo hints_TCP, *servinfo_TCP, *p_TCP;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];

	memset(&hints_TCP, 0, sizeof hints_TCP);
	hints_TCP.ai_family = AF_UNSPEC;
	hints_TCP.ai_socktype = SOCK_STREAM;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERMAINTCPPORT, &hints_TCP, &servinfo_TCP)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for(p_TCP = servinfo_TCP; p_TCP != NULL; p_TCP = p_TCP->ai_next){
		if((sockfd_TCP = socket(p_TCP->ai_family,p_TCP->ai_socktype,p_TCP->ai_protocol))==-1){
			continue;
		}
		if (setsockopt(sockfd_TCP, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}
		if(bind(sockfd_TCP, p_TCP->ai_addr, p_TCP->ai_addrlen) == -1){
			close(sockfd);
			perror("server:bind");
			continue;
		}
		break;
	}
	if(p_TCP==NULL){
		fprintf(stderr,"server:failed to bind socket\n");
		return 2;
	}


	//////////////////////////////////Get serverA country list/////////////////////////////////

	// get serverA info
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(LOCAL_HOST, SERVERAUDPPORT, &hints, &serverAinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// send the country list request to serverA
	const char* msg;
	msg = "Give me your country list!!!!";
	if ((numbytes = sendto(sockfd, msg, strlen(msg)+1, 0, serverAinfo->ai_addr, serverAinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	//printf("main server sent the request to serverA\n");
	
	// wait for serverA's reply(country list)
	char bufA[MAXBUFLEN];
	if((numbytes = recvfrom(sockfd, bufA, sizeof(bufA) , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
	printf("The Main server has received the country list from server A using UDP over port %s\n", SERVERMAINUDPPORT);
	//printf("main server received the country list from serverA: \"%s\"\n", bufA);

	// countryMap
	unordered_map<string,int> countryMap; //country->1(serverA):0(serverB)
	vector<string> A; //serverA's countries
	char* pp = strtok(bufA," "); 
	while(pp){
		string country_A = pp;
		A.push_back(country_A);
		countryMap[country_A] = 1;
		pp = strtok(NULL," ");
	}
	 
	
	///////////////////////////////// Get serverB country list //////////////////////////////
	
	// get serverB info
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(LOCAL_HOST, SERVERBUDPPORT, &hints, &serverBinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// send the country list request to serverB
	if ((numbytes = sendto(sockfd, msg, strlen(msg)+1, 0, serverBinfo->ai_addr, serverBinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	//printf("main server sent the request to serverB\n");
	
	// wait for serverB's reply(country list)
	char bufB[MAXBUFLEN];
	if((numbytes = recvfrom(sockfd, bufB, sizeof(bufB) , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
	printf("The Main server has received the country list from server B using UDP over port %s\n", SERVERMAINUDPPORT);
	//printf("main server received the country list from serverB: \"%s\"\n", bufB);

	// countryMap
	vector<string> B;//serverB's countries
	char* q = strtok(bufB," "); 
	while(q){
		string country_B = q;
		B.push_back(country_B);
		countryMap[country_B] = 0;
		q = strtok(NULL," ");
	}

	//print country lists on screen
	cout<<"ServerA"<<"              "<<"|"<<"ServerB"<<"              "<<endl;
	for(int i=0;i<min(A.size(),B.size());++i){
		char tmp1[] = "                     ";
		strncpy(tmp1,A[i].c_str(),strlen(A[i].c_str()));
		char tmp2[] = "                     ";
		strncpy(tmp2,B[i].c_str(),strlen(B[i].c_str()));
		cout<<tmp1<<"|"<<tmp2<<endl;
	}
	if(A.size()>B.size()){
		for(int i=B.size();i<A.size();++i){
			char tmp1[] = "                     ";
			strncpy(tmp1,A[i].c_str(),strlen(A[i].c_str()));
			char tmp2[] = "                     ";
			cout<<tmp1<<"|"<<tmp2<<endl;
		}
	}else if(A.size()<B.size()){
		for(int i=A.size();i<B.size();++i){
			char tmp1[] = "                     ";
			char tmp2[] = "                     ";
			strncpy(tmp2,B[i].c_str(),strlen(B[i].c_str()));
			cout<<tmp1<<"|"<<tmp2<<endl;
		}
	}
	///////////////////////////////// TCP listen ////////////////////////////

	if (listen(sockfd_TCP, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}	
	int new_fd;


	/////////////////////////// communicate with client /////////////////////////

	//loop
	while(true){
		new_fd = accept(sockfd_TCP, (struct sockaddr *)&their_addr, &addr_len);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,(struct sockaddr *)&their_addr,s, sizeof s);
		//printf("server: got connection from %s\n", s);
		if (!fork()) { // this is the child process
			close(sockfd_TCP); // child doesn't need the listener
			while(true){
				char clientMsg[50];
				if (recv(new_fd, clientMsg, sizeof(clientMsg), 0) == -1)
					perror("recv");
				//cout<<clientMsg<<endl;

				//get country name from clientMsg
				char clientMsg_copy[50];
				strncpy(clientMsg_copy,clientMsg,strlen(clientMsg)+1);
				string c = strtok(clientMsg_copy," ");
				string u = strtok(NULL," ");
				printf("The Main server has received the request on User%s in %s from client using TCP over port %s\n",u.c_str(),c.c_str(),SERVERMAINTCPPORT);
				//cout<<"country:"<<c<<endl;
				//cout<<"clientMsg:"<<clientMsg<<endl;

				////////////////////// get backend server name //////////////////

				//not found this country 
				
				if(!countryMap.count(c)){
					printf("%s does not show up in server A&B\n", c.c_str());
					char notFoundMsg[50];
					sprintf(notFoundMsg,"%s: Not found",c.c_str());
					if (send(new_fd, notFoundMsg, strlen(notFoundMsg)+1, 0) == -1)
						perror("send");
					printf("The Main Server has sent “%s: Not found” to client using TCP over port %s\n",c.c_str(),SERVERMAINTCPPORT);
					continue;
				}
				// serverA or serverB ?
				struct addrinfo* backendServerinfo;
				backendServerinfo = countryMap[c]==1? serverAinfo:serverBinfo;
				char bs = countryMap[c]==1? 'A':'B';
				
				/////////////////////////// send to backend server ////////////////////////

				printf("%s shows up in server%c\n",c.c_str(),bs);
				if ((numbytes = sendto(sockfd, clientMsg, strlen(clientMsg)+1, 0,
					 backendServerinfo->ai_addr, backendServerinfo->ai_addrlen)) == -1){
					perror("talker: sendto");
					exit(1);
				}
				printf("The Main Server has sent request from User%s to server%c using UDP over port %s\n",u.c_str(),bs,SERVERMAINUDPPORT);


				///////////////////////// wait for backend servers' reply ////////////////////

				char backendServerMsg[50];
				if((numbytes = recvfrom(sockfd, backendServerMsg, sizeof(backendServerMsg) , 0,
					 (struct sockaddr *)&their_addr, &addr_len))==-1){
					perror("recvfrom");
					exit(1);
				}
				char tmp[50];
				sprintf(tmp,"User%s not found",u.c_str());
				string s = tmp;
				if(s==backendServerMsg)
					printf("The Main server has received %s from server%c\n",tmp,bs);
				else
					printf("The Main server has received searching result(s) of User%s from server%c\n",u.c_str(), bs);	
			
				//send the recommended user to client
				//const char* backendServerMsg = "ok";
				if (send(new_fd, backendServerMsg, strlen(backendServerMsg)+1, 0) == -1)
					perror("send");
				if(s==backendServerMsg)
					printf("The Main Server has sent error to client using TCP over port%s\n", SERVERMAINTCPPORT);
				else
					printf("The Main Server has sent searching result(s) to client using TCP over port%s\n",SERVERMAINTCPPORT);
			}
			
		}
		close(new_fd);
	}
	
	return 0;
}
