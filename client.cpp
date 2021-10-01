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
#define LOCAL_HOST "127.0.0.1" //define local host address
#define MAXDATASIZE 100


int main(){
	cout<<"Client is up and running"<<endl;
	int sockfd;
	struct addrinfo hints, *servinfo, *p, *serverAinfo;
	int rv;
	int numbytes;
	
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(LOCAL_HOST, SERVERMAINTCPPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// create TCP socket
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
			perror("client: socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if(p==NULL){
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family,(struct sockaddr *)p->ai_addr,s, sizeof s);
	//printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo);
	//input request
	while(true){
		string countryName;
		string userIDorg;
		
		cout<<"Enter country name:";
		cin>>countryName;
		cout<<"Enter user ID:";
		cin>>userIDorg;
		//cout<<"countryName:"<<countryName<<" userIDorg:"<<userIDorg<<endl;

		string msg = countryName+" "+userIDorg;
		//cout<<"msg: "<<msg<<endl;

		//send request to main server
		if (send(sockfd, msg.c_str(), strlen(msg.c_str())+1, 0) == -1){
			perror("send");
			exit(1);
		}
		printf("Client has sent User%s and %s to Main Server using TCP\n", userIDorg.c_str(),countryName.c_str());
		
		//wait for reply from main server
		char buf[MAXDATASIZE];
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buf[numbytes] = '\0';
		char tmp1[50];
		char tmp2[50];
		sprintf(tmp1,"%s: Not found",countryName.c_str());

		sprintf(tmp2,"User%s not found",userIDorg.c_str());
		string s1 = buf;
		if(s1==tmp1){
			cout<<tmp1<<endl;
		}else if(s1==tmp2){
			cout<<tmp2<<endl;
		}else{
			printf("Client has received results from Main Server:User%s is possible friend of User%s in %s\n", buf,userIDorg.c_str(),countryName.c_str());
		}
		//printf("client: received '%s'\n",buf);
	}

	close(sockfd);
	return 0;
}