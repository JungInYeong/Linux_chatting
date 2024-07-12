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

#define IDSIZE 21               // id �ִ� ���ڿ� ���� 20
#define DBPKSIZE 46             // id + pw + ���� -> ȸ�����Կ�

#define MSGSIZE 101             // �߽� �Է¿�
#define PACKSIZE 131            // �߽ſ�
#define BUFFSIZE 131            // ���ſ�

#define PORT 7889               // �������� ���

// Ŭ���̾�Ʈ �� Ŀ�ǵ�
#define QUIT "-q"
#define TERMINATE "-t"

extern bool session_down;
extern bool user_down;
extern pthread_mutex_t mtx;

typedef struct // ������ �����Լ��� �Ѱ��� ����ü (������ �����Լ��� ���ڸ� �ϳ��ۿ� ������)
{
        int serv_socket;
        int* errCode;
        char userID[IDSIZE];
}Args;


bool is_correctIP(char IP[], int length);
bool login_process(int serv_socket, int* errCode, char* userID);
bool signup_process(int serv_socket, int* errCode, char* userID);
bool follow_rules(char input[]);
void* recv_msg(void* parameter); // ���Ͻĺ���, �����ڵ�, id
void* send_msg(void* parameter); // ���Ͻĺ���, �����ڵ�, id
void clean_recv();              // thread_cancle �� ���� ��������Լ�