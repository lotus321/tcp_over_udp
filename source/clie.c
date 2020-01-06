#include "tcp.h"
#include <pthread.h>
//#define _IP_ADDR "127.0.0.1"
#define _PORT 8888



char FILE_NAME[100];
TCP_HEADER m_sHeader;
struct sockaddr_in serv_addr;
struct sockaddr_in from_serv;
int sock;

int filesize;
int addr_size;




///////SLIDE WINDOW/////////////
PWINDOW wnd;
int win_N,win_BASE;
int win_cur;
int segn;
/////////////////////////////

int clientConnectonTermination()
{
	TCP_HEADER terTCP;
	PTCP_HEADER recv;
	recv=(PTCP_HEADER) malloc(sizeof(TCP_HEADER));

	terTCP.tcp_flag=FIN_FLAG;

	sendto(sock, &terTCP, sizeof(TCP_HEADER), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	recvfrom(sock, recv,sizeof(TCP_HEADER), 0, (struct sockaddr*)&from_serv, &addr_size);
	if(recv->tcp_flag==ACK_FLAG)
	{
		recvfrom(sock, recv,sizeof(TCP_HEADER), 0, (struct sockaddr*)&from_serv, &addr_size);
		if(recv->tcp_flag==FIN_FLAG)
		{
			terTCP.tcp_flag=ACK_FLAG;
			sendto(sock, &terTCP, sizeof(TCP_HEADER), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
			printf("terminated!!!!!!");
			return 1;
		}
	}

	return 0; //error!!
}

void* thread_rcv(void* arg)
{
	printf("rcv thread start!!!\n");
	PTCP_HEADER recvData;
	TCP_HEADER m_sHeader;
	
	while(1)
	{
		
		recvData=(PTCP_HEADER) malloc(sizeof(TCP_HEADER));
		recvfrom(sock, recvData,sizeof(TCP_HEADER), 0, (struct sockaddr*)&from_serv, &addr_size);
		//printf("recv %d\n",recvData->tcp_seq);
		/////////////

		memcpy(&wnd[recvData->tcp_seq].TCPseg,recvData,sizeof(TCP_HEADER));

		//printf("%d %d\n",wnd[recvData->tcp_seq].TCPseg.tcp_seq,wnd[recvData->tcp_seq].TCPseg.tcp_len);
		//////////////
		m_sHeader.tcp_flag=ACK_FLAG;
		m_sHeader.tcp_seq=recvData->tcp_seq;

		sendto(sock, &m_sHeader, sizeof(TCP_HEADER), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		printf("sendto ACK %d\r",recvData->tcp_seq);

		free(recvData);
		if(recvData->tcp_seq==segn-1)
			break;
	}
	printf("\n");
	/////////////file flush//////////////
	FILE* f;
	f=fopen(FILE_NAME,"w");
	int i=0;
	printf("file write!!\n");

	int w=filesize;
	while(i<segn)
	{
		//printf("[%d %d %d ]\r",i,wnd[i].TCPseg.tcp_len,segn);
		//fprintf(f,"%s",wnd[i].TCPseg.tcp_data);
		fwrite(wnd[i].TCPseg.tcp_data,sizeof(unsigned char),wnd[i].TCPseg.tcp_len,f);
		i++;
	}
	fclose(f);
	printf("\n");
	printf("file write!! end\n");

	////////////////////////////////////

	/////////////////termination//////////////
	if(clientConnectonTermination()==1)
	{
		printf("connection Terminated!!!!!!!!\n");
		//break;
	}
	else
		printf("termination errer!!!\n");
	/////////////////////////////////////////////
}


int main(int argc, char** argv)
{
	 
	
	///////////////////////
	void* ee;	 
	pthread_t tsend;
	pthread_t trcv;

	///////////////
	


	//printf("%s \n", INADDR_ANY);
	 //puts(INADDR_ANY);
	 char message[BUF];
	 
	
	 int str_len;
	 int i;
	
	//char* _IP_ADDR="127.0.0.1";
	 char ipa[50];
	 printf("input server ip address >>>> ");
	 scanf("%s",ipa);
	initUDPSock(&sock, &serv_addr, inet_addr(ipa),_PORT); //소켓 이니셜라이즈
	
	TCP_HEADER m_sHeader;
	PTCP_HEADER recvTCP;
	
	 //m_sHeader.tcp_sport=3333;
	// m_sHeader.tcp_dport=2222;
	 m_sHeader.tcp_seq=0;
	 m_sHeader.tcp_flag=SYN_FLAG;
	 strcpy(m_sHeader.tcp_data,"hello");
//	while(1)
	 {
		str_len=sendto(sock, &m_sHeader, sizeof(TCP_HEADER), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		printf("sendto SYN_FLAG\n");
		addr_size = sizeof(from_serv);
		recvTCP=(PTCP_HEADER) malloc(sizeof(TCP_HEADER));
	
		recvfrom(sock, recvTCP,sizeof(TCP_HEADER), 0, (struct sockaddr*)&from_serv, &addr_size);
		if(recvTCP->tcp_flag==	0x03) 
		{	
			printf("recvfromecvTCP->tcp_flag %d\n", recvTCP->tcp_flag);
			printf("filesize %s\n",recvTCP->tcp_data);//file size
		 	sscanf(recvTCP->tcp_data,"%d\t%s",&filesize,FILE_NAME);
			m_sHeader.tcp_flag=ACK_FLAG;

			sendto(sock,&m_sHeader, sizeof(TCP_HEADER), 0,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
			printf("send ACK\n");
			printf("server: TCP connection Success!!!!!!!!\n");
			{
						////////////////
				segn=(filesize/TCP_MAX_DATA)+1;
				wnd=(PWINDOW)malloc(sizeof(WINDOW)*segn);
				for(i=0;i<segn;i++)
				{
					wnd[i].winSeq=i;
				}

				pthread_create(&trcv,NULL,thread_rcv,0);			
				pthread_join(trcv,&ee);
						////////////////
			}
		 }
	 }
	 close(sock);
	 printf("%d",sizeof(TCP_HEADER));
	 return 0;
}
