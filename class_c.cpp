*** 여기서부터 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/

#include <sys/types.h> // basic type definitions
#include <sys/socket.h> // socket(), AF_INET, ...
#include <arpa/inet.h> // htons(), htonl(), ...
#include <netdb.h> // gethostbyname(), ...
#include <unistd.h> // close(), ...
#include <fcntl.h> // fcntl(), ...
#include <pthread.h> // pthread_create(), ...
#include <poll.h> // poll()
#include <sys/epoll.h> // epoll()

#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strerror(), ...
#include <errno.h> // errno

// Windows 소켓 코드와 호환성을 위한 정의
typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
        char *msgbuf = strerror(errno);
        printf("[%s] %s\n", msg, msgbuf);
        exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
        char *msgbuf = strerror(errno);
        printf("[%s] %s\n", msg, msgbuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
    char *msgbuf = strerror(errcode);
    printf("[오류] %s\n", msgbuf);
}

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char *argv[])
{
        int retval;

        // 명령행 인수가 있으면 IP 주소로 사용
        if (argc > 1) SERVERIP = argv[1];

        // 소켓 생성
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
          if (sock == INVALID_SOCKET) err_quit("socket()");

        // connect()
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
        serveraddr.sin_port = htons(SERVERPORT);
        retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) err_quit("connect()");

        // 데이터 통신에 사용할 변수
        char buf[BUFSIZE + 1];
        int len;

        // 서버와 데이터 통신
        while (1) {
                // 데이터 입력
                printf("\n[보낼 데이터] ");
                if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
                        break;

                // '\n' 문자 제거
                len = (int)strlen(buf);
                len = (int)strlen(buf);
                if (buf[len - 1] == '\n')
                        buf[len - 1] = '\0';
                if (strlen(buf) == 0)
                        break;

                // 데이터 보내기
                retval = send(sock, buf, (int)strlen(buf), 0);
                if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        break;
                }
                printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

                // 데이터 받기
                retval = recv(sock, buf, retval, MSG_WAITALL);
                if (retval == SOCKET_ERROR) {
                        err_display("recv()");
                        break;
                }
                else if (retval == 0)
                        break;

                // 받은 데이터 출력
                buf[retval] = '\0';
                printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
                printf("[받은 데이터] %s\n", buf);
        }

        // 소켓 닫기
        close(sock);
        return 0;
}
