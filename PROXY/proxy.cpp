#include <chrono>
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include "proxy_parse.h"
#include "md5.h"
#include "stuff.h"
#include <string>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <vector>
#include <memory>

#include <list>
#include <iterator>
#include <mutex> // std::mutex
#include <csignal>

#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

#include <pthread.h>

using namespace std;
pthread_mutex_t m;
string Spid;

int MSGSIZE = 100;
int CACHESIZE = 100;
int fdCacheManager[2];  
int cachingStrategy;

char *convert_Request_to_string(struct ParsedRequest *req)
{

	if (ParsedHeader_get(req, "Range") != NULL)
	{
		fs(12, "\t!!Due To Cache Complexity Pratial Request Is Not Allowed!!\t", false);
		exit(1);
	}

	/* Set headers */
	ParsedHeader_set(req, "Host", req->host);
	ParsedHeader_set(req, "Connection", "close");
	ParsedHeader_modify(req, "Cache-Control", "no-cache");
	ParsedHeader_modify(req, "If-None-Match", "AA");
	ParsedHeader_modify(req, "If-Modified-Since", "AA");
	ParsedHeader_modify(req, "Pragma", "no-cache");
	int iHeadersLen = ParsedHeader_headersLen(req);

	char *headersBuf;

	headersBuf = (char *)malloc(iHeadersLen + 1);

	if (headersBuf == NULL)
	{
		fprintf(stderr, " Error in memory allocation  of headersBuffer ! \n");
		exit(1);
	}

	ParsedRequest_unparse_headers(req, headersBuf, iHeadersLen);
	headersBuf[iHeadersLen] = '\0';

	int request_size = strlen(req->method) + strlen(req->path) + strlen(req->version) + iHeadersLen + 4;

	char *serverReq;

	serverReq = (char *)malloc(request_size + 1);

	if (serverReq == NULL)
	{
		fprintf(stderr, " Error in memory allocation for serverrequest ! \n");
		exit(1);
	}

	serverReq[0] = '\0';
	strcpy(serverReq, req->method);
	strcat(serverReq, " ");
	strcat(serverReq, req->path);
	strcat(serverReq, " ");
	strcat(serverReq, req->version);
	strcat(serverReq, "\r\n");
	strcat(serverReq, headersBuf);

	free(headersBuf);

	return serverReq;
}

int createserverSocket(char *pcAddress, char *pcPort)
{
	struct addrinfo ahints;
	struct addrinfo *paRes;

	int iSockfd;

	/* Get address information for stream socket on input port */
	memset(&ahints, 0, sizeof(ahints));
	ahints.ai_family = AF_UNSPEC;
	ahints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(pcAddress, pcPort, &ahints, &paRes) != 0)
	{
		fprintf(stderr, " Error in server address format ! \n");
		exit(1);
	}

	/* Create and connect */
	if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0)
	{
		fprintf(stderr, " Error in creating socket to server ! \n");
		exit(1);
	}
	if (connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0)
	{
		fprintf(stderr, " Error in connecting to server ! \n");
		exit(1);
	}

	/* Free paRes, which was dynamically allocated by getaddrinfo */
	freeaddrinfo(paRes);

	return iSockfd;
}

void writeToserverSocket(const char *buff_to_server, int sockfd, int buff_length)
{

	string temp;

	temp.append(buff_to_server);

	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length)
	{
		if ((senteach = send(sockfd, (void *)(buff_to_server + totalsent), buff_length - totalsent, 0)) < 0)
		{
			fprintf(stderr, " Error in sending to server ! \n");
			exit(1);
		}
		totalsent += senteach;
	}
}

void writeToclientSocket(const char *buff_to_server, int sockfd, int buff_length)
{

	string temp;

	temp.append(buff_to_server);

	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length)
	{
		if ((senteach = send(sockfd, (void *)(buff_to_server + totalsent), buff_length - totalsent, 0)) < 0)
		{
			fprintf(stderr, " Error in sending to server ! \n");
			fs(16, Spid + " ===   ERRR2");
			exit(1);
		}
		totalsent += senteach;
	}
}
string rFileName;
void writeFileToClient(int Clientfd, string relativeFileName)
{
	int MAX_BUF_SIZE = 1024 * 30;
	int iRecv = 0;
	char buf[MAX_BUF_SIZE];
	std::ifstream infile(relativeFileName, ios::in | ios::binary);

	do
	{
		infile.read(buf, MAX_BUF_SIZE);
		iRecv = infile.gcount();
		writeToclientSocket(buf, Clientfd, iRecv);

	} while (iRecv > 0);

	infile.close();
	write(fdCacheManager[1], (relativeFileName).c_str(), MSGSIZE);
}
void writeToClient(int Clientfd, int Serverfd, string relativeFileName)
{
	int MAX_BUF_SIZE = 1024 * 30;
	long totalRccv = 0;
	int iRecv;
	char buf[MAX_BUF_SIZE];
	bool isResponseEqual200 = false;
	bool ft = true;

	char bufT0[200];

	std::ofstream outfile(relativeFileName, ios::out | ios::binary);
	while ((iRecv = recv(Serverfd, buf, MAX_BUF_SIZE, 0)) > 0)
	{
		if (ft)
		{
			strncpy(bufT0, buf, 200);bufT0[199]='.';bufT0[198]='.';bufT0[197]='.';

			if (strncmp(buf, "HTTP/1.1 200 OK", 15) == 0)
			{
				isResponseEqual200 = true;
				fs(11, Spid + " Response is Okay");
				fs(11, bufT0);
			}
			else
			{
				fs(12, Spid + " Bad Response");
				fs(12, bufT0);
			}
		}

		ft = false;
		if (isResponseEqual200)
			outfile.write(buf, iRecv);
		writeToclientSocket(buf, Clientfd, iRecv); // writing to client
		memset(buf, 0, sizeof buf);
		totalRccv += iRecv;
	}
	outfile.close();
	rFileName = "";
	fs(17, relativeFileName + Spid + " ===   done -- total totalRccv: " + to_string(totalRccv));
	/* Error handling */
	if (iRecv < 0)
	{
		fprintf(stderr, "Yo..!! Error while recieving from server ! \n");
		remove(relativeFileName.c_str());
		exit(1);
	}

	if (!isResponseEqual200)
		remove(relativeFileName.c_str());
	if ((isResponseEqual200) && (iRecv == 0))
	{
		rename(relativeFileName.c_str(), (relativeFileName + "_Done").c_str());
		write(fdCacheManager[1], (relativeFileName + "_Done").c_str(), MSGSIZE);
	}
}
int count0;

void *datafromclient(void *sockid)
{
	std::stringstream ss;
	int MAX_BUFFER_SIZE = 5000;

	char buf[MAX_BUFFER_SIZE];

	int newsockfd = *((int *)sockid);

	char *request_message; // Get message from URL

	request_message = (char *)malloc(MAX_BUFFER_SIZE);

	if (request_message == NULL)
	{
		fprintf(stderr, " Error in memory allocation ! \n");
		exit(1);
	}

	request_message[0] = '\0';

	int total_recieved_bits = 0;

	while (strstr(request_message, "\r\n\r\n") == NULL)
	{ // determines end of request

		int recvd = recv(newsockfd, buf, MAX_BUFFER_SIZE, 0);

		if (recvd < 0)
		{
			fprintf(stderr, " Error while recieving ! \n");
			exit(1);
		}
		else if (recvd == 0)
		{
			break;
		}
		else
		{

			total_recieved_bits += recvd;

			/* if total message size greater than our string size,double the string size */

			buf[recvd] = '\0';
			if (total_recieved_bits > MAX_BUFFER_SIZE)
			{
				MAX_BUFFER_SIZE *= 2;
				request_message = (char *)realloc(request_message, MAX_BUFFER_SIZE);
				if (request_message == NULL)
				{
					fprintf(stderr, " Error in memory re-allocation ! \n");
					exit(1);
				}
			}
		}

		strcat(request_message, buf);
	}

	struct ParsedRequest *req; // contains parsed request

	req = ParsedRequest_create();

	if (ParsedRequest_parse(req, request_message, strlen(request_message)) < 0)
	{
		ss.str("");
		ss << "Error in request message..only http and get with headers are allowed " << req->method << req->host << req->path;
		fs(12, "\t Err Only Http Get \t", false);
		exit(0);
	}

	if (req->port == NULL) // if port is not mentioned in URL, we take default as 80
		req->port = (char *)"80";

	/*final request to be sent*/

	char *browser_req = convert_Request_to_string(req);

	ss.str("");
	ss << " request_message: Orginal " << count0 << " ======>" << request_message << "<============";
	fs(6, Spid + ss.str());
	ss.str("");
	ss << "browser_req: Changed " << count0 << " ======>" << browser_req << "<============";
	fs(3, Spid + ss.str());

	ss.str("");
	ss << req->host << req->path;
	fs(10, Spid + " host---path  : ======>\r\n  " + ss.str());
	string fileName = md5(ss.str());
	fs(14, Spid + "Md5Hash: " + fileName);

	int cond = 0;
	{
		pthread_mutex_lock(&m);
		cond = (fileExistInCache(fileName)) ? 1 : cond;
		cond = fileExistInCache(fileName + "_Done") ? 2 : cond;
		if (cond == 0)
		{
			rFileName = "./CACHE/" + fileName;
			std::ofstream outfile("./CACHE/" + fileName, ios::out | ios::binary);
			rFileName = "./CACHE/" + fileName;
			outfile.open("./CACHE/" + fileName, ios::out);
			outfile.close();
		}
		pthread_mutex_unlock(&m);
	}

	if (cond == 1)
	{
		fs(13, Spid + " CACHE HIT But Last Request Hasen't Done Yet Wait " + fileName);
		int lCount = 0;
		while ((fileExistInCache(fileName) == true) && (fileExistInCache(fileName + "_Done") == false) && (lCount++ < 5))
			sleep(1);

		cond = fileExistInCache(fileName + "_Done") ? 2 : 0;
	}

	if (cond == 2)
	{

		fs(12, " pid: " + to_string(getpid()) + " CACHE HIT " + fileName);
		writeFileToClient(newsockfd, "./CACHE/" + fileName + "_Done");
		close(newsockfd); // close the sockets
	}
	if (cond == 0)
	{
		fs(11, " pid: " + to_string(getpid()) + " CACHE MISS " + fileName);
		int iServerfd;

		iServerfd = createserverSocket(req->host, req->port);

		writeToserverSocket(browser_req, iServerfd, total_recieved_bits);
		writeToClient(newsockfd, iServerfd, "./CACHE/" + fileName);

		ParsedRequest_destroy(req);

		close(newsockfd); // close the sockets
		close(iServerfd);
	}
	//  Doesn't make any sense ..as to send something
	int y = 3;
	int *p = &y;
	return p;
}


int MainPrgPid, MainSockfd;
void signalHandler(int signum)
{
	//	cout << Spid<< "  Interrupt signal (" << signum << ") received.\n";
	if (signum == 13)
	{
		if (rFileName != "")
		{
			fs(18, "\t\tTrying To Remove Temp File " + rFileName + "\t\t", false);
			remove(rFileName.c_str());
		}
		exit(signum);
	}
	
	if (signum == 2)
	{
		if (MainPrgPid == getpid())
			close(MainSockfd);

		exit(signum);
	}
}

int main(int argc, char *argv[])
{

	MainPrgPid = getpid();
	Spid = Spid = " pid: " + to_string(getpid()) + "  ";
	for (int i = 1; i < 20; i++)
		signal(i, signalHandler);

	if (argc < 4)
	{
		fs(12, " Provide A Port !  And Also Chaching Strategy  and cacheMaximon Size \n 1:Fifo \n 2:LRU \n 3:LFU   \n ./proxy <portnumber> <Chaching Strategy> <maxCacheSize> \n");
		return 1;
	}
	CACHESIZE = atoi(argv[3]);
	if (CACHESIZE < 5 || CACHESIZE > 3000)
	{
		fs(12, "CACHE SIZE should be between 5 and 3000");
		return 1;
	}

	int cachingStrategy = atoi(argv[2]);
	if (cachingStrategy < 1 || cachingStrategy > 3)
	{
		fs(12, "Chaching Strategy should be between 1 and 3 \n 1:Fifo \n 2: LRU \n 3:LFU");
		return 1;
	}


	if (pipe(fdCacheManager) == -1)
	{
		fprintf(stderr, "Pipe Failed");
		return 1;
	}
	pid_t p;
	p = fork();

	if (p < 0)
	{
		fprintf(stderr, "fork Failed");
		return 1;
	}
	// child process 
	if (p == 0)
	{

		list<fileAccessData> ls0;
		struct stat t_stat;

		const auto &directory_path = std::string("./CACHE");
		const auto &files = GetDirectoryFiles(directory_path);
		for (const auto &file : files)
		{

			fileAccessData t0;
			t0.fileName = "./CACHE/" + file;
			stat(t0.fileName.c_str(), &t_stat);
			struct tm *timeinfo = gmtime(&t_stat.st_ctime);
			t0.firstTimeCreated = (timeinfo->tm_sec+(timeinfo->tm_min*60)+(timeinfo->tm_hour*60*60)+(timeinfo->tm_mday *24*60*60)+(timeinfo->tm_mon *24*30*60*60));
			t0.lastTimeAccessed = timeSinceEpochMillisec();
			t0.numberOfTimeAccessed = 0;
			ls0.push_front(t0);
		}

		int missN=0,hitN=0;
		int nbytes;
		char inbuf[MSGSIZE];
		while (true)
		{
			fileAccessData t0;

			if ((missN + hitN) > 0)
				fs(17, "Cache Manager " + Spid + "\t MissN: " + to_string(missN) + "\t HitN: " + to_string(hitN) + "\t MissRate: " + to_string(int(100 * missN / (missN + hitN))) + "%\t HitRate: " + to_string(int(100 * hitN / (missN + hitN))) + "%");
			if ((nbytes = read(fdCacheManager[0], inbuf, MSGSIZE)) > 0)
			{
				bool found=false;
				
				for (auto it = ls0.begin(); it != ls0.end(); ++it)
					if (  strncmp(  (it->fileName).c_str() ,  inbuf ,  (it->fileName).size()   ) == 0)
					{
						found = true;
						
						it->lastTimeAccessed = timeSinceEpochMillisec();
						it->numberOfTimeAccessed += 1;

						fs(14, "Cache Manager Hit Cache" + Spid + inbuf
						+"\n  numberOfTimeAccessed: "+to_string(it->numberOfTimeAccessed)+
						"  lastTimeAccessed: "+to_string(it->lastTimeAccessed)+
						"  firstTimeCreated: "+to_string(it->firstTimeCreated));
						hitN++;
						break;
					}

				if (!found)
				{
					t0.fileName = inbuf;
					t0.firstTimeCreated = timeSinceEpochMillisec();
					t0.lastTimeAccessed = timeSinceEpochMillisec();
					t0.numberOfTimeAccessed = 1;
					ls0.push_front(t0);
					fs(15, "Cache Manager MISS " + Spid + inbuf
						+"\n  numberOfTimeAccessed: "+to_string(t0.numberOfTimeAccessed)+
						"  lastTimeAccessed: "+to_string(t0.lastTimeAccessed)+
						"  firstTimeCreated: "+to_string(t0.firstTimeCreated));
						missN++;
				}

				while (ls0.size() > (unsigned)CACHESIZE)
				{
					auto  t1= ( ls0.begin());
					for (auto it = ls0.begin(); it != ls0.end(); ++it)
						if (t1->getData(cachingStrategy) >it->getData(cachingStrategy))
							t1=it;
					
					try
					{
						fs(8, "REMOVING FROM LIST DUE TO CAPACITY LIMIT! \n"+ t1->fileName +"\t  value by cachingStrategy: "+to_string(t1->getData(cachingStrategy)));
						remove(t1->fileName.c_str());
						
						ls0.erase(t1);
					}
					catch(const std::exception& e)
					{
						fs(9,e.what());
						std::cerr << e.what() << '\n';
					}
					
				}
			}
		}
	}
	// Parent process
	if (p > 0)
	{
		count0 = 0;
		int newsockfd;

		struct sockaddr_in serv_addr;
		struct sockaddr cli_addr;
		MainSockfd = socket(AF_INET, SOCK_STREAM, 0); // create a socket

		if (MainSockfd < 0)
		{
			fprintf(stderr, "SORRY! Cannot create a socket ! \n");
			return 1;
		}

		memset(&serv_addr, 0, sizeof serv_addr);

		int portno = atoi(argv[1]); // argument through terminal

		serv_addr.sin_family = AF_INET;			// ip4 family
		serv_addr.sin_addr.s_addr = INADDR_ANY; // represents for localhost i.e 127.0.0.1
		serv_addr.sin_port = htons(portno);

		int binded = bind(MainSockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		if (binded < 0)
		{
			fprintf(stderr, "Error on binding! \n");
			return 1;
		}

	 	fs(11,"\t\t                  Welcome                  \t\t");
		
		listen(MainSockfd, 100); // can have maximum of 100 browser requests

		int clilen = sizeof(struct sockaddr);

		while (1)
		{

			/* A browser request starts here */

			newsockfd = accept(MainSockfd, &cli_addr, (socklen_t *)&clilen);

			if (newsockfd < 0)
			{
				fprintf(stderr, "ERROR! On Accepting Request ! i.e requests limit crossed \n");
			}

			count0++;
			int pid = fork();

			if (pid == 0)
			{
				Spid = " pid: " + to_string(getpid()) + "  ";
				datafromclient((void *)&newsockfd);
				close(newsockfd);
				_exit(0);
			}
			else
			{
				close(newsockfd); // pid =1 parent process
			}
		}

		close(MainSockfd);
	}
	return 0;
}
