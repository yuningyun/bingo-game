#include <sys/types.h> // basic type definitions
#include <sys/socket.h> // socket(), AF_INET, ...
#include <arpa/inet.h> // htons(), htonl(), ...
#include <netdb.h> //gethostbyname(), ...
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
#define SOCKET_ERROR    -1
#define INVALID_SOCKET  -1

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
void err_display(int ercode)
{
    char *msgbuf = strerror(ercode);
    printf("[오류] %s\n", msgbuf);
}

// 서버 IP 학교 IP주소 입력 //220.149.128.100 or 220.149.128.103
char *SERVERIP = (char *)"220.149.128.100";
int SERVERPORT = 4018; // 포트 번호 내자리 18으로 4018, 4118, 4218, 4318, 4418, 4518 사용가능
#define BUFSIZE 100 // 버퍼 사이즈

#define BOARD_SIZE  5
#define NAME_SIZE   10

// Bingo_Game 구조체
struct Game{
    int Game_on;        // 게임 중
    int game_turn;      // 몇 번째 턴인지
    int my_turn;        // 내 차례일 때
    int my_bingo;       // 내 현재 빙고 개수 (만들어진 빙고 개수)
    int W_flag;         // Win : 0진행, 1패배, 2무승부, 3승리
    int board[BOARD_SIZE][BOARD_SIZE];
    int bingo[BOARD_SIZE][BOARD_SIZE];
};
struct Game B_MyGame = {0,};

// 채팅 구조체로 변수
char msgQ[5][NAME_SIZE+BUFSIZE];

// 사용자 이름 저장 name, main 매개변수용 char 
char name[NAME_SIZE] = "[DEFAULT]";

// 함수선언
void Game_Print(int anything);      // 게임 print
void Make_Bingo();                  // 빙고판 만들기
int Bingo_Check(int board[][BOARD_SIZE]);   // 빙고 개수 체크

void* send_msg(void* arg);  // 서버에 메세지 보내기
void* recb_msg(void* arg);  // 메세지 받아오기
void* game_set(void* arg);  // game start

void error_handling(char* mse);

// main 문
int main(int argc, char* argv[])
{
    // 사용 변수
    pthread_t snd_thread, rcv_thread, game_thread;
    void* thread_return;


    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1)
        error_handling("connect err");
    
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_create(&game_thread, NULL, game_set, (void*)&sock);

    pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	pthread_join(game_thread, &thread_return);
	
    if(1){
        close(sock);
        return 0;
    }
}

// error 출력
void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

// send_msg 서버에 msg 전송
void* send_msg(void* arg) {
    int sock = *((int*)arg);
    char set[111];
    sprintf(set, "%1s%10s", "S", name); // 이름 최초 1회 검증
    write(sock, set, strlen(set));
    while(1)
    {
        char msg[BUFSIZE];
        char chat[NAME_SIZE+BUFSIZE+2];
        // 배열버퍼, stdin버퍼 초기화

        fgets(msg, BUFSIZE, stdin);
        if(!strcmp(msg, "q\n")||!strcmp(msg,"Q\n")) // Q 입력시 종료
        {
            close(sock);
            exit(0);
        }
        if(!strcmp(msg, "c\n")||!strcmp(msg, "C\n")) // C 입력시 채팅창 출력
        {
            printf("type msg: ");

            fgets(msg, BUFSIZE, stdin);
            msg[strlen(msg)-1]='\0';

            // 입력받은 msg로 chat 내용 세그먼크화(채팅 -10자리이름 (공백으로 줄맞춤))
            sprintf(chat, "%1s%10s%s", "C", name, msg);
            write(sock, chat, strlen(chat));
            printf("[Debug]writed\n");


        }
        if(B_MyGame.my_turn == 1 && !strcmp(msg, "N\n")) // Myturn 일때 N을 입력하면 숫자를 입력받는다.
        {
            while(1) {
                printf("NUM: ");
                fgets(msg, BUFSIZE, stdin);

                if(atoi(msg)==0) {
                    continue;
                } else {
                    msg[strlen(msg)-1]=10; // 개행문자
                    sprintf(chat, "%1s%10s%2s", "N", name, msg);
                    write(sock, chat, strlen(chat));
                    B_MyGame.my_turn--;
                }
                break;
            }
            printf("[Debug]writed\n");
        } else if(!strcmp(msg, "r\n")||!strcmp(msg, "R\n")) // R을 입력하면 레디내역을 서버에 보낸다.
        {
            for(int i = 0;i<BUFSIZE; i++)
            {
                msg[i] = '\0';
            }
            sprintf(chat, "%1s%10s", "R", name);
            write(sock, chat, strlen(chat));
            printf("[Debug]writed\n");
        }
        
    }

    return NULL;
}

void* recv_msg(void* arg) {
    int sock = *((int*)arg);
    char chat[BUFSIZE];
    char FLAG[1+NAME_SIZE+BUFSIZE];
    ssize_t str_len;
    char msg[BUFSIZE];
    while(1)
    {
        if(str_len=read(sock, msg, 1+BUFSIZE+NAME_SIZE)!=0) {
            char tmpName[10]; //
            for(int i=0, j=0;i<10;i++) {
                if(msg[i+1]!=32) {tmpName[j++]=msg[i+1];}
            }
            char tmpMsg[100];
            for(int i=0; i<111; i++) {
                tmpMsg[i]=msg[i+11];
            }
            system("clear");

            if(strcmp(msg, "GAMEON")==0) B_MyGame.Game_on=1;
            if(msg[0]==87)// W로 시작하는 제어문이 오면 Wflag: 0진행 1패배 2무승부 3승리
            {
                if(strcmp(tmpName, name)==0){
                    printf("\n승리플래그 메세지 검증%d\n", tmpMsg[0]);
                    switch (tmpMsg[0]) {
                        case 48: B_MyGame.W_flag=0; break;
                        case 49: B_MyGame.W_flag=1; break;
                        case 50: B_MyGame.W_flag=2; break;
                        case 51: B_MyGame.W_flag=3; break;
                        default: B_MyGame.W_flag=-1; break;
                    }
                }
            }
            if(msg[0]==67) // C로 시작하는 채팅내역이 오면
            {
                strcpy(msgQ[4],msgQ[3]);
                strcpy(msgQ[3],msgQ[2]);
                strcpy(msgQ[2], msgQ[1]);
                strcpy(msgQ[1],msgQ[0]);
                sprintf(msgQ[0],"%s%s",tmpName,tmpMsg);
            }
            if(msg[0]==84) //T로 시작하는 제어문이 오면
            {
                if(strcmp(tmpName, name)==0) B_MyGame.my_turn++;
            }
            if(msg[0]==78) // N로 시작하는 제어문이 오면
            {
                // 2자리문장ㄹ로 온 숫자를 아스키코드표에 따라 숫자로 변환
                //printf("숫자[%d][%d]",tmpMsg[0],tmpMsg[1]);
                int NUM=0;
                printf("서버입력받아 변환할 값 %d %d", tmpMsg[0], tmpMsg[1]);
                if(tmpMsg[1]==10){NUM=tmpMsg[0]-48;}
                else{NUM=(10*(tmpMsg[0]-48))+tmpMsg[1]-48;}
                // printf("받아서 변환된숫자: %d\n",NUM);
                for(int i=0; i<BOARD_SIZE;i++){
                    for(int j=0;j<BOARD_SIZE;j++){
                        if(B_MyGame.board[i][j]==NUM){
                            B_MyGame.bingo[i][j]=1;
                            B_MyGame.game_turn++;
                            // printf("smp checker");
                        }
                    }
                }
                for(int i=0;i<BUFSIZE;i++)
                {
                    FLAG[i]='\0';
                }
                B_MyGame.my_bingo=Bingo_Check(B_MyGame.bingo);
                // 리스브가 모두 끝나고 난 뒤에 승리플래그를 보낼지 검증해야한다.
                if(B_MyGame.my_bingo==3)
                {
                    sprintf(FLAG, "%1s%10s%s", "W", name,"1");
                    int k=write(sock, FLAG, strlen(FLAG));
                    if(k!=-1) {printf("[Debug][bingo3]wtired\n");}
                }
                else{
                    sprintf(FLAG,"%1s%10s%s", "W", name,"0");
                    write(sock, FLAG, strlen(FLAG));
                    printf("[Debug]writed\n");
                }
            }
            // UI 표시부
            Game_Print(0);
        }
    }
    return NULL;
}

void* game_set(void* arg){
	int sock = *((int*)arg);
}

// 게임 화면 print
void Game_Print(int anything)
{
    if(B_MyGame.Game_on==1){
        int i, j, x;
        printf("%c[1;33m", 27);

        printf("|----- client bingo -----|\n");
        printf("turn : %3d bingo : %3d\n", B_MyGame.game_turn, B_MyGame.my_bingo);
        printf("--------------------------\n");
        for (i = 0; i < BOARD_SIZE; i++)
        {
            for (j = 0; j < BOARD_SIZE; j++)
            {
                if(B_MyGame.bingo[i][j]==1){
                    printf("|\033[1;31m %2d \033[1;33m", B_MyGame.board[i][j]);
                }
                else {
                    printf("| %2d ", B_MyGame.board[i][j]);
                }
                printf("|\n");
                printf("--------------------------\n");
            }
        }

        printf("%c[0m", 27);
        /*
        if (turn_count != 0)
        {
            printf("number: %d\n", 1);
            printf("bingo count: %d\n", 1);
        }*/
    } else {
        printf("\n\n\n\n\n\n\n\n");
    } 
    printf("===================================\n");
    if(B_MyGame.W_flag == 3) {printf("WIN!!!!!\n");}
    else if(B_MyGame.W_flag == 2) {printf("DRAW!\n");}
    else if(B_MyGame.W_flag == 1) {printf("LOSE.....\n");}
    else if(B_MyGame.W_flag == -1) {printf("ERR  TT\n");}
    else if(B_MyGame.my_turn == 1) {printf("my turn!\n");}
    else {printf("\n");}
    printf("===================================\n");
    printf("5:%s \n 4: %s \n3: %s \n2: %s \n1: %s \n", msgQ[4], msgQ[3], msgQ[2], msgQ[1], msgQ[0]);
    printf("===================================\n");
    printf("C to chat, R to Ready, N to Number Q to quit\n");
}

//빙고 체크 함수, 매개변수로 넘어온 배열(빙고판)의
//체크된 것을 모두 확인해서 빙고수를 반환한다.
int Bingo_Check(int board[][BOARD_SIZE])
{
    int i;
    int count = 0;

    for(i=0; i < BOARD_SIZE; i++) 
    {
        // 가로가 빙고 일때
        if(board[i][0] == 1 && board[i][1] == 1 && board[i][2] == 1 && board[i][3] == 1 && board[i][4] == 1)
            count++;
        // 세로가 빙고 일때
        if(board[0][i] == 1 && board[1][i] == 1 && board[2][i] == 1 && board[3][i] == 1 && board[4][i] == 1)
            count++;
    }
    // 역슬레시 모양으로 빙고 일때
    if(board[0][0] == 1 && board[1][1] == 1 && board[2][2] == 1 && board[3][3] == 1 && board[4][4] == 1)
        count++;
    // 슬레시 모양으로 빙고 일때
    if(board[0][4] == 1 && board[1][3] == 1 && board[2][2] == 1 && board[3][1] == 1 && board[4][0] == 1)
        count++;

    return count;
}

// 보드판 생성 함수
void Make_Bingo()
{
    int temp;
    int check_number[BOARD_SIZE*BOARD_SIZE] = { 0 };
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            while(1)
            temp = rand() % 25;

            if (check_number[temp] == 0)
            {
                check_number[temp] = 1;
                B_MyGame.board[i][j] = temp + 1;
                break;
            }
        }
    }
}
