#include "serv.h"

int clnt_socketList[CLNT_MAX] = {0};
int clnt_cnt = 0;
pthread_mutex_t mtx;
bool server_down = false;

int main()
{
    // 일단, 클라이언트와 연결할 수 있게 할것

    /*----------------------------------*/
    // 1. 변수선언 + 초기화
    /*----------------------------------*/

    int serv_socket, clnt_socket; // 서버 자체의 소켓 + 임시로 받을 클라의 소켓
    struct sockaddr_in serv_addr, clnt_addr; // 서버 자체의 주소정보 + 임시로 받을 클라의 주소정보
    int clnt_addr_size; // 클라 주소정보의 크기값
    pthread_t serv_thread;
    pthread_t cmd_thread;

    /*----------------------------------*/
    // 2. 소켓열기 + bind 과정
    /*----------------------------------*/

    serv_socket = socket(PF_INET,SOCK_STREAM,0);
    if(serv_socket == -1)
    {
        printf("server socket creation error!\n");
        return 0;
    }

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port=htons(PORT);

    if(bind(serv_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("bind error!\n");
        return 0;
    }

    /*----------------------------------*/
    // 3. 이제부터 listen 상태
    /*----------------------------------*/

    if(listen(serv_socket, WAIT_NUM) == -1)
    {
        printf("listen error!\n");
        return 0;
    }
    else
        printf("server will start listening from now on\n");

    /*----------------------------------*/
    // 4. listen 상태이기에, 클라로부터 connect 대기 (accept)
    /*----------------------------------*/

    clnt_addr_size = sizeof(clnt_addr);
    pthread_mutex_init(&mtx,NULL);

    system("clear");
    
    printf("if you type '-q', server will be closed safely!\n\n");

    DATA copiedDATA = {0,{0}};
    // 서버를 고의적으로 끌 때, 반복문 전체를 뮤텍스로 못덮음

    /*----------------------------------*/
    // 5. 커맨드를 위한 스레드 만들기
    /*----------------------------------*/

    pthread_create(&cmd_thread,NULL, input_cmd,(void *) &copiedDATA); // 커맨드 입력을 위한 스레드 생성

    while(1)
    {
        pthread_mutex_lock(&mtx); // clnt_cnt 는 전역변수이기에 조건문에서 사용하면 무조건 뮤텍스

        if(clnt_cnt < CLNT_MAX) // 최대 10명까지 받을 수 있기에, 10명이 된 순간부터 안받음
        {
            pthread_mutex_unlock(&mtx);
            clnt_socket = accept(serv_socket, (struct sockaddr *) &clnt_addr, (socklen_t *) &clnt_addr_size);

            if(clnt_socket == -1)
            {
                printf("Accept error at client[%d]\n",clnt_socket);
                continue;
            }
            else
                printf("server and client[%d] is connected now!\n", clnt_socket);

            /*----------------------------------*/
            // 6. 수신을 위한 스레드 만들기
            /*----------------------------------*/

            pthread_mutex_lock(&mtx);
            clnt_socketList[clnt_cnt++] = clnt_socket; // 클라의 소켓식별자가 리스트에 저장되고, 접속자 수++
            pthread_mutex_unlock(&mtx);

            pthread_create(&serv_thread, NULL, clnt_handler, (void*) &clnt_socket); // 1:1 챗을 위한 소켓 생성
            printf("Thread was made for the client[%d] right now\n", clnt_socket);
        }
        else
        {
            pthread_mutex_unlock(&mtx);
            printf("we've reached the maximum number of people we can accommodate!\n");
        }

        if(server_down)
            break;
    }

    system("clear");
    printf("waiting for termination of all threads!\n");

    // pthread_cancel()

    for(int idx = 0; idx < temp_cnt; idx++)
    {
        pthread_join(clnt_socketList[idx],NULL);
        printf("thread for client[%d] is terminated!\n",clnt_socketList[idx]);
    }

    close(serv_socket);
    pthread_mutex_destroy(&mtx);

    printf("resource cleanup completed! good bye!\n");
    return 0;
}