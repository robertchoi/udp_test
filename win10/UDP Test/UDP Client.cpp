// UDP Test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

//-----------------------------------------------------------------------------
#define NOTI_DESC_SIZE 256

//비정상 화면 인식시 송신할 Packet
//비정상 화면이 인식 될때 마다 아래 Packet을 전송
typedef struct _packet
{
	int		_msgIndex;		//메시지 ID값(최소 송신시 0에서 1씩 증가, 이미지 인식시에만 증가)
	int		_cntRepeat;		//메시지는 2번 전송되며, 재전송되는 메시지의 순서를 
	long	_timeMsgSent;	//인식 메시지를 송출시키는 시간(현재 지역의 시간을 Unix Time 포맷 저장)
	long	_timeDetected;	//비정상 이미지가 인식된 시간(현재 지역의 시간을 Unix Time 포맷 저장)
	int		_typeError;		//비정상 화면의 유형(1: , 2: ,  3: , 4: , 5: , 999: 정의되지 않은 오류)
	char	_strDesc[NOTI_DESC_SIZE];	//기타 부가 설명 저장용 변수
} NotiPacket;


/* 비정상 화면 인식시 다음과 같이 동작하도록 구현 필요

  1. 송신한 데이터 패킷 구성
    1) 비정상 화면 검출 카운트 설정
	2) 비정상 화면 유형값 설정
	3) 인식 시간 및 현재 시간값 설정
	4) 기타 부가 정보값 설정
  2. 서버 Connection (port : 7878)
  3. 데이터 전송
    1) 송출 시 2회 반복 전송. 최초 전송시 _cntRepeat 값을 0으로 설정하여 전송
	2) 두번째 전송시 _cntRepeat값을 1로 설정하여 전송
  4. Socket Close
  5. 비정상 화면이 인식될 때마다 1~4번 과정 반복

  ※ Server에 연결이 되지 않는 경우, 데이터 패킷에 담겨야 할 내용을 내부 로그 파일에 저장
*/


int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode, char *strDesc);
int makeResendNotiPacket(NotiPacket *npSend);
void printNotiPacket(NotiPacket *npSend);
//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	char message[BUF_SIZE];
	int strLen;
	NotiPacket npDetected, npRecv;
	SOCKADDR_IN servAdr;
	time_t tLocal;
	struct tm* timeinfo;
	
	int nIdx = 0;
    
	if(argc!=3) 
    {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
    
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	
	npDetected._msgIndex = 0;
	npDetected._cntRepeat = 0;
	npDetected._timeDetected = 0;
	npDetected._timeMsgSent = 0;
	npDetected._typeError = 0;


	while(1)
	{
		sock=socket(PF_INET, SOCK_DGRAM, 0);   
		if(sock==INVALID_SOCKET)
			ErrorHandling("socket() error");
	
		memset(&servAdr, 0, sizeof(servAdr));
		servAdr.sin_family=AF_INET;
		servAdr.sin_addr.s_addr=inet_addr(argv[1]);
		servAdr.sin_port=htons(atoi(argv[2]));

		time(&tLocal);
		timeinfo = gmtime(&tLocal);
		tLocal = mktime(timeinfo);


		fputs("Insert message(q to quit): ", stdout);
		fgets(message, sizeof(message), stdin);     

		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n"))	
			break;

		/*
		send(sock, message, strlen(message), 0);
		strLen=recv(sock, message, sizeof(message)-1, 0);
		message[strLen]=0;
		printf("Message from server: %s", message);
		*/
		connect(sock, (SOCKADDR*)&servAdr, sizeof(servAdr));
		memset(&npRecv, 0, sizeof(NotiPacket));
		makeNotiPacket(&npDetected, nIdx++, (long)tLocal, (nIdx%5 + 1), message);
		send(sock, (char*)&npDetected, sizeof(NotiPacket), 0);
		strLen = recv(sock, (char*)&npRecv, sizeof(NotiPacket)-1, 0);
		
		printNotiPacket(&npRecv);
		closesocket(sock);
	}	

	WSACleanup();

	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

//-----------------------------------------------------------------------------
int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode, char *strDesc)
{
	time_t tLocal;
	time(&tLocal);
	struct tm* timeinfo;

	timeinfo = gmtime(&tLocal);
	tLocal = mktime(timeinfo);

	npSend->_msgIndex = index;
	npSend->_typeError = errorCode;
	npSend->_timeMsgSent = (long)tLocal;
	npSend->_timeDetected = detectTime;
	memset(npSend->_strDesc, 0, NOTI_DESC_SIZE);
	strncpy(npSend->_strDesc, strDesc, NOTI_DESC_SIZE);

	//For Debugging 
	printNotiPacket(npSend);

	return 0;
};

int makeResendNotiPacket(NotiPacket *npSend)
{
	npSend->_cntRepeat = npSend->_cntRepeat + 1;
	
	printf("Repeat Count : %d\n", npSend->_cntRepeat);

	return 0;
}

void printNotiPacket(NotiPacket *npSend)
{
	printf("index : %d, sendtime : %li, detecttime : %li, error type : %d\n", \
		npSend->_msgIndex, npSend->_timeMsgSent, npSend->_timeDetected, npSend->_typeError);
	printf("description : %s\n", npSend->_strDesc);
}
//-----------------------------------------------------------------------------