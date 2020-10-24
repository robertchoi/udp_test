// UDP Test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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