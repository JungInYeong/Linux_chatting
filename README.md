# 리눅스 소켓 프로그래밍
&nbsp;
&nbsp;

## ☁️ 프로젝트 소개
wsl2안에서 Ubuntu를 사용해, C99를 이용해서 CLI기반 간단한 채팅프로그램을 구현해본 프로젝트입니다
&nbsp;
&nbsp;

## 🤔 주요기능
```sh
[client shell]
* 로그인기능, 회원가입기능
* 커맨드기능 : 메인메뉴 복귀, 프로세스 종료
```
```sh
[server shell]
* 계정사용 기능
* 컴파일 옵션을 통한 DEBUG모드와 RELEASE모드 분리
* 커맨드기능 : 프로세스 종료, 강퇴, 현접자 확인

[모드정보]
RELEASE모드 : MYSQL DB를 연동하여, 계정사용기능을 컨트롤 (사전에 MySQL을 리눅스에 깔아야 할것)
DEBUG모드 : .txt 형태로 DB를 만들어, 계정사용기능을 컨트롤
```

## ℹ️ 개발정보

* 개발기간 : 24.07.02 ~ 07.17
* 개발인원 : 류성수(dbtjdtn325@gmail.com), 정인영
* 개발환경 : wsl2 Ubuntu (22.04.4) , C99, MySQL (8.0.32)

## ⚡설치 및 실행방법

레포지토리를 clone 하거나 zip Download를 통해 파일을 받습니다.
```sh
https://github.com/supergravityy/Linux_chatting.git
```

### 💻클라이언트

#### 1. 저장된 디렉터리 안으로 들어가 gcc로 파일들을 다음의 명령어로 컴파일 해줍니다.
   ```sh
   gcc -lpthread clnt_socket.c clnt_shell2.c clnt_commu.c -o client.out
   ```
   (혹은 'make'만 입력해도 상관없습니다)

#### 2. client.out 파일실행시, 서버의 IP주소도 넣어줍니다


   ![스크린샷 2024-07-17 185557](https://github.com/user-attachments/assets/27a279ba-3566-409c-a511-d28741c6d3f7)

(만약, wsl2 안의 리눅스라면, 반드시 포트포워딩작업과 방화벽을 꺼주셔야 합니다 -> 로컬환경에서 작동시 필요 X)

#### 3. 서버와 연결되면, 메인메뉴로 들어가 원하는 작업을 완료하고 다른 사용자들과 채팅을 시작하십시오

   ![스크린샷 2024-07-17 191222](https://github.com/user-attachments/assets/39c6b8fa-81ff-4a0b-aefb-0652953df56e)

### 🗒️서버 (DEBUG 모드 기준)

#### 1. 저장된 디렉터리 안으로 들어가 gcc로 파일들을 다음의 명령어로 컴파일 해줍니다
   ```sh
   gcc -lpthread serv_DB.c serv_commu2.c serv_shell2.c -DDEBUG -o server.out
   ```
   (그냥 'make'만 입력시, release모드로 컴파일 됩니다 -> MySQL 사전 설치작업 필요)

#### 2. server.out 을 실행시켜줍니다.

(만약, wsl2 안의 리눅스라면, 반드시 포트포워딩작업과 방화벽을 꺼주셔야 합니다 -> 로컬환경에서 작동시 필요 X)

![스크린샷 2024-07-18 011937](https://github.com/user-attachments/assets/a7ec1a70-a413-4027-8ebc-e0f4f7e6e8b1)

### 💽서버 (RELEASE 모드 기준)

#### 1. Ubuntu 환경 MySQL 설치 및 설정

##### 1) MySQL 설치
   ```sh
   $ sudo apt-get install mysql-server -y
   ```

##### 2) MySQL 포트 변경, 외부 접속허용 설정
   ```sh
   $ sudo vim /etc/mysql/mysql.conf.d/mysqld.cnf
	Port = 자신이 정한 포트번호
   bind-adress = 0.0.0.0
   // mysql-bind-address = 127.0.0.1 (외부 접속을 위해 주석처리)
   ```

##### 3) MySQL 재시작.
   ```sh
   $ sudo service mysql restart
   ```

##### 4) 기존포트(3306) 닫기, 외부 접속포트 방화벽 해제
   ```sh
   $ sudo ufw deny 3306 && sudo ufw allow PORT(포트번호)
   ```

##### 5) MySQL 실행 후, 유저와 IP 접속 권한 만들기
   ```sh
   $ sudo mysql – u root
      - USE mysql;
      - CREATE USER ‘ID’@’%’ IDENTIFIED BY MYSQL_NATIVE_PASSWORD ‘[PW]’;
      - CREATE DATABASE <DB명>;
	   - GRANT ALL PRIVILEGES ON <DB명>.* to ‘ID’@’%’;
	   - FLUSH PRIVILEGES; (변경사항 적용)
   ```

#### 2. MySQL C API 자료형
```sh
1) MYSQL : Database 연결에 대한 핸들러입니다. 
2) MYSQL_RES : 행(SELECT, SHOW, DESCRIBE)을 query의 결과를 나타냅니다.
3) MYSQL_ROW : 한 행의 데이터에 대한 형식이 안전한 표현, 바이트 문자열의 배열로 구현됨, 행은 mysql_fetch_row(MYSQL_RES *result)를 호출하여 얻습니다.
4) MYSQL_FIELD : 필드의 이름, 타입, 크기에 관한 정보를 저장하는 자료형입니다.
```

#### 3. MySQL Database 연결
```sh
1) mysql_init(MYSQL *mysql)
  - mysql_real_connect()를 위하여 MYSQL 객체 초기화(MYSQL 객체를 초기화 하므로 mysq_real_connect()전에 꼭 호출해야합니다.)

2) mysql_real_connect(MYSQL* mysql, const char* host, const char* user, const char* passwd, const char* db, uint port, const char* unix_socket, uint client_flag)
 - host에 지정된 서버로 연결을 시도하는 함수
 - mysql : MYSQL 변수에 대한 포인터 형
 - host : 연결하고자 하는 서버의 IP Address 혹은 도메인 이름을 적어주면 됩니다. NULL로 적어주면 localhost를 의미합니다.
 - user : 접속시의 사용자 이름입니다.. NULL이면 현재 login한 user ID가 됩니다.
 - passwd : user의 암호를 나타냅니다.. NULL이면 암호가 없다는 의미입니다.
 - db : 접속시에 사용하고자 하는 database를 나타낸다. NULL로 지정을 하면 연결 후에 mysql_select_db() 혹은 mysql_query()를 이용해서 지정할 수 있고, database를 바꿀 수도 있습니다.
 - port : TCP/IP 연결시에 사용할 포트 번호를 나타냅니다.
 - unix_socket : 보통 NULL로 하면됩니다.
 - client_flag : 이 인자도 보통 0으로 해주면 됩니다.

mysql_real_connect()는 성공적으로 연결이 되면, MYSQL 포인터를 넘겨주고 연결에 실패하였을 경우 NULL을 리턴합니다.

3) mysql_close(MYSQL* mysql)
  - 서버와 연결을 끊고 mysql에 할당되었던 메모리를 해제합니다.
```

#### 4. MySQL query 및 결과값 얻어오기
```sh
1) mysql_query(MYSQL* mysql, const char* query)
  - query 실행 시킴(mysql 클라이언트에서 했던 것 처럼 query의 끝에 ‘;’가 포함되어서는 안 됨)

 2) mysql_store_result(MYSQL* mysql)
  -  query의 결과로 리턴되는 ROW들을 한꺼번에 얻어옴

 3) mysql_fetch_row(MYSQL_ROW* result)
 - result에 있는 ROW들에서 한 개의 ROW를 얻어 옴
```

#### 5. MySQL C API 개발환경 구축
```sh
 1) 라이브러리(mysql.h) 설치
  - apt-get install libmysqlclient-dev

 2) include 방법
 - #include <mysql/mysql.h>

 3) 컴파일 방법
 - gcc –o 파일이름 파일이름.c –lmysqlclient
```

#### 6. Ubuntu환경에서 DB확인 방법
##### 1) root권환 접속을 합니다.
  ![스크린샷 2024-07-28 152328](https://github.com/user-attachments/assets/de45f704-c646-42ea-86bb-bb3a61b247e1)

##### 2) DB 종류를 검색합니다.
  ![스크린샷 2024-07-20 233128](https://github.com/user-attachments/assets/cab0e824-67e4-49e3-a39c-df0ae9a30d90)

##### 3) 사용할 DB를 선택합니다.
  ![스크린샷 2024-07-20 233235](https://github.com/user-attachments/assets/756e5385-a2d2-4714-ac67-e894da8d92b9)

##### 4) Table을 확인합니다.
  ![스크린샷 2024-07-20 233320](https://github.com/user-attachments/assets/53895671-1428-4078-9550-a61b65c6712b)

##### 5) Data를 조회합니다.
  ![스크린샷 2024-07-20 233419](https://github.com/user-attachments/assets/af1ef9cc-eb54-4bba-bdba-28132f15a443)
