#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>

#define IDSIZE 21		// id 최대 문자열 길이 20
#define DBPKSIZE 46		// id + pw + 마진 -> 회원가입용

#define MSGSIZE 101		// 발신 입력용
#define PACKSIZE 131		// 발신용
#define BUFFSIZE 131		// 수신용

#define PORT 7889		// 서버로의 통로

// 클라이언트 단 커맨드
#define QUIT "-q"
#define TERMINATE "-t"

extern bool session_down;
extern bool user_down;
extern pthread_mutex_t mtx;

typedef struct // 스레드 동작함수에 넘겨줄 구조체 (스레드 동작함수는 인자를 하나밖에 못받음)
{
	int serv_socket;
	int* errCode;
	char userID[IDSIZE];
}Args;


bool is_correctIP(char IP[], int length);
bool login_process(int serv_socket, int* errCode, char* userID);
bool signup_process(int serv_socket, int* errCode, char* userID);
bool follow_rules(char input[]);
void* recv_msg(void* parameter); // 소켓식별자, 에러코드, id
void* send_msg(void* parameter); // 소켓식별자, 에러코드, id
void clean_recv();
void clean_send(); // thread_cancel 을 위한 종료출력함수
