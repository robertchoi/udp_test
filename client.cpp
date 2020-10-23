//--------------------------------------------------------------
// client : udp_echocli.c
// command : cc -o udp_echocli  udp_echocli.c
// localip : udp_echocli  127.0.0.1 8999
// Client 소스코드 파일을 열어서 서버에 전송 및 저장하고, 다시 읽어 들여
// 유효성 체크를 하는 코드
//--------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAXLINE  511 //최대값 지정
#define BLOCK 255 //BLOCK 단위로 저장

struct sockaddr_in servaddr;
int addrlen = sizeof(servaddr); //서버 주소의 size를 저장

#define NOTI_DESC_SIZE 30

typedef struct _packet
{
    int _msgIndex;
    int _cntRepeat;
    long _timeMsgSent;
    long _timeDetected;
    int _typeError;
    //char _strDesc[NOTI_DESC_SIZE];
} NotiPacket;


//메시지 전송 부분 처리
void sendMessage(int s, char* buf) {
    if((sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
        perror("sendto fail");
        exit(0);
    }
}

void delay(clock_t n)
{
    clock_t start = clock();
    while(clock() - start < n);
}

int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode);


int main(int argc, char *argv[]) {
    int s; //socket
    int nbyte;
    char buf[MAXLINE+1], buf2[MAXLINE+1];

    NotiPacket npDetected, npRecv;
    time_t tLocal;
    struct tm* timeinfo;

    int nIdx = 0;

    //./udp_echocli.c ip주소, 포트번호, 입출력 파일명
    if(argc != 3) {
        printf("usage: %s ip_address port_number\n", argv[0]);
        exit(0);
    }

    //socket 연결 0보다 작으면 Error
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }
    
    npDetected._msgIndex = 0;
    npDetected._cntRepeat = 0;
    npDetected._timeDetected = 0;
    npDetected._timeMsgSent = 0;
    npDetected._typeError = 0;


    while(1)
    {
        //서버 주소 구조
        memset(&servaddr, 0, addrlen); //bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET; //인터넷 Addr Family
        servaddr.sin_addr.s_addr = inet_addr(argv[1]); //argv[1]에서 주소를 가져옴
        servaddr.sin_port = htons(atoi(argv[2])); //argv[2]에서 port를 가져옴

        time(&tLocal);
        timeinfo = gmtime(&tLocal);
        tLocal = mktime(timeinfo);


        makeNotiPacket(&npDetected, nIdx++, (long)tLocal, 1);

        printf("%d, %d, %d, %d, %d\n", npDetected._typeError, npDetected._msgIndex, (int)sizeof (npDetected),npDetected._timeDetected, npDetected._timeMsgSent);

        if((sendto(s, &npDetected, sizeof (npDetected), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
            perror("sendto fail");
            exit(0);
        }

        delay(1000000);

        if(nIdx > 5)
            break;

    }



    close(s); //socket close
    return 0;
}

int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode)
{
    time_t tLocal;
    time(&tLocal);

    npSend->_msgIndex = index;
    npSend->_typeError = errorCode;
    npSend->_timeMsgSent = (long)tLocal;
    npSend->_timeDetected = detectTime;

    return 0;

}

