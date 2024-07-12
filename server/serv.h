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

#define PORT 7889
#define CMDLEN 11

#define CLNT_MAX 3                     // �� ��ȭ�� �� �ִ� Ŭ�� ��
#define BUFFSIZE 131                    // Ŭ�󿡰� �޴� ��Ŷ�� ���� ���� ũ��
#define WAIT_NUM 5                      // accept �ޱ������ ����� ��
#define IDSIZE 21                       // ����� id �ִ�������

// Ŀ�ǵ�
#define QUIT "-q"
#define KICK "-k"
#define PRNT "-p"

extern int clnt_socketList[CLNT_MAX];   // Ŭ���̾�Ʈ���� 1:1 ���� �ĺ��ڵ� ����
extern int clnt_cnt;                    // �ӽ÷� ���� Ŭ����� �ĺ���
extern pthread_mutex_t mtx;
extern bool server_down;

typedef struct 
{
    int temp_cnt;
    int temp_List[CLNT_MAX];
}DATA;


void* clnt_handler(void *arg);
void* input_cmd(void* Args);
void send_all_clnt(char *buffer, int from_clnt_socket, bool is_alarm);
void extractID(char clnt_id[],char buffer[]);

#define DEBUG // ����׿�
#ifdef DEBUG // ����׿� �Լ�����

void login_ID(int clnt_socket, char packet[]);
void login_PW(int clnt_socket, char packet[]);
void unique_ID(int clnt_socket, char packet[]);
void save_Info(int clnt_socket, char packet[]);

#else

// ���ο��� ä���

#endif