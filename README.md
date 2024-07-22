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
   &nbsp;
&nbsp;

   ![스크린샷 2024-07-17 185557](https://github.com/user-attachments/assets/27a279ba-3566-409c-a511-d28741c6d3f7)

(만약, wsl2 안의 리눅스라면, 반드시 포트포워딩작업과 방화벽을 꺼주셔야 합니다 -> 로컬환경에서 작동시 필요 X)

#### 3. 서버와 연결되면, 메인메뉴로 들어가 원하는 작업을 완료하고 다른 사용자들과 채팅을 시작하십시오
   &nbsp;
  &nbsp;
   ![스크린샷 2024-07-17 191222](https://github.com/user-attachments/assets/39c6b8fa-81ff-4a0b-aefb-0652953df56e)

### 🖥️서버 (DEBUG 모드 기준)

#### 1. 저장된 디렉터리 안으로 들어가 gcc로 파일들을 다음의 명령어로 컴파일 해줍니다
   ```sh
   gcc -lpthread serv_DB.c serv_commu2.c serv_shell2.c -DDEBUG -o server.out
   ```
   (그냥 'make'만 입력시, release모드로 컴파일 됩니다 -> MySQL 사전 설치작업 필요)

#### 2. server.out 을 실행시켜줍니다.

(만약, wsl2 안의 리눅스라면, 반드시 포트포워딩작업과 방화벽을 꺼주셔야 합니다 -> 로컬환경에서 작동시 필요 X)
&nbsp;

&nbsp;
![스크린샷 2024-07-18 011937](https://github.com/user-attachments/assets/a7ec1a70-a413-4027-8ebc-e0f4f7e6e8b1)
