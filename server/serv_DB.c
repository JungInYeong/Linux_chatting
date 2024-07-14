#include"serv.h"

#ifdef DEBUG // 디버그용

#define INFOSIZE 42
// txt 파일에 id pw 으로 저장됨

void login_ID(int clnt_socket, char packet[]) 
// txt 파일에서 ID가 있는지 확인 
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_id[IDSIZE] = { 0 };   // 로그인 요청 클라이언트 아이디
    char temp_id[IDSIZE] = { 0 };   // txt 파일에서의 아이디 목록
    int seek_result = 0;

    // 1. 파일열기

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. 패킷에서 id 추출
    for(int idx = 3; packet[idx] != '\0'; idx++) 
    // 아이디 로그인 프로세스 패킷구성 -> id sungsu
    {
        clnt_id[idx-3] = packet[idx];
    }

    // 3. txt에서 한줄씩 읽고 그 id와 수신받은 id를 검사
    int str_cursor;

    while(fgets(info, sizeof(info), fp) != NULL) 
    // 파일을 한줄씩 EOF까지 읽어서 거기서 id만 추출해서 검사
    {
        memset(temp_id, 0, sizeof(temp_id)); // 초기화

        for(str_cursor = 0; info[str_cursor] != ' '; str_cursor++) 
        {
            temp_id[str_cursor] = info[str_cursor];
            // 그 한줄에서 id만 추출
        }
        temp_id[str_cursor] = '\0';

        if(!strcmp(temp_id,clnt_id))
        {
            printf("[%d]'s ID found!\n",clnt_socket);
            seek_result = 1;
            break;
        }
    }
    
    // 4. 해당결과를 클라에게 반환
    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void login_PW(int clnt_socket, char packet[]) 
/* --- 그냥 클라이언트가 ID와 PW를 전송하게 하자 안그러면 너무 복잡 --- */
/* --- 생각해보니 클라가 ID와 PW에 띄어쓰기를 넣으면 안됨 --- */
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_account[INFOSIZE] = {0};
    int seek_result = 0;

    // 1. 파일열기

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. 패킷에서 id와 pw만 추출
    
    int idx;
    for( idx = 3; idx < strlen(packet);idx++)
    {
	clnt_account[idx-3] = packet[idx];
    }
    clnt_account[idx] = 0;

    // 3. txt에서 한줄씩 읽고 패킷자체와 그 한줄을 검사

    while(fgets(info, sizeof(info), fp) != NULL) 
    // 패킷구성 -> "id" "pw" == "id" "pw" (txt의 한줄)
    {
        info[strcspn(info, "\n")] = 0; // 개행문자 제거

        if(!strcmp(info,clnt_account))
        {
            printf("client[%d] is Allowed!\n", clnt_socket);
            seek_result = 1;
            break;
        }
    }

    // 4. 해당결과를 클라에게 반환

    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void unique_ID(int clnt_socket, char packet[])
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_id[IDSIZE] = { 0 };   // 로그인 요청 클라이언트 아이디
    char temp_id[IDSIZE] = { 0 };   // txt 파일에서의 아이디 목록
    int seek_result = 1;            // 기본적으로 유니크하다고 가정

    // 1. 파일열기

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. 패킷에서 id 추출
    for(int idx = 3; packet[idx] != '\0'; idx++) 
    // 아이디 고유확인 프로세스 패킷구성 -> id sungsu
    {
        clnt_id[idx-3] = packet[idx];
    }
    clnt_id[strlen(packet) - 3] = '\0'; // clnt_id를 끝내줄것

    // 3. txt에서 한줄씩 읽고 그 id와 수신받은 id를 검사
    int str_cursor;

    while(fgets(info, sizeof(info), fp) != NULL) 
    // 파일을 한줄씩 EOF까지 읽어서 거기서 id만 추출해서 검사
    {
        memset(temp_id, 0, sizeof(temp_id)); // 초기화

        for(str_cursor = 0; info[str_cursor] != ' '; str_cursor++) 
        {
            temp_id[str_cursor] = info[str_cursor];
            // 그 한줄에서 id만 추출
        }
        temp_id[str_cursor] = '\0';
	/*디버그용*/
	printf("temp_id : %s\n",temp_id);

        if(!strcmp(temp_id,clnt_id))
        {
            printf("[%d]'s new ID [%s] is NOT Unique!\n",clnt_socket,clnt_id);
            seek_result = 0;
        }
	
    }

    if(seek_result == 1)
    	printf("[%d]'s new ID [%s] is Unique!\n",clnt_socket,clnt_id);

    // 4. 해당결과를 클라에게 반환
    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void save_Info(int clnt_socket, char packet[])
{
    FILE *fp;
    char info[INFOSIZE] = {0};
    int result = 0;

    // 1. 파일열기
    fp = fopen("testDB.txt","a");
    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. 패킷에서 정보만 추출
    int idx;
    for(idx = 4; packet[idx] != '\0'; idx++)
    // 패킷구조 -> new "ID" "PW"
    {
        info[idx-4] = packet[idx];
    }
    info[idx-4] = '\0';

    // 3. 계정정보 쓰기
    fprintf(fp, "%s\n", info);
    printf("[%d]'s account info has been written at DB!\n",clnt_socket);
    
    // 4. 작성되었다고 알리기
    write(clnt_socket, &result, sizeof(int));

    fclose(fp);
}

#else // 릴리스용

// 정인영이 채울것

#endif
