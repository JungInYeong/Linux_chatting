#include "serv.h"

int clnt_socketList[CLNT_MAX] = {0};
int clnt_cnt = 0;
pthread_mutex_t mtx;
bool server_down = false;

int main()
{
    // �ϴ�, Ŭ���̾�Ʈ�� ������ �� �ְ� �Ұ�

    /*----------------------------------*/
    // 1. �������� + �ʱ�ȭ
    /*----------------------------------*/

    int serv_socket, clnt_socket; // ���� ��ü�� ���� + �ӽ÷� ���� Ŭ���� ����
    struct sockaddr_in serv_addr, clnt_addr; // ���� ��ü�� �ּ����� + �ӽ÷� ���� Ŭ���� �ּ�����
    int clnt_addr_size; // Ŭ�� �ּ������� ũ�Ⱚ
    pthread_t serv_thread;
    pthread_t cmd_thread;

    /*----------------------------------*/
    // 2. ���Ͽ��� + bind ����
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
    // 3. �������� listen ����
    /*----------------------------------*/

    if(listen(serv_socket, WAIT_NUM) == -1)
    {
        printf("listen error!\n");
        return 0;
    }
    else
        printf("server will start listening from now on\n");

    /*----------------------------------*/
    // 4. listen �����̱⿡, Ŭ��κ��� connect ��� (accept)
    /*----------------------------------*/

    clnt_addr_size = sizeof(clnt_addr);
    pthread_mutex_init(&mtx,NULL);

    system("clear");
    
    printf("if you type '-q', server will be closed safely!\n\n");

    DATA copiedDATA = {0,{0}};
    // ������ ���������� �� ��, �ݺ��� ��ü�� ���ؽ��� ������

    /*----------------------------------*/
    // 5. Ŀ�ǵ带 ���� ������ �����
    /*----------------------------------*/

    pthread_create(&cmd_thread,NULL, input_cmd,(void *) &copiedDATA); // Ŀ�ǵ� �Է��� ���� ������ ����

    while(1)
    {
        pthread_mutex_lock(&mtx); // clnt_cnt �� ���������̱⿡ ���ǹ����� ����ϸ� ������ ���ؽ�

        if(clnt_cnt < CLNT_MAX) // �ִ� 10����� ���� �� �ֱ⿡, 10���� �� �������� �ȹ���
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
            // 6. ������ ���� ������ �����
            /*----------------------------------*/

            pthread_mutex_lock(&mtx);
            clnt_socketList[clnt_cnt++] = clnt_socket; // Ŭ���� ���Ͻĺ��ڰ� ����Ʈ�� ����ǰ�, ������ ��++
            pthread_mutex_unlock(&mtx);

            pthread_create(&serv_thread, NULL, clnt_handler, (void*) &clnt_socket); // 1:1 ê�� ���� ���� ����
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