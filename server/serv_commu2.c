// serv_commu.c
#include "serv.h"

extern char clnt_idList[CLNT_MAX][IDSIZE]; // 클라이언트들의 id 저장
extern pthread_t clnt_threadList[CLNT_MAX]; // 클라이언트들과의 통신스레드 식별자들 저장
extern int clnt_socketList[CLNT_MAX];   // 클라이언트와의 1:1 소켓 식별자들 저장
extern int clnt_cnt;                    // 현 클라수
extern pthread_mutex_t mtx;
extern bool server_down;


void cleanup_handler(void *arg)
{
    int clnt_socket = *(int *)arg;
    printf("clean_up process!\n");

    if(pthread_mutex_trylock(&mtx) == 0) // 뮤텍스락이 안잠김 -> 그래서 함수에서 잠가줌
    {
        for(int i =0; i < clnt_cnt; i++) // 소켓리스트와 스레드리스트 앞으로 땡기기
        {
                if(clnt_socketList[i] == clnt_socket)
                {
			//printf("founded!! -> %d, %d\n",clnt_cnt,clnt_socket);
                        for(; i < clnt_cnt -1; i++)
                        {	
				//printf("%d, %d, %s\t",clnt_cnt,clnt_socketList[i],clnt_idList[i]);
                                clnt_socketList[i] = clnt_socketList[i+1];
                                clnt_threadList[i] = clnt_threadList[i+1];
                                strcpy(clnt_idList[i] , clnt_idList[i+1]);
                        }
                        break;
                }
        }
        // 접종전의 유저 정보 인덱스를 초기화
        clnt_socketList[clnt_cnt-1] = 0;
        memset(&clnt_threadList[clnt_cnt-1], 0, sizeof(pthread_t));
        memset(clnt_idList[clnt_cnt-1], 0, IDSIZE);
	
	printf("clnt_cnt : %d\n",clnt_cnt);
        clnt_cnt --; // 현 접속자수-1
        pthread_mutex_unlock(&mtx);

        close(clnt_socket); // 소켓닫기
        printf("socket [%d] closed!\n",clnt_socket);
    }

    else // 뮤텍스락이 잠겨있음 이미
    {

        for(int i =0; i < clnt_cnt; i++) // 소켓리스트와 스레드리스트 앞으로 땡기기
        {
                if(clnt_socketList[i] == clnt_socket)
                {
                        //printf("founded!! -> %d, %d\n",clnt_cnt,clnt_socket);
                        for(; i < clnt_cnt -1; i++)
                        {
				//printf("%d, %d, %s\t",clnt_cnt,clnt_socketList[i],clnt_idList[i]);
                                clnt_socketList[i] = clnt_socketList[i+1];
                                clnt_threadList[i] = clnt_threadList[i+1];
				strcpy(clnt_idList[i] , clnt_idList[i+1]);
                        }
                break;
                }
        }

        // 접종전의 유저 정보 인덱스를 초기화
        clnt_socketList[clnt_cnt-1] = 0;
        memset(&clnt_threadList[clnt_cnt-1], 0, sizeof(pthread_t));
       	memset(clnt_idList[clnt_cnt-1], 0, IDSIZE);

	printf("clnt_cnt : %d\n",clnt_cnt);
        clnt_cnt --; // 현 접속자수-1

        close(clnt_socket);     // 소켓닫기
        printf("socket [%d] closed!\n",clnt_socket);
    }
}

void* clnt_handler(void *arg)
{
    int clnt_socket = *(int*)arg;
    char buffer[BUFFSIZE] = {0};
    char clnt_id[IDSIZE] = {0};               // 클라이언트 전역배열의 인덱스 번호
    int length = 0;
    bool haveID = false;

    pthread_cleanup_push(cleanup_handler, (void*) &clnt_socket);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1)
    {
        length = read(clnt_socket, buffer, BUFFSIZE);

        if(length <= 0)
        // 고려사항 2 : 클라에게 문제가 있어서 연결이 끊겼을 때
        {
            if(length == 0)
                printf("\nclient [%d] disconnected\n", clnt_socket);
            else
                printf("\nread error!\n");

            if(!haveID)
                sprintf(buffer, "unknown client disconnected!\n");
            else
                sprintf(buffer, "[%s] : is disconnected!\n", clnt_id);

            send_all_clnt(buffer, clnt_socket, false);
            break;
        }
        else
        // 고려사항 3 : 첫글자를 확인해서, 어떤 명령인지 파악
        {
            switch(buffer[0])
            {
                case '[': // 일반 작동방식
                    printf("read client[%d] msg : %s\n", clnt_socket, buffer);
                    send_all_clnt(buffer, clnt_socket, false);
                    break;
                case 'I':
                    login_ID(clnt_socket, buffer);
                    break;
                case 'P':
                    login_PW(clnt_socket, buffer);
                    break;
                case 'i':
                    unique_ID(clnt_socket, buffer);
                    break;
                case 'n':
                    save_Info(clnt_socket, buffer);
                    break;
            }

            if(!haveID)
            // 고려사항 4 : id를 아는가?
                haveID = extractID(clnt_id, clnt_socket);
        }
    }

    pthread_cleanup_pop(1); // 클라와의 스레드는 각각 클린업 핸들러를 개별적으로 가짐
    pthread_exit(0);
    return NULL;
}

void send_all_clnt(char *buffer, int from_clnt_socket, bool is_alarm)
{
    if(is_alarm && from_clnt_socket != 0)   // 서버가 꺼진다고(커맨드) 연결대상 클라에게 알림 (알람)
        write(from_clnt_socket,buffer,strlen(buffer)+1);

    else if(from_clnt_socket == 0) // 서버가 꺼진다고(커맨드) 모든 클라에게 알림 (알람)
    {
        pthread_mutex_lock(&mtx);

        for(int idx = 0; idx < clnt_cnt; idx++)
            write(clnt_socketList[idx], buffer, strlen(buffer)+1);

        pthread_mutex_unlock(&mtx);
    }

    else // 클라에게 받은 메시지를 다른사람들에게 뿌리기
    {
        pthread_mutex_lock(&mtx);

        for(int idx = 0; idx < clnt_cnt; idx++)
        {
            if(clnt_socketList[idx] == from_clnt_socket)
                continue;

            write(clnt_socketList[idx], buffer, strlen(buffer)+1);
        }

        pthread_mutex_unlock(&mtx);
    }
}

bool extractID(char *clnt_id, int clnt_socket)
{
    pthread_mutex_lock(&mtx);
    for(int idx = 0; idx < clnt_cnt; idx++) // 똑같은 사람이면, 모든 전역배열의 정보인덱스는 같다
    {
        if(clnt_socketList[idx] == clnt_socket)
        {
            if(clnt_idList[idx][0] == '\0') // 아직 id가 안들어왔으면
            {
                pthread_mutex_unlock(&mtx);
                return false;
            }
            else // 로그인을 해서 id가 있으면
            {
                strcpy(clnt_id, clnt_idList[idx]);
                pthread_mutex_unlock(&mtx);
                return true; // id 추출성공
            }
        }
    }
    pthread_mutex_unlock(&mtx);
    return false;
}

void* input_cmd(void *Args)
{
    DATA *copiedDATA = Args;
    char command[CMDLEN] = {0};
    char msg[BUFFSIZE] = {0};
    int selected_socket;    // kick 유저 고르기

    while(1)
    {
        fgets(command, CMDLEN-1, stdin);        // 커맨드 입력
        command[strcspn(command, "\n")] = 0;    // 개행 없애기

        if(!strcmp(command, QUIT))              // 1. 서버 고의 다운 커맨드
        {
            //pthread_mutex_lock(&mtx);
            send_all_clnt("Admin: server will down!\n", 0, false);  // 모든유저에게 곧 꺼진다고 알리기
            server_down = true;
            //pthread_mutex_unlock(&mtx);

            break; // 커맨드창 루프탈출
        }

        else if(!strcmp(command, KICK))         // 2. 특정 클라 강퇴 커맨드
        {
            /*--- 1. 클라이언트 디스크립터 입력받기 ---*/

            printf("input Number: _\b");
            scanf("%d", &selected_socket);
            while(getchar() != '\n');
            putchar('\n');

            strcpy(msg, "Admin: you have been kicked!\n");

            /*--- 2. 해당 디스크립터가 존재하는지 확인 ---*/
            pthread_mutex_lock(&mtx);
            int count;
            for(count = 0; count < clnt_cnt; count++)   // 입력한 디스크립터가 혀재 있는 사람들인가?
            {
                if(selected_socket == clnt_socketList[count])
                        break;
            }

            if(count == clnt_cnt) /*--- 3.1. 해당 디스크립터가 존재하는지 확인 -> 없음 ---*/
            {
                printf("there is no User having [%d]!\n", selected_socket);
                pthread_mutex_unlock(&mtx); // continue 되기에 여기서 풀어주기
                continue;
            }
            else /*--- 3.1. 해당 디스크립터가 존재하는지 확인 -> 있음 ---*/
            {
                printf("User[%d] Kick process!\n", selected_socket);
                write(selected_socket, msg, strlen(msg) + 1);   // 해당 클라에게 강퇴메시지 보내기

                /*--- 4. 해당 디스크립터를 갖는 스레드를 종료 + 리스트정보에서 삭제 ---*/
                int theClnt;
                for(theClnt = 0; theClnt < clnt_cnt; theClnt++)
                {
                    if(selected_socket == clnt_socketList[theClnt])
                    {
                        if(pthread_cancel(clnt_threadList[theClnt]) != 0)
                                perror("pthread_cancel err!\n");        // 해당클라와 연결하는 스레드 종료 -> 종료되면서, 소멸자(클린업핸들러) 실행됨
                        if (pthread_join(clnt_threadList[theClnt], NULL) != 0)   // 종료 기다리기
                                perror("pthread_join err!\n");
                        break;
                    }
                }
                pthread_mutex_unlock(&mtx);
            }
        }

        else if(!strcmp(command, PRNT))     // 3. 현재 접속한 클라이언트들의 수, 클라들과 연결된 디스크립터 확인
        {
            pthread_mutex_lock(&mtx);
            printf("number of clients: %d\n", clnt_cnt);
            if(clnt_cnt > 0)
            {
                for(int idx = 0; idx < clnt_cnt; idx++)
                    printf("[%d, %s] ", clnt_socketList[idx], clnt_idList[idx]);

                putchar('\n');
            }
            else
                printf("No client\n");

            pthread_mutex_unlock(&mtx);
        }
    }


    pthread_exit(0);
    return NULL;
}
