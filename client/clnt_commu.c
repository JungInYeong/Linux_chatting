#include "clnt.h"

extern bool session_down;
extern bool user_down;
extern pthread_mutex_t mtx;

void clean_recv()
{
        printf("\nrecv : user shut down process\n");
        printf("recv : process will be turned off soon\n");
}

void* recv_msg(void* parameter)
{
    Args* args = (Args*)parameter;
    int serv_socket = args->serv_socket;
    int* errCode = args->errCode;

    char buffer[BUFFSIZE] = { 0 };
    int length;

    pthread_cleanup_push(clean_recv,NULL);

    // �������� �����ۿ�
    while (1)
    {
        if (user_down || (length = read(serv_socket, buffer, BUFFSIZE - 1)) <= 0)
        {
            pthread_mutex_lock(&mtx);
            if (length == 0 && !user_down) // ���� 2 : ������ �ٿ��
            {
                printf("\nrecv : your session with server has been disconnected!\n");
                *errCode = 2;
            }
            else if(user_down)
            {
                printf("\nrecv : user shut down process\n");
                *errCode = 1;
            }
            else // ���� 3 : read �Լ��� ���۵� (�� ��� ������� �� ����)
            {
                printf("\nrecv : unknown error occurred while receiving!\n");
                *errCode = 3;
            }

            session_down = true;
            pthread_mutex_unlock(&mtx);
            break;
        }

        buffer[length] = '\0';
        printf("\nrecv : %s", buffer); // �����۵�
    }

    pthread_cleanup_pop(1);
    printf("recv : process will be turned off soon\n");
    pthread_exit(0);
    return NULL;
}

void* send_msg(void* parameter) // ���Ͻĺ���, �����ڵ�, id
{
        Args* args = (Args*)parameter;
        int serv_socket = args->serv_socket;
        int* errCode = args->errCode;
        char id[IDSIZE] = { 0 };
        strcpy(id, args->userID);

        char msg[MSGSIZE] = { 0 };
        char packet[PACKSIZE] = { 0 }; // id + msg = packet

        printf("user command list\n");
        printf("-q : quit from communication\n");
        printf("-t : terminate from shell\n");

        while (1)
        {
                printf("send : input[100] : ");
                fgets(msg, MSGSIZE - 1, stdin); // �޽��� �Է¹ޱ�
                msg[strcspn(msg, "\n")] = 0;    // ���� ���� ����

                // Ŀ�ǵ� Ȯ��
                if (!strcmp(msg, QUIT)) // ������ �泪���� Ŀ�ǵ带 �Է�������
                {
                        pthread_mutex_lock(&mtx);
                        printf("\nsend : you choose quit!\n"); // ���� 1 : ������ ������� ���� ������μ��� ����

                        *errCode = 1;
                        user_down = true; // join �Լ��� main�Լ��ȿ� �ְ�, �ش� �����ڵ带 ���� goto�� ���ϻ����κ����� �̵�
                        pthread_mutex_unlock(&mtx);

                        if( !close(serv_socket)) // �̷��� �Ǹ�, recv�� �����ϰ� ���� -> �޸𸮴��� X
                                printf("socket has been closed!\n");
                        break;
                }
                else if (!strcmp(msg, TERMINATE)) // ������ �������� Ŀ�ǵ带 �Է�������
                {
                        pthread_mutex_lock(&mtx);
                        printf("\nsend :you choose terminate!\n"); // ���� 10 : ������ ������� ���� �� ����

                        *errCode = 10;
                        user_down = true;
                        pthread_mutex_unlock(&mtx);

                        close(serv_socket);
                        break;
                }
                pthread_mutex_lock(&mtx);
                if (session_down && !user_down) // ������ ���� ����
                {
                        printf("\nsend : your session with server has been disconnected!\n");
                        pthread_mutex_unlock(&mtx);
                        break;
                }
                pthread_mutex_unlock(&mtx);

                // ��Ŷ ����
                sprintf(packet, "[%s] : %s\n", id, msg);


                // ��Ŷ ����
                if (write(serv_socket, packet, strlen(packet) + 1) < 0)
                {
                        printf("send : there is a problem with sending packet!\n"); // ���� 4 : ����� ��Ŷ ���۸���
                        pthread_mutex_lock(&mtx);
                        *errCode = 4;
                        session_down = true;
                        pthread_mutex_unlock(&mtx);
                        break;
                }
        }

        printf("send : process will be turned off soon\n");
        pthread_exit(0);
        return NULL;
}