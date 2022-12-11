# bingo-game



Server, Client 빙고 게임 만들기


1. 빙고 사이즈 : 5x5
2. default Server IP : 220.149.128.100
3. default PORT : 4018
4. 빙고 보드판을 시작과 동시에 하나를 생성한다.
5. default Server iP, PORT 는  argv로 변경가능

    a. argc 1개 : error --> error문구 출력 및 입력 어떻게 해야하는지 출력 후 exit

    b. argc 2개 : argv[2] 입력 값을 이름으로 저장

    c. argc 3개 : argv[2] --> PORT, argv[3] --> name 으로 저장

    d. argc 4개 : argv[2] --> Server IP, argv[3] --> PORT, argv[4] --> name으로 저장

6. 소켓을 생성하고 서버에 연결한다.
7. 서버가 열려있지 않을 시 connect err를 출력하며 종료됨
8. 서버에 2명이 접속했을 때 둘다 ready가 되면 게임 start
9. 서버에 3명이 접속했을 때 3명 다 ready가 되면 게임 start


Client에서 Server로 메세지 보낼 때

1. 이름은 최초로 접속할 때 서버로 1번 보낸다.
2. M 입력 되었을 때 빙고 보드를 새로 만들어 최초로 생성된 보드판 대신 변경한다.<br/>
        빙고 게임중엔 변경되지 못 하도록 하였다.
3. P 입력 되었을 때 현재 나의 보드판을 보여준다. - 검은 색으로 출력
4. C 입력 되었을 때 채팅
        채팅에 보낼 메시지를 입력받고 서버로 보낸다.
5. Q 입력 되었을 때 종료
6. 내 차례일 때 N이 입력되면 빙고판에서 지우고 싶은 숫자를 입력하면 된다.<br/>
        숫자가 입력되면 서버로 입력된 숫자를 보낸다.<br/>
        숫자가 범위를 벗어났을 경우 다시 입력하라는 문구를 출력하고 다시 입력을 받는다.<br/>
7. R 입력 되었을 때 레디내역을 서버에 보낸다.

Server에서 Client로 메세지 받았을 때

1. C로 시작하는 채팅내역이 오면 buffer에 있는 채팅 내용을 msgQ에 저장하고 출력한다.
2. T로 오는 제어문이 오면 내 차례가 되었다고 알려주는 부분이다.<br/>
        my_turn flag를 1로 변경한다.
3. W로 시작하는 제어문이 오면 <br/>
    0 --> 진행, 1 --> 패배, 2 --> 무승부, 3 --> 승리
    값에 따라 게임을 계속 진행, 승리, 패배, 무승부에 대한 결과를 알려준다.
4. N로 시작하는 제어문이 오면 두자리 문자열로 온 숫자를 변환한다.<br/>
    각각 하나씩 변환한 후 10에 자리에 있는 숫자를 x10을 한다.<br/>
    board에서 해당 숫자의 위치를 찾아 그 부분은 입력 되어 있다고 체크한다.<br/>
    턴의 숫자를 증가한다.<br/>
    빙고가 3개 되었는지 체크한다.<br/>
    3개가 되었다면 W 로 승리 1을 서버로 보낸다.<br/>
    3개가 아니라면 W 로 0을 서버로 보내 다음을 진행하도록 한다.<br/>
5. P로 시작하는 제어문이 온다면 현재 빙고판을 검은 색으로 출력한다.  <br/>
        --> Client에서 Server로 보내는 부분에서 해보았으나 그러면 system(clear)로 인해 안 보인다.<br/>따라서 Server에서 Client로 P라는 제어문을 보내 출력 될 수 있도록 하였다.  
6. 모든 제어 제어문을 완료하고 난 후 game_Print(0) 로 게임을 화면에 출력한다.

Client에서 사용하는 함수
1. game_Print()
    --> game에 진행상황 프린트한다.
        gameon = 1 일때  <br/>
        빙고판 내용 출력 이미 입력 된 숫자는 빨간색으로 아닌 것은 주황색으로 출력한다.  <br/>
        내 차례 일땐 its My turn을 출력한다. <br/> 
        Win_flag가 1,2,3,-1일 때 각각 승리, 무승부, 패배, 에러를 출력한다.  <br/>
        gameon = 0 일때<br/>
        채팅 부분과 제어를 위해 사용할 부분을 사용자에게 알려주기 위하여 설명내용 출력한다.<br/>
2. bingo_print()<br/>
    --> 현재 빙고판을 프린트한다.
3. Bingo_Check()<br/>
    --> 빙고의 개수가 몇개인지 체크한다.<br/>
        가로, 세로, 대각선 빙고가 있을 시 count++로 빙고 개수를 체크하고 이 값을 return 한다.
4. Make_Bingo()<br/>
    --> 새로운 빙고판을 만든다.<br/>
        실행하게 되면 board가 변경된다.

Server에서
1. 포트 4018 연다. argv 2개면 입력된 포트로 연다.
2. 서버에서 가지고 있는 정보
    --> 클리아언트 현황 clnt ip, port, name, ready, bingo
    R로 클라이언트의 상황을 파악한다. 
    0 == WAIT, 1 == READY, 2 == INGAME, 3 == TURN
3. 채팅현황
    msgQ[]에 채팅 내역을 저장하고 있다.
4. 게임현황
    win_check, clnt_cnt*clnt_cnt    누가 이기고 있는지 체크


![Screenshot from 2022-12-11 22-03-16](https://user-images.githubusercontent.com/90883561/206905315-a31fac99-3731-400f-9731-7fb281a2115c.png)

![Screenshot from 2022-12-11 22-18-56](https://user-images.githubusercontent.com/90883561/206907668-1425d03f-9222-451a-b3eb-40f29140e4ae.png)

![Screenshot from 2022-12-11 22-19-24](https://user-images.githubusercontent.com/90883561/206907671-c97f0b10-6013-4c27-bcf5-89ae31c200c0.png)

![Screenshot from 2022-12-11 22-19-42](https://user-images.githubusercontent.com/90883561/206907681-58dce90d-f5fc-45e4-afc8-944416d4d9fb.png)

![Screenshot from 2022-12-11 22-38-38](https://user-images.githubusercontent.com/90883561/206907686-07b0d71b-c616-4642-911a-45e0fb5e1940.png)

![Screenshot from 2022-12-11 22-39-04](https://user-images.githubusercontent.com/90883561/206907699-b1083a24-16ba-42a1-a367-e5ff90352604.png)

![Screenshot from 2022-12-11 22-39-57](https://user-images.githubusercontent.com/90883561/206907715-616d2bed-041f-4899-8f19-3b4ed881233f.png)

![Screenshot from 2022-12-11 22-42-00](https://user-images.githubusercontent.com/90883561/206907724-d5e22c99-c338-4942-92d5-5ea10433327d.png)

![Screenshot from 2022-12-11 22-42-26](https://user-images.githubusercontent.com/90883561/206907736-5d67065f-df12-4c1a-8624-3d56d4a659b5.png)

![Screenshot from 2022-12-11 22-48-21](https://user-images.githubusercontent.com/90883561/206907741-cbee94d3-a902-4c92-a977-d814369a9f31.png)








