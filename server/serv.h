#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <mysql/mysql.h>

#define PORT 7889
#define CMDLEN 11

#define CLNT_MAX 3                     // 총 소화할 수 있는 클라 수
#define BUFFSIZE 131                    // 클라에게 받는 패킷을 담을 버퍼 크기
#define WAIT_NUM 5                      // accept 받기까지의 대기자 수
#define IDSIZE 21                       // 사용자 id 최대허용길이
#define INFOSIZE 256			//db에 id pw 으로 저장됨
// 커맨드
#define QUIT "-q"
#define KICK "-k"
#define PRNT "-p"

extern int clnt_socketList[CLNT_MAX];   // 클라이언트와의 1:1 소켓 식별자들 저장
extern int clnt_cnt;                    // 임시로 받을 클라소켓 식별자
extern pthread_mutex_t mtx;
extern bool server_down;
extern pthread_t clnt_threadList[CLNT_MAX];

typedef struct 
{
    int temp_cnt;
    int temp_List[CLNT_MAX];
}DATA;


void* clnt_handler(void *arg);
void* input_cmd(void* Args);
void* accept_connections(void * arg);
void cleanup_handler(void *arg);
void send_all_clnt(char *buffer, int from_clnt_socket, bool is_alarm);
void extractID(char clnt_id[],char buffer[]);

//#define DEBUG // 디버그용
#ifdef DEBUG // 디버그용 함수원형

void login_ID(int clnt_socket, char packet[]);
void login_PW(int clnt_socket, char packet[]);
void unique_ID(int clnt_socket, char packet[]);
void save_Info(int clnt_socket, char packet[]);

#else
void finish_with_error(MYSQL *con);
void login_ID(int clnt_socket, char packet[]);
void login_PW(int clnt_socket, char packet[]);
void unique_ID(int clnt_socket, char packet[]);
void save_Info(int clnt_socket, char packet[]);

// 정인영이 채우기

#endif
