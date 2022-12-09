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


Client에서 Server로 메세지 보낼 때

1. 이름은 최초로 접속할 때 서버로 1번 보낸다.
2. M 입력 되었을 때 빙고 보드를 새로 만들어 최초로 생성된 보드판 대신 변경한다.
        빙고 게임중엔 변경되지 못 하도록 하였다.
3. P 입력 되었을 때 현재 나의 보드판을 보여준다. - 검은 색으로 출력
4. C 입력 되었을 때 채팅
        채팅에 보낼 메시지를 입력받고 서버로 보낸다.
5. Q 입력 되었을 때 종료
6. 내 차례일 때 N이 입력되면 빙고판에서 지우고 싶은 숫자를 입력하면 된다.
7. R 입력 되었을 때 레디내역을 서버에 보낸다.
8. 