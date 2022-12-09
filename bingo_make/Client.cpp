#include <stdio.h>      // printf(), ...
#include <stdlib.h>     // exit(), ...
#include <time.h>       // 현재 시간, 날짜 구하기
#include <string.h>     // strerror(), strcmp(), ...
#include <unistd.h>     // close(), ...
#include <arpa/inet.h>  // htons(), htonl(), ...
#include <sys/socket.h> // socket(), AF_INET, ...
#include <sys/select.h> // 다중 입출력
#include <netinet/in.h> // connect(), sendmsg(), sendto(), ...
#include <pthread.h>    // pthread_create(), ...

#define BOARD_SIZE 5        // 빙고 보드 사이즈
#define NAME_SIZE 10        // 사용자 이름 사이즈
#define BUFSIZE 100        // 버퍼 사이즈

void* send_msg(void* arg);
void* recv_msg(void* arg);
void* game_set(void* arg);

void error_handling(char* mse);

void game_Print(int anything);      // 게임 print
void Make_Bingo();                  // 빙고판 만들기
int Bingo_Check(int board[][BOARD_SIZE]);   // 빙고 개수 체크
void bingo_print(int any); 			// 빙고판만 print

// 서버 IP 학교 IP주소 입력 // 220.149.128.100 or 220.149.128.103
char *SERVERIP = (char *)"220.149.128.100";
int SERVERPORT = 4018; // 기본 포트 번호

//게임관련 구조체로 묶을 변수
struct Game{
    int Game_on;
    int game_turn;
    int my_turn;
    int my_bingo; 
    int Win_flag; //Wflag: 0진행 1패배 2무승부 3승리
    int board[BOARD_SIZE][BOARD_SIZE];
    int bingo[BOARD_SIZE][BOARD_SIZE];

};
struct Game B_MyGame ={0,};

//채팅관련 구조체로 묶을변수

char msgQ[5][NAME_SIZE+BUFSIZE];

//main의 매개변수용 char
char name[NAME_SIZE]="[DEFAULT]";

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread, gam_thread;
	void* thread_return;

	srand(time(NULL));

	// 처음 세팅으로 빙고판 만들기
	Make_Bingo(); // 맨처음 빙고판 만들기

	// 인수가 1개 들어왔을 때 error
	if(argc == 1) {
		// error 문구 출력
		printf("name 이나\n");
		printf("port, name이나\n");
		printf("ip, port, name을 입력하세요\n");
		exit(1);	
	}
	// 인수가 2개 일때 name 저장
	else if (argc == 2) {
		sprintf(name, "[%s]", argv[1]);
	}
	// 인수가 3개 일때 PORT, name 저장
	else if(argc == 3) {
		sprintf(name, "[%s]", argv[2]);
		SERVERPORT = atoi(argv[1]);
	}
	// 인수가 4개 들어왔을 때 SERVERIP, SERVERPORT, name 저장
	else if (argc == 4) {
		printf("ip, port, name");
		sprintf(name,"[%s]",argv[3]); // 4번째 인수를 이름으로 저장
		sprintf(SERVERIP, "%s", argv[1]); // SERVERIP 주소 저장
		SERVERPORT = atoi(argv[2]); // SERVERPORT 저장
	}

	// 소켓 생성
	sock = socket(PF_INET, SOCK_STREAM, 0);

	// connect()
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVERIP);
	serv_addr.sin_port = htons(SERVERPORT);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect err");
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_create(&gam_thread, NULL, game_set, (void*)&sock);
	//pthread_detach(snd_thread);
	//pthread_detach(rcv_thread);
	//pthread_detach(gam_thread);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	pthread_join(gam_thread, &thread_return);
	if(1){
        close(sock);
        return 0;
    }
}

// 서버에 msg보내는 함수
void* send_msg(void* arg) {
	int sock = *((int*)arg);
	char set[111];
	sprintf(set,"%1s%10s","S",name);//이름을 최초 1회 보내는걸로 검증한다.
	write(sock, set, strlen(set));
	while (1) 
	{	
		char msg[BUFSIZE];
		char chat[NAME_SIZE+BUFSIZE+2];
		//배열버퍼, stdin버퍼 초기화

		fgets(msg, BUFSIZE, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))//Q를 입력하면 종료로 인식한다.
		{
			close(sock);
			exit(0);
		}
		if(!strcmp(msg, "c\n")||!strcmp(msg,"C\n")) //C를 입력하면 채팅입력창을 출력한다.
		{
			printf("type msg: ");
			/*
			for(int i=0; i<BUFSIZE;i++)
			{
				msg[i]='\0';
			}
			for(int i=0; i<NAME_SIZE+BUFSIZE+1;i++)
			{
				chat[i]='\0';
			}
			*/
			fgets(msg, BUFSIZE, stdin);
			msg[strlen(msg)-1]='\0';
			
			//입력받은 msg로 chat내용을 세그먼트화 한다. (채팅-10자리이름(공백으로 줄맞춤)-메세지내용)
			sprintf(chat,"%1s%10s%s","C",name,msg);
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
			
			
		}
		if(!strcmp(msg, "p\n")||!strcmp(msg, "P\n")) // P가 입력되었을 때 빙고판 확인하게 해준다.
		{
			write(sock, "P", 2);
			printf("bingo print\n");
		}
		if(!strcmp(msg, "m\n")||!strcmp(msg, "M\n")) // M이 입력되었을 때 빙고판을 새로 만든다.
		{
			if(B_MyGame.Game_on == 0) // 게임이 시작되기 전에 변경가능
				Make_Bingo();
			write(sock, "M", 2);
		}
		if(B_MyGame.my_turn==1&&!strcmp(msg,"N\n")) //내턴일때 N을 입력하면 숫자를 입력받는다.
		{
			while(1) {
				printf("NUM:");
				fgets(msg, BUFSIZE, stdin);

				if(atoi(msg)==0) {
					continue;
				} else {
					msg[strlen(msg)-1]=10;//개행문자
					sprintf(chat,"%1s%10s%2s","N",name,msg);
					write(sock, chat, strlen(chat));
					B_MyGame.my_turn--;
				}
				break;
			}
			printf("[Debug]writed\n");
		}
		else if(!strcmp(msg, "r\n")||!strcmp(msg,"R\n")) //R를 입력하면 레디내역을 서버에보낸다.
		{
				for(int i=0; i<BUFSIZE;i++)
				{
				msg[i]='\0';
				}
			sprintf(chat,"%1s%10s","R",name);
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
		}
		
	}
	return NULL;
}

// 받은 msg 처리하는 함수
void* recv_msg(void* arg) {
	int sock = *((int*)arg);
	char chat[BUFSIZE];
	char FLAG[1+NAME_SIZE+BUFSIZE];
	ssize_t str_len;
	char msg[BUFSIZE];
	while (1)
	{
		if(str_len=read(sock, msg, 1+BUFSIZE+NAME_SIZE)!=0){
			char tmpName[10]; //
			for(int i=0,j=0;i<10;i++){
				if(msg[i+1]!=32) {tmpName[j++]=msg[i+1];}
			}
			char tmpMsg[100]; //
			for(int i=0;i<111;i++){
			tmpMsg[i]=msg[i+11];
			}
			system("clear");

			if(strcmp(msg,"GAMEON")==0) B_MyGame.Game_on=1;
			if(msg[0]==87)//W로 시작하는 제어문이 오면 Wflag: 0진행 1패배 2무승부 3승리
			{
				if(strcmp(tmpName, name)==0){
					printf("\n승리플래그 메세지 검증%d\n",tmpMsg[0]);
					switch (tmpMsg[0]) {
						case 48 : B_MyGame.Win_flag=0; break;
						case 49 : B_MyGame.Win_flag=1; break;
						case 50 : B_MyGame.Win_flag=2; break;
						case 51 : B_MyGame.Win_flag=3; break;						
						default : B_MyGame.Win_flag=-1; break;						
					}
				}
			}
			if(msg[0]==67) //C로 시작하는 채팅내역이오면
			{	
				strcpy(msgQ[4],msgQ[3]);
				strcpy(msgQ[3],msgQ[2]);
				strcpy(msgQ[2],msgQ[1]);
				strcpy(msgQ[1],msgQ[0]);
				sprintf(msgQ[0],"%s%s",tmpName,tmpMsg);
			}
			if(msg[0]==84)//T로 시작하는 제어문이 오면
			{
				if(strcmp(tmpName, name)==0) B_MyGame.my_turn++;
			}
			if(msg[0]==78)//N로 시작하는 제어문이 오면
			{
					//2자리문자열로 온 숫자를 아스키코드표에 따라 숫자로 변환
					//printf("숫자[%d][%d]",tmpMsg[0],tmpMsg[1]);
				int NUM=0;
				printf("서버입력받아 변환할 값 %d %d",tmpMsg[0],tmpMsg[1]);
				if(tmpMsg[1]==10){NUM=tmpMsg[0]-48;}			
				else{NUM=(10*(tmpMsg[0]-48))+tmpMsg[1]-48;}
				//printf("받아서 변환된숫자: %d\n",NUM);
				for(int i=0; i<BOARD_SIZE;i++){
					for(int j=0; j<BOARD_SIZE;j++){
						if(B_MyGame.board[i][j]==NUM){
							B_MyGame.bingo[i][j]=1;
							B_MyGame.game_turn++;
							//printf("smp checker");
						}
					}
				  }
				for(int i=0; i<BUFSIZE;i++)
				{
				FLAG[i]='\0';
				}
				B_MyGame.my_bingo = Bingo_Check(B_MyGame.bingo);
				//리시브가 모두 끝나고 난 뒤에, 승리플래그를 보낼지 검증해야한다.
				if(B_MyGame.my_bingo==3)
				{
					sprintf(FLAG,"%1s%10s%s","W",name,"1");
					int k= write(sock, FLAG, strlen(FLAG));
					if(k!=-1) {printf("[Debug][bingo3]writed\n");}
				}
				else{
					sprintf(FLAG,"%1s%10s%s","W",name,"0");
					write(sock, FLAG, strlen(FLAG));
					printf("[Debug]writed\n");
				}
			}

			if(msg[0]==80) // P로 시작하는 제어문이 오면
			{
				bingo_print(0);
			}

			for(int i=0; i<BUFSIZE;i++){
				msg[i]='\0';
			}

			//UI표시부
			game_Print(0);
			
						
		}
	}
	return NULL;
}

void* game_set(void* arg){
	int sock = *((int*)arg);
}

// error 관리
void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

// game print 게임 내용 출력
void game_Print(int any)
{
	if(B_MyGame.Game_on==1){
	int i, j, x;
	printf("%c[1;33m", 27); 

	printf("@----- client bingo -----@\n");
	printf("turn: %3d bingo: %3d\n", B_MyGame.game_turn, B_MyGame.my_bingo);
	printf("*-----*-----*-----*-----*-----*\n");
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			/*
			if (B_MyGame.board[i][j] == 0)
			{
				printf("| ");
				printf("%c[1;31m", 27);
				printf("%2c ", 88);
				printf("%c[1;33m", 27);
			}
			else
				printf("| %2d ", B_MyGame.board[i][j]);
			*/

			// 나왔었던 번호는 red로 출력, 나머진 orange로 출력
			if(B_MyGame.bingo[i][j]==1){
				printf("|\033[1;31m %2d \033[1;33m", B_MyGame.board[i][j]);
			}
			else
				printf("| %2d ", B_MyGame.board[i][j]);
		}
		printf("|\n");
		printf("*-----*-----*-----*-----*-----*\n");
	}
	
	printf("%c[0m", 27);
	/*
	if (turn_count != 0)
	{
		printf("number: %d\n", 1);
		printf("bingo count: %d\n", 1);
	}*/
	}
	else{
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}
	printf("=====================================\n");
	if(B_MyGame.Win_flag==3){printf("YOU WIN!!\n");} // Win_flag 3일때 승리
	else if(B_MyGame.Win_flag==2){printf("DRAW!!\n");} // Win_flag 2일때 무승부
	else if(B_MyGame.Win_flag==1){printf("LOSE\n");} // Win_flag 1일때 패배
	else if(B_MyGame.Win_flag==-1){printf("ERR\n");} // Win_flag -1일때 에러
	else if(B_MyGame.my_turn==1){printf("its your turn\n");} // 내차례
	else {printf("\n");}
	printf("=====================================\n");
	printf("5:%s \n4:%s \n3:%s \n2:%s \n1:%s \n",msgQ[4],msgQ[3],msgQ[2],msgQ[1],msgQ[0]); // 채팅 출력
	printf("=====================================\n");
	printf("M to makeBingo, P to check_Board, C to chat, R to Ready, N to Number, Q to quit\n");
}

void bingo_print(int any)
{
	int i, j;
	printf("%c[1;30m", 27);  // 검은 색으로 출력

	printf("@----- My bingo board -----@\n");
	printf("*-----*-----*-----*-----*-----*\n");
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			printf("| %2d ", B_MyGame.board[i][j]);
		}
		printf("|\n");
		printf("*-----*-----*-----*-----*-----*\n");
	}
	
	printf("%c[0m", 27);
}

// 빙고 개수가 몇개인지 체크하는 함수
//빙고 체크 함수, 매개변수로 넘어온 배열(빙고판)의
//체크된 것을 모두 확인해서 빙고수를 반환한다.
int Bingo_Check(int board[][BOARD_SIZE])
{
	int i;
	int count=0;

	for(i=0; i < BOARD_SIZE; i++)
	{
        // 가로
		if(board[i][0]==1&&board[i][1]==1&&board[i][2]==1&&board[i][3]==1&&board[i][4]==1) //가로
		{
			count++;
		}
        // 세로
		if(board[0][i]==1&&board[1][i]==1&&board[2][i]==1&&board[3][i]==1&&board[4][i]==1) //세로
			count++;
	}
    // '\'로 빙고일 때
	if(board[0][0]==1&&board[1][1]==1&&board[2][2]==1&&board[3][3]==1&&board[4][4]==1)
		count++;
    // '/'로 빙고일 때
	if(board[0][4]==1&&board[1][3]==1&&board[2][2]==1&&board[3][1]==1&&board[4][0]==1)
		count++;
	return count;
}

// 보드판 생성 함수,
// 원래 초기에 1번 정해진 보드판으로 게임을 진행하게 되는데
// 원하는 보드판이 나올 때까지 보드판을 변경할 수 있도록한다.
void Make_Bingo()
{
    int check_number[BOARD_SIZE*BOARD_SIZE] = { 0 };
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            while(1)
			{
				int temp = rand() % 25;

				if (check_number[temp] == 0)
				{
					check_number[temp] = 1;
					B_MyGame.board[i][j] = temp + 1;
					break;
				}
			}        
        }
    }
}