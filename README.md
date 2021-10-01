USC EE450 Socket Programming Project

In this project, I implemented a simple application to generate recommendations based on user queries, which consists of four parts, Client, the Main server, Backend server A, Backend server B. the client sends queries to the main server, then the main server further sends those queries to their corresponding backend servers (A or B). The backend server will perform local computation to generate recommendations based on its own database and sends recommendations back to the main server. The main server then sends recommendations back to the corresponding client. 
d. Code files:
	1. client.cpp
		-create TCP connection with main server
		-send country name an user ID to main server over TCP
		-receive recommendation from main server
	2. servermain.cpp
		-create UDP and TCP sockets
		-ask coutry lists from server A and B over UDP
		-create a country map to store coutries from server A and B
		-receive queries from clients over TCP
		-decode queries and send to server A or B over UDP
		-receive recommendation from backend server and send back to client over TCP
	3.serverA.cpp
		-read data1.txt and constrcut country graphs
		-create UDP socket
		-send the country list to main server over UDP
		-receive queries from main server over UDP
		-computation and send back the results to main server over UDP
	4.serverB.cpp
		-read data2.txt and constrcut country graphs
		-create UDP socket
		-send the country list to main server over UDP
		-receive queries from main server over UDP
		-computation and send back the results to main server over UDP
e. the format of all the messages exchanged is char*.
	1. client.cpp
		-queries sending to main server("msg.c_str()"):"Country name"+" "+"User ID"
		-recommendation received from main server("buf"): "User ID"
	2. servermain.cpp
		-ask coutry lists from server A and B("msg"): "Give me your country list!!!!"
		-queries sending to server A or B("clientMsg"):"Country name"+" "+"User ID" 
		-recommendation sending back to client("notFoundMsg"): "Country name: Not found"
		-recommendation sending back to client("backendServerMsg"):"User ID" 
	3.serverA.cpp
		-coutry list sending to the main server("msg_countryLit"): "coutry1"+" "+"coutry2"+" "...
		-recommendation sending to main server("notFoundUser"): "User# not found"
		-recommendation sending to main server("recommendedUser.c_str()"): "User ID"
	4.serverB.cpp
		-coutry list sending to the main server("msg_countryLit"): "coutry1"+" "+"coutry2"+" "...
		-recommendation sending to main server("notFoundUser"): "User# not found"
		-recommendation sending to main server("recommendedUser.c_str()"): "User ID"
f.This project must run on the given Ubuntu environment and the format of data1.txt and data2.txt must satisfy the assumptions in the project pdf. 
g.Reused code: Beej's Code: http://www.beej.us/guide/bgnet/
	-I used it in create TCP and UDP connections parts. 
