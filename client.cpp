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
	int		_msgIndex;		//메시지 ID값(최소 송신시 0에서 1씩 증가, 이미지 인식시에만 증가)
	int		_cntRepeat;		//메시지는 2번 전송되며, 재전송되는 메시지의 순서를 
	int		_typeError;		//비정상 화면의 유형(1: , 2: ,  3: , 4: , 5: , 999: 정의되지 않은 오류)
	long	_timeMsgSent;	//인식 메시지를 송출시키는 시간(현재 지역의 시간을 Unix Time 포맷 저장)
	long	_timeDetected;	//비정상 이미지가 인식된 시간(현재 지역의 시간을 Unix Time 포맷 저장)
	//char	_strDesc[NOTI_DESC_SIZE];	//기타 부가 설명 저장용 변수
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

int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode, int cntRepeat);


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


//    while(1)
    {
        //서버 주소 구조
        memset(&servaddr, 0, addrlen); //bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET; //인터넷 Addr Family
        servaddr.sin_addr.s_addr = inet_addr(argv[1]); //argv[1]에서 주소를 가져옴
        servaddr.sin_port = htons(atoi(argv[2])); //argv[2]에서 port를 가져옴

        time(&tLocal);
        timeinfo = gmtime(&tLocal);
        tLocal = mktime(timeinfo);


        makeNotiPacket(&npDetected, nIdx++, (long)tLocal, 1, 0);
        printf("%d, %d\n", (int)sizeof (npDetected._cntRepeat),(int)sizeof (npDetected._timeDetected));

        printf("%d, %d, %d, %d\n", npDetected._msgIndex, (int)sizeof (npDetected),(int)npDetected._timeDetected, (int)npDetected._timeMsgSent);

        if((sendto(s, &npDetected, sizeof (npDetected), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
            perror("sendto fail");
            exit(0);
        }

        delay(1000000);

        makeNotiPacket(&npDetected, nIdx++, (long)tLocal, 1, 1);
        if((sendto(s, &npDetected, sizeof (npDetected), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
            perror("sendto fail");
            exit(0);
        }

        delay(3000000);

        time(&tLocal);
        timeinfo = gmtime(&tLocal);
        tLocal = mktime(timeinfo);


        makeNotiPacket(&npDetected, nIdx++, (long)tLocal, 2, 0);
        printf("%d, %d\n", (int)sizeof (npDetected._cntRepeat),(int)sizeof (npDetected._timeDetected));

        printf("%d, %d, %d, %d\n", npDetected._msgIndex, (int)sizeof (npDetected),(int)npDetected._timeDetected, (int)npDetected._timeMsgSent);

        if((sendto(s, &npDetected, sizeof (npDetected), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
            perror("sendto fail");
            exit(0);
        }

        delay(1000000);

        makeNotiPacket(&npDetected, nIdx++, (long)tLocal, 2, 1);
        if((sendto(s, &npDetected, sizeof (npDetected), 0, (struct sockaddr *)&servaddr, addrlen)) < 0) {
            perror("sendto fail");
            exit(0);
        }



//        if(nIdx > 5)
//            break;

    }



    close(s); //socket close
    return 0;
}

int makeNotiPacket(NotiPacket *npSend, int index, long detectTime, int errorCode, int cntRepeat)
{
    time_t tLocal;
    time(&tLocal);

    npSend->_msgIndex = index;
    npSend->_typeError = errorCode;
    npSend->_cntRepeat = cntRepeat;
    npSend->_timeMsgSent = (long)tLocal;
    npSend->_timeDetected = detectTime;

    return 0;

}

