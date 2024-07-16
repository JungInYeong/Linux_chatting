// serv_shell.c
#include "serv.h"

int clnt_socketList[CLNT_MAX];
pthread_t clnt_threadList[CLNT_MAX];
char clnt_idList[CLNT_MAX][IDSIZE];
int clnt_cnt = 0;
pthread_mutex_t mtx;
bool server_down = false;

int main()
{
    /*----------------------------------*/
    // 1. 변수선언 + 초기화
    /*----------------------------------*/

    int serv_socket; // 서버 자체의 소켓 + 임시로 받을 클라의 소켓
    struct sockaddr_in serv_addr; // 서버 자체의 주소정보 + 임시로 받을 클라의 주소정보
    pthread_t serv_thread ,cmd_thread ,accept_thread;

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

    pthread_mutex_init(&mtx,NULL);

    //system("clear");

    //printf("if you type '-q', server will be closed safely!\n\n");

    DATA copiedDATA = {0,{0}};
    // 서버를 고의적으로 끌 때, 반복문 전체를 뮤텍스로 못덮음

    /*----------------------------------*/
    // 5. 커맨드를 위한 스레드 만들기
    /*----------------------------------*/

    pthread_create(&cmd_thread,NULL, input_cmd,(void *) &copiedDATA); // 커맨드 입력을 위한 스레드 생성
    pthread_create(&accept_thread, NULL, accept_connections, (void *) &serv_socket);

    /*----------------------------------*/
    // 6. 서버 종료시까지 대기
    /*----------------------------------*/

    while(!server_down) usleep(500); // 0.5초마다 서버가 다운되는지 살핌

    for (int idx = 0; idx < clnt_cnt; idx++)
    {
        pthread_cancel(clnt_threadList[idx]);
        pthread_join(clnt_threadList[idx], NULL);
        close(clnt_socketList[idx]);
        printf("thread for client[%d] is terminated!\n", clnt_socketList[idx]);
    }

    // 커맨드 입력 스레드 종료
    pthread_cancel(cmd_thread);
    pthread_cancel(accept_thread);

    pthread_join(cmd_thread, NULL);
    pthread_join(accept_thread, NULL);

    close(serv_socket);
    pthread_mutex_destroy(&mtx);

    printf("resource cleanup completed! good bye!\n");
    return 0;
}

void* accept_connections(void * arg)
{
    int serv_socket = *(int *)arg;
    int clnt_socket;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);

    while(1)
    {
	sleep(2);
        pthread_mutex_lock(&mtx);

        if(clnt_cnt < CLNT_MAX)
        {
            pthread_mutex_unlock(&mtx);
            clnt_socket = accept(serv_socket, (struct sockaddr *) &clnt_addr, (socklen_t *) &clnt_addr_size);

            if(clnt_socket == -1)
            {
                printf("Accept error at client[%d]\n",clnt_socket);
                continue;
            }
            else
	    {
		printf("server and client[%d] are connected now!\n", clnt_socket);

            	// 수신을 위한 스레드 만들기
            	pthread_mutex_lock(&mtx);
            	clnt_socketList[clnt_cnt] = clnt_socket;
            	pthread_create(&clnt_threadList[clnt_cnt++], NULL, clnt_handler, (void*) &clnt_socket);

		if(clnt_cnt == CLNT_MAX) printf("server now has the maximum number of people\n");
            	pthread_mutex_unlock(&mtx);

            	printf("Thread was made for the client[%d] right now\n", clnt_socket);
		
		int AcceptedMSG = 1;
		if(write(clnt_socket,&AcceptedMSG,sizeof(int)))
		       printf("Accepted message has been sent to this client!\n");	
		// 대기열에서 벗어났고, 정상적으로 동작한다고 알림
	    }
        }
        else if(clnt_cnt > CLNT_MAX)
        {
            pthread_mutex_unlock(&mtx);
            printf("Additional Access denied!\n");
        }
	else
	    pthread_mutex_unlock(&mtx);
    }

    pthread_exit(0);
}
