//computer network[02] 2013 hw04
// 200802201 wonhee joeng
//


#include "tcp.h"
#include <pthread.h>

#define _PORT 8888

#define SLIDE_WINDOW_SIZE 10
///////window/////////
PWINDOW wnd;
int win_N,win_BASE;
int win_cur;
//////////////////

//////////socket//////////////
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int udp_sock;
TCP_HEADER m_sHeader;
int client_addr_size;
////////////////////

///////////timer//////////
struct sigaction act;
int state;
////////////////

//////////Mute//////////
pthread_mutex_t mutex;
//////////////
int segn;
char FILE_NAME[100];

int slidewindowmoveable()
{//how much move slide window????????

	int i;

	for(i=win_BASE; i<win_BASE+win_N-1;i++)
	{
		if(wnd[i].ackOK==0)
		{
			
			return i-win_BASE-1;
		}
	}
	//printf("moveable!!!!\n");
	return win_N-1;

}

void timer(int sig)
{
	TCP_HEADER sendTCP;
	printf("timeout!!\n");
	int i;
	for(i=win_BASE; i<win_BASE+win_N-1;i++)
	{
		if(wnd[i].ackOK==0) //retransmission
		{
			memcpy(&sendTCP,&wnd[i].TCPseg,sizeof(TCP_HEADER));
			printf("resend   sendTCP sq %d\n",sendTCP.tcp_seq);
			sendto(udp_sock, &sendTCP, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, client_addr_size);
		}
	}
	alarm(1);
}


void windowInit()
{ //window initialize
	wnd=(PWINDOW)malloc(sizeof(WINDOW)*segn);
	win_N=SLIDE_WINDOW_SIZE;
	win_BASE=0;
	win_cur=0;
}

void* thread_send(void *arg)
{
	printf("send thread start!!!\n");
	char buf[TCP_MAX_DATA];
	int i=0;
	FILE *f;
	int readlen;

	if((f=fopen(FILE_NAME, "r"))==NULL)
	{
		printf("file error!!!!");
		exit(1);
	}
	

	segn=(getFileSize(f)/TCP_MAX_DATA)+1;

	windowInit(); //window initialize


	
	//int t=0;

	///init timer/////

		act.sa_handler=timer;
		sigemptyset(&act.sa_mask);
		act.sa_flags=0;
		state=sigaction(SIGALRM,&act,0);
		if(state!=0)
		{

			puts("sigaction() error!!");
			exit(1);
		}



	////////////


	while(!feof( f))
	{
		memset(&wnd[i], 0, sizeof(WINDOW));
		readlen=fread((void*)buf,sizeof(char),TCP_MAX_DATA,f);

		wnd[i].winSeq=i;
		wnd[i].ackOK=0;
		wnd[i].TCPseg.tcp_seq=i;
		wnd[i].TCPseg.tcp_flag=DATA_FLAG;


		wnd[i].TCPseg.tcp_len=readlen;
		memcpy(wnd[i].TCPseg.tcp_data,buf, readlen);
		printf("%d %d \r",wnd[i].TCPseg.tcp_seq ,readlen);
	//	fwrite(wnd[i].TCPseg.tcp_data,1,wnd[i].TCPseg.tcp_len,g);
		i++;


	}
	//fclose(g);
	TCP_HEADER sendTCP;
	
	alarm(1); //timer set!!

	while(1)
	{
		//printf("--------------------------------------------\n");

	
		
		if(win_BASE+win_N>win_cur)
		{
			
			//printf("base: %d  N: %d  cur: %d \n",win_BASE,win_N,win_cur);
			
			//printf("if(win_BASE+win_N>win_cur)%d \n",wnd[win_cur].TCPseg.tcp_seq);
			memcpy(&sendTCP,&wnd[win_cur].TCPseg,sizeof(TCP_HEADER));
		
			
			if(sendTCP.tcp_seq==5||sendTCP.tcp_seq==10||sendTCP.tcp_seq==2)
			{
				//printf("pass~~~~ %d\n",sendTCP.tcp_seq);
			}
				else
			
			{
				printf("sendTCP sq %d\r",sendTCP.tcp_seq);
				sendto(udp_sock, &sendTCP, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, client_addr_size);
			}
		//	pthread_mutex_lock(&mutex);
			win_cur++;
		
		}
		


		if(win_BASE+win_N==win_cur)
		{
			
			int c;// moveable window size

			if((c=slidewindowmoveable())>0)
			{		
				pthread_mutex_lock(&mutex);
				win_BASE+=c; //move slide window
				//printf("move slide window %d\n",c);
				pthread_mutex_unlock(&mutex);
				alarm(1);//timer set!!
			}
			//printf("wait\n");
		}
	
		if(win_cur==segn)
			break; //file tranmit

		
	}
}
int connectionTermination()
{

	TCP_HEADER terHeader;
	 PTCP_HEADER ppp;
	 ppp=(PTCP_HEADER)malloc(sizeof(TCP_HEADER));

	terHeader.tcp_flag=ACK_FLAG;
	sendto(udp_sock, &terHeader, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, client_addr_size);
	terHeader.tcp_flag=FIN_FLAG;
	sendto(udp_sock, &terHeader, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, client_addr_size);

	recvfrom(udp_sock, ppp,sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, &client_addr_size);

	if(ppp->tcp_flag==ACK_FLAG)
	{
		return 1;

	}
	return 0;
}

void* thread_rcv(void* arg)
{
	printf("rcv thread start!!!\n");

	//ack receive!!!
	 PTCP_HEADER ppp;
	while(1)
	{
		

		ppp=(PTCP_HEADER)malloc(sizeof(TCP_HEADER));
		recvfrom(udp_sock, ppp,sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, &client_addr_size);
		//printf("ppp->tcp_flag     %d\n",ppp->tcp_flag);
		if(ppp->tcp_flag==ACK_FLAG)
		{
			//printf("\n\n-------ack %d rcv!!\r",ppp->tcp_seq);
			pthread_mutex_lock(&mutex);
			wnd[ppp->tcp_seq].ackOK=1;
			pthread_mutex_unlock(&mutex);
		}
		 if(ppp->tcp_flag== FIN_FLAG)
		{ //termination
			
			if(1==connectionTermination())
			{
				printf("connectionTermination!!!!!\n");
				break;
			}


		}
		
		free(ppp);

		
	}

}

int main(int argc, char** argv)
{
	//	printf("error");
	 
	FILE *f;
	 char message[BUF];
	 
	 int str_len;
	
	////thread ///////
	void* ee;	 
	pthread_t tsend;
	pthread_t trcv;

	///////////////
	pthread_mutex_init(&mutex, NULL);
	////////////////

	///

	PTCP_HEADER ppp;
	TCP_HEADER sendTCP;
	////

	printf("input file name to send>>>");
	scanf("%s",FILE_NAME);
	if((f=fopen(FILE_NAME, "r"))==NULL) //file open
			{
				printf("file not found\n");
				exit(1);
			}else
				fclose(f);

	
	//initialize socket
	if(initUDPSock(&udp_sock, &server_addr, htonl(INADDR_ANY),_PORT) ==FALSE)
		printf("error");
	//bind socket!
	 bind(udp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	 

	printf("wait for client!!!................\n");
	
	/////////// 3way HAND shaking!!!!!!!//////////////
	 {
		  client_addr_size = sizeof(client_addr);
		  ppp=(PTCP_HEADER) malloc(sizeof(TCP_HEADER));

		
		str_len=recvfrom(udp_sock, ppp, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, &client_addr_size);
		 
		if(ppp->tcp_flag==0x01) 
		{
			printf("recvfrom( SYN FLAG!!!\n");
			sendTCP.tcp_flag=SYN_ACK_FLAG;
		 
		 	////////////FILE SIZE ////////////////
			
			//strcpy(FILE_NAME,"test.jpg");
			

			if((f=fopen(FILE_NAME, "r"))==NULL) //file open
			{
				printf("file not found");
				exit(1);
			}
			char fsize_buf[TCP_MAX_DATA];
			int t=getFileSize(f); //<<filesize
			fclose(f);
			/////////////////////////////////
			
			printf("file size::: %d\n",t);
			sprintf(fsize_buf,"%d\t%s",t,FILE_NAME);
			
			memcpy(sendTCP.tcp_data,fsize_buf,TCP_MAX_DATA);

			/////////////////////////////

		 	sendto(udp_sock, &sendTCP, sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, client_addr_size);
		  	printf(" sendto SYN_ACK_FLAG!!!\n");
			free(ppp);
			ppp=(PTCP_HEADER)malloc(sizeof(TCP_HEADER));
			recvfrom(udp_sock, ppp,sizeof(TCP_HEADER), 0, (struct sockaddr*)&client_addr, &client_addr_size);
			if(ppp->tcp_flag==ACK_FLAG)
			{

				printf("ACK_FLAG\n");
				printf("server: TCP connection Success!!!!!!!!\n");
				////////////////thread start!!!!!!!!!
				pthread_create(&tsend,NULL,thread_send,0);
				pthread_create(&trcv,NULL,thread_rcv,0);			
				pthread_join(tsend,&ee);
				pthread_join(trcv,&ee);
				
				//wnd=(PWIDDOW)malloc(sizeof(WINDOW)*100);

		
				
			///////////////
			
			
			}


}
	}
	 close(udp_sock);

	 pthread_mutex_destroy(&mutex);
	 return 0;
}
