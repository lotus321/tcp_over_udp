//computer network[02] 2013 hw04
// 200802201 wonhee joeng
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>



#define BUF 1024


#define TRUE 1
#define FALSE 0
#define BOOL int
//#define _MODE int
#define SERVER_MODE 999
#define CLIENT_MODE 888

#define TCP_HEADER_SIZE                 28
#define TCP_MAX_DATA                    1300
#define SYN_FLAG 0x01
#define ACK_FLAG 0x02
#define SYN_ACK_FLAG 0x03
#define FIN_FLAG 0x04
#define DATA_FLAG 0x08


typedef struct _TCP_HEADER {
                unsigned short  tcp_sport;      /* source port 2*/
                unsigned short  tcp_dport;      /* destination port 2*/
                unsigned int    tcp_seq;        /* sequence number 4*/  
                unsigned int    tcp_ack;        /* acknowledge sequence4 */
                unsigned char   tcp_offset;     /* NO USE 1*/
                unsigned char   tcp_flag;       /* control flag 1*/
                unsigned short  tcp_window;     /* NO USE 4*/
                unsigned short  tcp_cksum;      /* check sum 4*/
                unsigned short tcp_len; //4
                unsigned short  tcp_urgptr;     /* NO USE4 */
                unsigned char   Padding[4]; //4
                unsigned char tcp_data[TCP_MAX_DATA];   /* 1300data part */
        } TCP_HEADER, *PTCP_HEADER;

//typedef struct sgiaction sigac;
typedef struct _WINDOW{
				TCP_HEADER TCPseg;
				int winSeq;
				int ackOK;
				
				//struct sgiaction act;

}WINDOW, *PWINDOW;


int getFileSize(FILE *f)
{
	int filesize=0;
	fseek(f,0L,SEEK_END);

	filesize=ftell(f);
	fseek(f,0L,SEEK_SET);
	return filesize;

}


BOOL initUDPSock(int* psock,struct sockaddr_in* pserv_addr,unsigned int ipaddr, int port);//소켓 초기화



BOOL initUDPSock(int* psock,struct sockaddr_in* pserv_addr,unsigned int ipaddr, int port)//소켓 초기화
{
	 *psock = socket(PF_INET, SOCK_DGRAM, 0);

	 if(*psock<0){
		 printf("ㅁㄴㅇㄻㄴㅇㄹㅇㄴㅁㄹ");
		 return FALSE;
	 }

	// printf("UDP socket desc : %d\n", sock);
	 memset(pserv_addr, 0, sizeof(*pserv_addr));
	 pserv_addr->sin_family = AF_INET;
	 pserv_addr->sin_addr.s_addr = ipaddr;//inet_addr(ipaddr);
	 pserv_addr->sin_port = htons(port);

	 return TRUE;
}



