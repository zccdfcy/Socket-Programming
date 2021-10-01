
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

#define SERVERMAINUDPPORT "32863" //define servermain's UDP port number
#define SERVERBUDPPORT "31863" //define serverB's UDP port number
#define LOCAL_HOST "127.0.0.1" //define local host address
#define MAXBUFLEN 200
#define MAXLINESIZE 20000
#define MAXUSERS 120

// give a set which contains some users' re-indexed ID in a country and 
// the original ID -> re-indexed ID map of this country
//return the smallest original ID of these users in set
string findSamllestOrgID(set<int>& s,unordered_map<string,int>& map){
	if(s.empty()) return "";
	set<int> newSet;
	for(int i:s){
		for(auto pair:map){
			if(pair.second==i) newSet.insert(stoi(pair.first));
		}
	}
	char ans[20];
	sprintf(ans, "%d", *newSet.begin());
	return string(ans);

}


// give a country and a user, calculate the recommeded user
string recommendate(string countryName,string userIDorg,
	unordered_map<string,vector<vector<int>>>& dict,
	unordered_map<string,unordered_map<string,int>>& mapID,
	unordered_map<string,int>& userNUM){
	if(!mapID[countryName].count(userIDorg)){
		return "No such user!";
	}
	int n = userNUM[countryName];
	
	//Case 1-a: only one user in this country
	if(n==1) return "None";
	
	//get re-indexed ID of user u
	int userIDrein = mapID[countryName][userIDorg];
	//get all unconnected user into an set unconnected
	set<int> unconnected;
	for(int i=0;i<n;++i){
		if(i==userIDrein) continue;
		if(dict[countryName][userIDrein][i]==0){//unconnected user
			unconnected.insert(i);
		}
	}
	
	//Case 1: connected all other users, no unconnected user
	if(unconnected.empty()) return "None";
	

	//Case 2: user u has at least one unconnected user
	// For each user i in unconnected, count the number of common neighbors between u and i.
	int MAXCOMNEI = 0;
	set<int> res;
	for(int i:unconnected){
		//count the number of common neighbors
		int count = 0;
		for(int j=0;j<n;++j){
			if(dict[countryName][userIDrein][j]==1&&dict[countryName][i][j]==1)
				count++;
		}
		//cout<<"i:"<<i<<" commonneis:"<<count<<endl;
		if(count>MAXCOMNEI){
			MAXCOMNEI = count;
			res = {};
			res.insert(i);
		}else if(count==MAXCOMNEI&&count>0){
			res.insert(i);
		}
	}
	
	//Case 2-a no unconnected user i shares any common neighbor with u
	string recomUser = "";
	if(MAXCOMNEI==0){
		//count degree of each unconnected user i find highest degree set
		int MAXDEGREE = 0;
		set<int> highestDegreeSet;
		for(int i:unconnected){
			int degree = 0;
			for(int j=0;j<n;++j) degree+=dict[countryName][i][j];
			//cout<<"i:"<<i<<" degree:"<<degree<<endl;
			if(degree>MAXDEGREE){
				MAXDEGREE = degree;
				highestDegreeSet = {};
				highestDegreeSet.insert(i);
			}else if(degree==MAXDEGREE){
				highestDegreeSet.insert(i);
			}
		}
		
		recomUser = findSamllestOrgID(highestDegreeSet,mapID[countryName]);
	}
	else{//Case 2-b some unconnected user i shares some common neighbors with u
		recomUser = findSamllestOrgID(res,mapID[countryName]);
	}
	return recomUser;
}

int main(){
	/////////////////////////////////// 1.Computation ////////////////////////////////////////
	//read the file data2.txt
	ifstream data2;
	data2.open("data2.txt");
	if(!data2) {
		fprintf(stderr,"not find file data2.txt\n");
    	exit(1);
	}
	string countryList = "";
	char line_tmp[MAXLINESIZE];
	int ID = 0;
	
	unordered_map<string,vector<vector<int>>> dict; //country:matrix
	unordered_map<string,unordered_map<string,int>> mapID; //country:userID->ID
	unordered_map<string,int> userNUM;//country->users num in this country
	string country = "";
	vector<vector<int>> matrix_tmp(MAXUSERS,vector<int>(MAXUSERS,0));
	//get data from data2.txt line by line
	while(data2.getline(line_tmp, MAXLINESIZE)){
		
		//get country name
		if(isalpha(line_tmp[0])){
			if(country!=""){
				dict[country] = matrix_tmp;
				userNUM[country] = ID;
			} 
			country = strtok(line_tmp," ");;
			if(!isalpha(country[country.length()-1])){
				country = country.substr(0,country.length()-1);
			}
			countryList += (country + " ");
			ID = 0;
			matrix_tmp = vector<vector<int>>(MAXUSERS,vector<int>(MAXUSERS,0));
			continue;
		}
		
		// create mapID
		string userID = strtok(line_tmp," ");//fisrt user in this line
		
		if(!isdigit(userID[userID.length()-1])){
			userID = userID.substr(0,userID.length()-1);
		}
		//cout<<"userID:"<<userID<<endl;
		//assign new ID for userID
		if(!mapID[country].count(userID)){
			mapID[country][userID] = ID;
			//cout<<"userID:"<<userID<<"-ID:"<<ID<<endl;
			ID++;
		}
		//traverse neibors and assign new ID and create dict 
		char* p = strtok(NULL," ");//next neibor
		while(p){
			string nei = p;
			if(!isdigit(nei[nei.length()-1])){
				nei = nei.substr(0,nei.length()-1);
			}
			//cout<<"nei:"<<nei<<endl;
			//assign new ID
			if(!mapID[country].count(nei)){
				mapID[country][nei] = ID;
				//cout<<"nei:"<<nei<<"-neiID:"<<ID<<endl;
				ID++;
			}
			//create dict
			matrix_tmp[mapID[country][userID]][mapID[country][nei]] = 1;
			matrix_tmp[mapID[country][nei]][mapID[country][userID]] = 1;
			p = strtok(NULL," ");
		}

	}
	dict[country] = matrix_tmp; //last country
	userNUM[country] = ID;
	
	char msg_countryList[countryList.length()+1];
	strncpy(msg_countryList, countryList.c_str(), strlen(countryList.c_str())+1);
	//printf("countryList:%s\n", countryList.c_str());
	//cout<<countryList.length()<<" "<<strlen(countryList.c_str())<<endl;
	//cout<<strlen(msg_countryList)<<endl;

	////////////////////////////////////////// 2.Communication /////////////////////////////////
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	//struct sockaddr* their_addr;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char buf[MAXBUFLEN];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERBUDPPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	//create UDP socket and bind
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
		fprintf(stderr,"talker:failed to create socket\n");
		return 2;
	}
	printf("The server B is up and running using UDP on port %s\n", SERVERBUDPPORT);

	//send country list to main server
	addr_len = sizeof their_addr;

	while(true){
		if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
			perror("recvfrom");
			exit(1);
		}
		string msg = buf;
		//
		if(msg == "Give me your country list!!!!"){
			//printf("serverB received the request from main server:%s\n",buf);
			//cout<<strlen(msg_countryList)<<endl;
			//send country list back to main server
			if ((numbytes = sendto(sockfd, msg_countryList, strlen(msg_countryList)+1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
				perror("talker: sendto");
				exit(1);
			}
			printf("The server B has sent a country list to Main Server\n");
		}
		else{ //
			string countryName = strtok(buf," ");
			string userIDorg = strtok(NULL," ");
			printf("The server B has received request for finding possible friends of User%s in %s\n", userIDorg.c_str(), countryName.c_str());
			//cout<<"countryName:"<<countryName<<" userIDorg:"<<userIDorg<<endl;
			
			if(!mapID[countryName].count(userIDorg)){
				printf("User%s does not show up in %s\n", userIDorg.c_str(), countryName.c_str());
				char notFoundUser[100];
				sprintf(notFoundUser,"User%s not found",userIDorg.c_str());
				if ((numbytes = sendto(sockfd, notFoundUser, strlen(notFoundUser)+1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
					perror("talker: sendto");
					exit(1);
				}
				printf("The server B has sent “User%s not found” to Main Server\n", userIDorg.c_str());
			}else{
				printf("The server B is searching possible friends for User%s  …\n", userIDorg.c_str());
				string recommendedUser = recommendate(countryName,userIDorg,dict,mapID,userNUM);
				printf("Here are the results: User%s\n", recommendedUser.c_str());
				//send recommended user to the main server
				if ((numbytes = sendto(sockfd, recommendedUser.c_str(), strlen(recommendedUser.c_str())+1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
					perror("talker: sendto");
					exit(1);
				}
			printf("The server B has sent the result(s) to Main Server\n");
			}
		}
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}