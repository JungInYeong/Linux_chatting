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

    // 본격적인 수신작용
    while (1)
    {
        if (user_down || (length = read(serv_socket, buffer, BUFFSIZE - 1)) <= 0)
        {
            pthread_mutex_lock(&mtx);
            if (length == 0 && !user_down) // 오류 2 : 서버가 다운됨
            {
                printf("\nrecv : your session with server has been disconnected!\n");
                *errCode = 2;
            }
            else if(user_down)
            {
                printf("\nrecv : user shut down process\n");
                *errCode = 1;
            }
            else // 오류 3 : read 함수의 오작동 (더 깊게 살펴봐야 할 오류)
            {
                printf("\nrecv : unknown error occurred while receiving!\n");
                *errCode = 3;
            }

            session_down = true;
            pthread_mutex_unlock(&mtx);
            break;
        }

        buffer[length] = '\0';
        printf("\nrecv : %s", buffer); // 정상작동
    }

    pthread_cleanup_pop(1);
    printf("recv : process will be turned off soon\n");
    pthread_exit(0);
    return NULL;
}

void* send_msg(void* parameter) // 소켓식별자, 에러코드, id
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
                fgets(msg, MSGSIZE - 1, stdin); // 메시지 입력받기
                msg[strcspn(msg, "\n")] = 0;    // 개행 문자 제거

                // 커맨드 확인
                if (!strcmp(msg, QUIT)) // 유저가 방나가기 커맨드를 입력했을시
                {
                        pthread_mutex_lock(&mtx);
                        printf("\nsend : you choose quit!\n"); // 오류 1 : 유저가 명령으로 직접 통신프로세스 종료

                        *errCode = 1;
                        user_down = true; // join 함수는 main함수안에 있고, 해당 에러코드를 보고 goto로 소켓생성부분으로 이동
                        pthread_mutex_unlock(&mtx);

                        if( !close(serv_socket)) // 이렇게 되면, recv도 안전하게 꺼짐 -> 메모리누수 X
                                printf("socket has been closed!\n");
                        break;
                }
                else if (!strcmp(msg, TERMINATE)) // 유저가 꺼버리기 커맨드를 입력했을시
                {
                        pthread_mutex_lock(&mtx);
                        printf("\nsend :you choose terminate!\n"); // 오류 10 : 유저가 명령으로 직접 쉘 종료

                        *errCode = 10;
                        user_down = true;
                        pthread_mutex_unlock(&mtx);

                        close(serv_socket);
                        break;
                }
                pthread_mutex_lock(&mtx);
                if (session_down && !user_down) // 서버에 의해 꺼짐
                {
                        printf("\nsend : your session with server has been disconnected!\n");
                        pthread_mutex_unlock(&mtx);
                        break;
                }
                pthread_mutex_unlock(&mtx);

                // 패킷 구성
                sprintf(packet, "[%s] : %s\n", id, msg);


                // 패킷 전송
                if (write(serv_socket, packet, strlen(packet) + 1) < 0)
                {
                        printf("send : there is a problem with sending packet!\n"); // 오류 4 : 제대로 패킷 전송못함
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