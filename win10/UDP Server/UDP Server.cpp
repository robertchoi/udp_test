// UDP Server.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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

//������ ȭ�� �νĽ� �۽��� Packet
//������ ȭ���� �ν� �ɶ� ���� �Ʒ� Packet�� ����
typedef struct _packet
{
	int		_msgIndex;		//�޽��� ID��(�ּ� �۽Ž� 0���� 1�� ����, �̹��� �νĽÿ��� ����)
	int		_cntRepeat;		//�޽����� 2�� ���۵Ǹ�, �����۵Ǵ� �޽����� ������ 
	long	_timeMsgSent;	//�ν� �޽����� �����Ű�� �ð�(���� ������ �ð��� Unix Time ���� ����)
	long	_timeDetected;	//������ �̹����� �νĵ� �ð�(���� ������ �ð��� Unix Time ���� ����)
	int		_typeError;		//������ ȭ���� ����(1: , 2: ,  3: , 4: , 5: , 999: ���ǵ��� ���� ����)
	char	_strDesc[NOTI_DESC_SIZE];	//��Ÿ �ΰ� ���� ����� ����
} NotiPacket;


/* ������ ȭ�� �νĽ� ������ ���� �����ϵ��� ���� �ʿ�

  1. �۽��� ������ ��Ŷ ����
    1) ������ ȭ�� ���� ī��Ʈ ����
	2) ������ ȭ�� ������ ����
	3) �ν� �ð� �� ���� �ð��� ����
	4) ��Ÿ �ΰ� ������ ����
  2. ���� Connection (port : 7878)
  3. ������ ����
    1) ���� �� 2ȸ �ݺ� ����. ���� ���۽� _cntRepeat ���� 0���� �����Ͽ� ����
	2) �ι�° ���۽� _cntRepeat���� 1�� �����Ͽ� ����
  4. Socket Close
  5. ������ ȭ���� �νĵ� ������ 1~4�� ���� �ݺ�

  �� Server�� ������ ���� �ʴ� ���, ������ ��Ŷ�� ��ܾ� �� ������ ���� �α� ���Ͽ� ����
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