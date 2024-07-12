#include "serv.h"

extern int clnt_socketList[CLNT_MAX];   // Ŭ���̾�Ʈ���� 1:1 ���� �ĺ��ڵ� ����
extern int clnt_cnt;                    // �ӽ÷� ���� Ŭ����� �ĺ���
extern pthread_mutex_t mtx;
extern bool server_down;

void* clnt_handler(void *arg)
{
    int clnt_socket = *(int*)arg;
    char buffer[BUFFSIZE] = {0};
    char clnt_id[IDSIZE] = {0};
    int length = 0;
    bool haveID = false;

    while(1)
    {
        // ������� 1 : �����ڰ� ���Ƿ� ������ ������ ��
        pthread_mutex_lock(&mtx);
        if(server_down)
        {
            pthread_mutex_unlock(&mtx);
            sprintf(buffer, "caution : server admin try to terminate server!\n");
            send_all_clnt(buffer,clnt_socket,true);
            break;
        }
        else
            pthread_mutex_unlock(&mtx);

        if( (length = read(clnt_socket, buffer, BUFFSIZE)) <= 0) // Ŭ��κ��� ������
        {
            // ������� 2 : �����ڰ� ���Ƿ� ������ ������ ��
            if(length == 0)
                printf("\nclient [%d] disconnected\n",clnt_socket);
            else
                printf("\nread error!\n"); // �ش� 1:1 ������ ��ȯ���� �Ұ�

            if(!haveID)
                sprintf(buffer, "unknown client disconnected!\n"); // ���� id�� ���� �𸦶�
            else
                sprintf(buffer, "[%s] : is disconnected!\n",clnt_id); // id�� �� ��

            send_all_clnt(buffer,clnt_socket,false); // ������ �����ߴٰ� �˸�

            break; // Ż�� & ����������
        }
        else    // ��������
        {
            /*-------------------------------------*/
            // ������� 3 : ù���ڸ� Ȯ���ؼ�, � ������� �ľ�
            /*-------------------------------------*/

            switch(buffer[0])
            {
                case '[':
                // �̰� ���� ���� Ʈ������ ������ ���̱⿡ ���ϸ��� ����������
                printf("read client[%d] msg : %s\n", clnt_socket, buffer);
                send_all_clnt(buffer, clnt_socket,false);
                break;

                case 'I':
                // DB�� ID�� �����ϸ� 1�� Ŭ�󿡰� ����
                login_ID(clnt_socket,buffer);
                break;

                case 'P':
                // DB�� ID�� PW�� �Ѵ� ������ 1�� Ŭ�󿡰� ����
                login_PW(clnt_socket,buffer);
                break;

                case 'i':
                // DB�� ID�� ������ 1�� Ŭ�󿡰� ����
                unique_ID(clnt_socket,buffer);
                break;

                case 'n':
                // DB�� ID�� PW�� ����. ���� �� 1�� Ŭ�󿡰� ����
                save_Info(clnt_socket,buffer);
                break;
            }

            /*-------------------------------------*/
            // ������� 4 : id�� �����ϱ�
            /*-------------------------------------*/

            if(!haveID && buffer[0] == '[')
            {
                extractID(clnt_id,buffer); // ���ۿ��� id�� �����ؼ� clntID�� ����
                haveID = true;
            }

            /*-------------------------------------*/
            // �Ϲ����� ä�� �۵����
            /*-------------------------------------*/
        }
    }

    pthread_mutex_lock(&mtx);
    for(int idx = 0; idx < clnt_cnt; idx ++) // �ش� Ŭ��� �������
    {
        if(clnt_socket == clnt_socketList[idx])
        {
            for(; idx < clnt_cnt-1; idx++) // Ŭ�� ����Ʈ�� ��ҵ��� ��ĭ�� ���� ������ �б�
                    clnt_socketList[idx] = clnt_socketList[idx+1];

            break;
        }
    }

    clnt_cnt--; // �����ڼ�-1
    pthread_mutex_unlock(&mtx);

    // �ڿ���ȯ
    close(clnt_socket);
    pthread_exit(0);
    return NULL;
}

void send_all_clnt(char *buffer, int from_clnt_socket, bool is_alarm) // �ٸ� ����ڵ鿡�Ը� ��Ŷ������
{

    if(is_alarm && from_clnt_socket != 0) // ������ �����ٰ� ������ Ŭ�󿡰� �˸�
    {
        write(from_clnt_socket,buffer,strlen(buffer));
        printf("alarm has been sent to the client, server will be down!\n");
    }

    else if(from_clnt_socket == 0)  // ��ü Ŭ���̾�Ʈ�鿡�� ������ �����ٰ� �˸���
    {
        pthread_mutex_lock(&mtx);

        for(int idx = 0; idx < clnt_cnt; idx++)
                    write(clnt_socketList[idx], buffer, strlen(buffer));

        pthread_mutex_unlock(&mtx);
    }

    else
    {
        pthread_mutex_lock(&mtx);

        for(int idx = 0; idx < clnt_cnt; idx++)
            {
                    if(clnt_socketList[idx] == from_clnt_socket)
                            continue;

                    write(clnt_socketList[idx], buffer, strlen(buffer));
            }

        pthread_mutex_unlock(&mtx);
    }
}

void extractID(char clnt_id[],char buffer[])
{
    int idx;

    for(idx = 0; buffer[idx+1] != ']'; idx++) // [] ������ ���ڰ� �� id �̴�.
        clnt_id[idx] = buffer[idx+1];

    clnt_id[idx+1] = '\0'; // ���߰�
    return;
}

void* input_cmd(void* Args)
{
    DATA* copiedDATA = Args;
    char command[CMDLEN] = {0};             // Ŀ�ǵ� �Է�â
    char msg[BUFFSIZE] = {0};

    while (true)
    {
        fgets(command, CMDLEN-1, stdin);        // Ŀ�ǵ� �Է�
        command[strcspn(command, "\n")] = 0;    // ���� ���ֱ�
        int clnt_socket;

        if(!strcmp(command,QUIT))   // �����ٿ� Ŀ�ǵ尡 �����
        {
            send_all_clnt("Admin : server will down!\n",0,false); // ��üŬ��鿡�� �������Ḧ �˸�

            pthread_mutex_lock(&mtx);
            copiedDATA->temp_cnt = clnt_cnt;
            memcpy(copiedDATA->temp_List, clnt_socketList, sizeof(int) * CLNT_MAX); // ���ؽ��� �̰� ������ ����

            server_down = true; // �� ��������� �̰ɺ��� ���� �����Ų��
            pthread_mutex_unlock(&mtx);
        }

        else if(!strcmp(command,KICK))    // ���� Ŀ�ǵ尡 ����� 
        {
            printf("input Number : _\b"); // ������ ���Ͻĺ��� �Է�
            scanf("%d",clnt_socket);
            while(getchar() != '\n');
            putchar('\n');

            strcpy(msg,"Admin : you has been kicked!\n");

            pthread_mutex_lock(&mtx);

            /* Ŭ�� ����Ʈ�� �ִ��� Ȯ�� */

            write(clnt_socket,msg,strlen(msg) + 1); // �ش� Ŭ�󿡰� ����޽��� ������

            for(int idx = 0; idx < clnt_cnt; idx ++) // �ش� Ŭ������ ����Ʈ���� ����
            {
                if(clnt_socket == clnt_socketList[idx])
                {
                    for(; idx < clnt_cnt-1; idx++) // Ŭ�� ����Ʈ�� ��ҵ��� ��ĭ�� ���� ������ �б�
                        clnt_socketList[idx] = clnt_socketList[idx+1];

                    break;
                }
            }

            clnt_cnt--; // �����ڼ�-1
            pthread_mutex_unlock(&mtx);

            close(clnt_socket);
        }

        else if(!strcmp(command,PRNT))
        {
            pthread_mutex_lock(&mtx);
            
            printf("number of clients : %d",clnt_cnt);
            
            if(clnt_cnt > 0) // Ŭ�� �� �Ѹ��̶� �����,
            {
                for(int idx = 0; idx < clnt_cnt; idx++)
                    printf("[%d] ", clnt_socketList[idx]);
                
                putchar('\n');
            }
            else            // ���� ��
                printf("No client\n");

            pthread_mutex_unlock(&mtx);
        }

    }
    


    pthread_exit(0);
    return NULL;
}