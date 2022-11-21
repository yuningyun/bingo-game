/*** 여기서부터 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/

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

#define SERVERPORT 9000
#define BUFSIZE    512
void *ProcessClient(void *arg)
{
        int retval;
        SOCKET client_sock = (SOCKET)(long long)arg;
        struct sockaddr_in clientaddr;
        char addr[INET_ADDRSTRLEN];
        socklen_t addrlen;
        char buf[BUFSIZE + 1];

        // 클라이언트 정보 얻기
        addrlen = sizeof(clientaddr);
        getpeername(client_sock, (struct sockaddr *)&clientaddr, &addrlen);
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

        while (1) {
                // 데이터 받기
                retval = recv(client_sock, buf, BUFSIZE, 0);
                if (retval == SOCKET_ERROR) {
                        err_display("recv()");
                        break;
                }
                else if (retval == 0)
                        break;

                // 받은 데이터 출력
                buf[retval] = '\0';
                printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

                // 데이터 보내기
                retval = send(client_sock, buf, retval, 0);
                if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        break;
                }
        }

        // 소켓 닫기
        close(client_sock);
        printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
                addr, ntohs(clientaddr.sin_port));
        return 0;
}
int main(int argc, char *argv[])
{
        int retval;

        // 소켓 생성
        SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock == INVALID_SOCKET) err_quit("socket()");

        // bind()
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(SERVERPORT);
        retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) err_quit("bind()");

        // listen()
        retval = listen(listen_sock, SOMAXCONN);
        if (retval == SOCKET_ERROR) err_quit("listen()");

        // 데이터 통신에 사용할 변수
        SOCKET client_sock;
        struct sockaddr_in clientaddr;
        socklen_t addrlen;
        pthread_t tid;

        while (1) {
                // accept()
                addrlen = sizeof(clientaddr);
                client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
                if (client_sock == INVALID_SOCKET) {
                        err_display("accept()");
                        break;
                }

                // 접속한 클라이언트 정보 출력
                char addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
                printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
                        addr, ntohs(clientaddr.sin_port));

                // 스레드 생성
                retval = pthread_create(&tid, NULL, ProcessClient,
                        (void *)(long long)client_sock);
                if (retval != 0) { close(client_sock); }
        }

        // 소켓 닫기
        close(listen_sock);
        return 0;
}
