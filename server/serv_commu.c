#include "serv.h"

extern int clnt_socketList[CLNT_MAX];   // 클라이언트와의 1:1 소켓 식별자들 저장
extern int clnt_cnt;                    // 임시로 받을 클라소켓 식별자
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
        // 고려사항 1 : 관리자가 고의로 서버를 내렸을 때
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

        if( (length = read(clnt_socket, buffer, BUFFSIZE)) <= 0) // 클라로부터 수신중
        {
            // 고려사항 2 : 관리자가 고의로 서버를 내렸을 때
            if(length == 0)
                printf("\nclient [%d] disconnected\n",clnt_socket);
            else
                printf("\nread error!\n"); // 해당 1:1 스레드 반환까지 할것

            if(!haveID)
                sprintf(buffer, "unknown client disconnected!\n"); // 아직 id가 뭔지 모를때
            else
                sprintf(buffer, "[%s] : is disconnected!\n",clnt_id); // id를 알 때

            send_all_clnt(buffer,clnt_socket,false); // 유저가 접종했다고 알림

            break; // 탈출 & 스레드종료
        }
        else    // 문제없음
        {
            /*-------------------------------------*/
            // 고려사항 3 : 첫글자를 확인해서, 어떤 명령인지 파악
            /*-------------------------------------*/

            switch(buffer[0])
            {
                case '[':
                // 이게 가장 많은 트래픽을 차지할 것이기에 제일먼저 빠져나가기
                printf("read client[%d] msg : %s\n", clnt_socket, buffer);
                send_all_clnt(buffer, clnt_socket,false);
                break;

                case 'I':
                // DB에 ID가 존재하면 1을 클라에게 전송
                login_ID(clnt_socket,buffer);
                break;

                case 'P':
                // DB에 ID와 PW가 둘다 맞으면 1을 클라에게 전송
                login_PW(clnt_socket,buffer);
                break;

                case 'i':
                // DB에 ID가 없으면 1을 클라에게 전송
                unique_ID(clnt_socket,buffer);
                break;

                case 'n':
                // DB에 ID와 PW를 저장. 저장 후 1을 클라에게 전송
                save_Info(clnt_socket,buffer);
                break;
            }

            /*-------------------------------------*/
            // 고려사항 4 : id를 추출하기
            /*-------------------------------------*/

            if(!haveID && buffer[0] == '[')
            {
                extractID(clnt_id,buffer); // 버퍼에서 id만 추출해서 clntID에 담음
                haveID = true;
            }

            /*-------------------------------------*/
            // 일반적인 채팅 작동방식
            /*-------------------------------------*/
        }
    }

    pthread_mutex_lock(&mtx);
    for(int idx = 0; idx < clnt_cnt; idx ++) // 해당 클라와 연결끊기
    {
        if(clnt_socket == clnt_socketList[idx])
        {
            for(; idx < clnt_cnt-1; idx++) // 클라 리스트의 요소들을 한칸씩 전부 앞으로 밀기
                    clnt_socketList[idx] = clnt_socketList[idx+1];

            break;
        }
    }

    clnt_cnt--; // 접속자수-1
    pthread_mutex_unlock(&mtx);

    // 자원반환
    close(clnt_socket);
    pthread_exit(0);
    return NULL;
}

void send_all_clnt(char *buffer, int from_clnt_socket, bool is_alarm) // 다른 사용자들에게만 패킷보내기
{

    if(is_alarm && from_clnt_socket != 0) // 서버가 꺼진다고 연결대상 클라에게 알림
    {
        write(from_clnt_socket,buffer,strlen(buffer));
        printf("alarm has been sent to the client, server will be down!\n");
    }

    else if(from_clnt_socket == 0)  // 전체 클라이언트들에게 서버가 꺼진다고 알리기
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

    for(idx = 0; buffer[idx+1] != ']'; idx++) // [] 사이의 글자가 곧 id 이다.
        clnt_id[idx] = buffer[idx+1];

    clnt_id[idx+1] = '\0'; // 널추가
    return;
}

void* input_cmd(void* Args)
{
    DATA* copiedDATA = Args;
    char command[CMDLEN] = {0};             // 커맨드 입력창
    char msg[BUFFSIZE] = {0};

    while (true)
    {
        fgets(command, CMDLEN-1, stdin);        // 커맨드 입력
        command[strcspn(command, "\n")] = 0;    // 개행 없애기
        int clnt_socket;

        if(!strcmp(command,QUIT))   // 서버다운 커맨드가 실행됨
        {
            send_all_clnt("Admin : server will down!\n",0,false); // 전체클라들에게 서버종료를 알림

            pthread_mutex_lock(&mtx);
            copiedDATA->temp_cnt = clnt_cnt;
            memcpy(copiedDATA->temp_List, clnt_socketList, sizeof(int) * CLNT_MAX); // 뮤텍스로 이걸 못덮기 때문

            server_down = true; // 각 스레드들은 이걸보고 전부 종료시킨다
            pthread_mutex_unlock(&mtx);
        }

        else if(!strcmp(command,KICK))    // 강퇴 커맨드가 실행됨 
        {
            printf("input Number : _\b"); // 강퇴할 소켓식별자 입력
            scanf("%d",clnt_socket);
            while(getchar() != '\n');
            putchar('\n');

            strcpy(msg,"Admin : you has been kicked!\n");

            pthread_mutex_lock(&mtx);

            /* 클라 리스트에 있는지 확인 */

            write(clnt_socket,msg,strlen(msg) + 1); // 해당 클라에게 강퇴메시지 보내기

            for(int idx = 0; idx < clnt_cnt; idx ++) // 해당 클라정보 리스트에서 삭제
            {
                if(clnt_socket == clnt_socketList[idx])
                {
                    for(; idx < clnt_cnt-1; idx++) // 클라 리스트의 요소들을 한칸씩 전부 앞으로 밀기
                        clnt_socketList[idx] = clnt_socketList[idx+1];

                    break;
                }
            }

            clnt_cnt--; // 접속자수-1
            pthread_mutex_unlock(&mtx);

            close(clnt_socket);
        }

        else if(!strcmp(command,PRNT))
        {
            pthread_mutex_lock(&mtx);
            
            printf("number of clients : %d",clnt_cnt);
            
            if(clnt_cnt > 0) // 클라가 단 한명이라도 존재시,
            {
                for(int idx = 0; idx < clnt_cnt; idx++)
                    printf("[%d] ", clnt_socketList[idx]);
                
                putchar('\n');
            }
            else            // 없을 때
                printf("No client\n");

            pthread_mutex_unlock(&mtx);
        }

    }
    


    pthread_exit(0);
    return NULL;
}