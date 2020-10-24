// UDP Server.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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


int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen;
	int clntAdrSz;
	NotiPacket npRecv;

	
	SOCKADDR_IN servAdr, clntAdr;
	if(argc!=2) 
    {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
    if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	servSock=socket(PF_INET, SOCK_DGRAM, 0);
	
    if(servSock==INVALID_SOCKET)
		ErrorHandling("UDP socket creation error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(atoi(argv[1]));
	
	if(bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	sizeof(NotiPacket);
	while(1) 
	{
		clntAdrSz=sizeof(clntAdr);
		memset(message, 0, BUF_SIZE);

		//strLen=recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
		//printf("%s(len : %d)\n", message, strLen);
		//sendto(servSock, message, strLen, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));

		strLen = recvfrom(servSock, (char*)&npRecv, sizeof(NotiPacket), 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
		printf("Recv Data Length : %d\n", strLen);
		printNotiPacket(&npRecv);
		sendto(servSock, (char*)&npRecv, sizeof(NotiPacket), 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
	}	
	
    closesocket(servSock);
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