#include"serv.h"

#ifdef DEBUG // ����׿�

#define INFOSIZE 42
// txt ���Ͽ� id pw ���� �����

void login_ID(int clnt_socket, char packet[])
// txt ���Ͽ��� ID�� �ִ��� Ȯ��
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_id[IDSIZE] = { 0 };   // �α��� ��û Ŭ���̾�Ʈ ���̵�
    char temp_id[IDSIZE] = { 0 };   // txt ���Ͽ����� ���̵� ���
    int seek_result = 0;

    // 1. ���Ͽ���

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. ��Ŷ���� id ����
    for(int idx = 3; packet[idx] != '\0'; idx++)
    // ���̵� �α��� ���μ��� ��Ŷ���� -> id sungsu
    {
        clnt_id[idx-3] = packet[idx];
    }

    // 3. txt���� ���پ� �а� �� id�� ���Ź��� id�� �˻�
    int str_cursor;

    while(fgets(info, sizeof(info), fp) != NULL)
    // ������ ���پ� EOF���� �о �ű⼭ id�� �����ؼ� �˻�
    {
        memset(temp_id, 0, sizeof(temp_id)); // �ʱ�ȭ

        for(str_cursor = 0; info[str_cursor] != ' '; str_cursor++)
        {
            temp_id[str_cursor] = info[str_cursor];
            // �� ���ٿ��� id�� ����
        }
        temp_id[str_cursor] = '\0';

        if(!strcmp(temp_id,clnt_id))
        {
            printf("[%d]'s ID found!\n",clnt_socket);
            seek_result = 1;
            break;
        }
    }

    // 4. �ش����� Ŭ�󿡰� ��ȯ
    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void login_PW(int clnt_socket, char packet[])
/* --- �׳� Ŭ���̾�Ʈ�� ID�� PW�� �����ϰ� ���� �ȱ׷��� �ʹ� ���� --- */
/* --- �����غ��� Ŭ�� ID�� PW�� ���⸦ ������ �ȵ� --- */
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_account[INFOSIZE] = {0};
    int seek_result = 0;

    // 1. ���Ͽ���

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. ��Ŷ���� id�� pw�� ����

    int idx;
    for( idx = 3; idx < strlen(packet);idx++)
    {
        clnt_account[idx-3] = packet[idx];
    }
    clnt_account[idx] = 0;

    // 3. txt���� ���پ� �а� ��Ŷ��ü�� �� ������ �˻�

    while(fgets(info, sizeof(info), fp) != NULL)
    // ��Ŷ���� -> "id" "pw" == "id" "pw" (txt�� ����)
    {
        info[strcspn(info, "\n")] = 0; // ���๮�� ����

        if(!strcmp(info,clnt_account))
        {
            printf("client[%d] is Allowed!\n", clnt_socket);
            seek_result = 1;
            break;
        }
    }

    // 4. �ش����� Ŭ�󿡰� ��ȯ

    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void unique_ID(int clnt_socket, char packet[])
{
    FILE* fp;
    char info[INFOSIZE] = { 0 };
    char clnt_id[IDSIZE] = { 0 };   // �α��� ��û Ŭ���̾�Ʈ ���̵�
    char temp_id[IDSIZE] = { 0 };   // txt ���Ͽ����� ���̵� ���
    int seek_result = 1;            // �⺻������ ����ũ�ϴٰ� ����

    // 1. ���Ͽ���

    fp = fopen("testDB.txt","r");

    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. ��Ŷ���� id ����
    for(int idx = 3; packet[idx] != '\0'; idx++)
    // ���̵� ����Ȯ�� ���μ��� ��Ŷ���� -> id sungsu
    {
        clnt_id[idx-3] = packet[idx];
    }
    clnt_id[strlen(packet) - 3] = '\0'; // clnt_id�� �����ٰ�

    // 3. txt���� ���پ� �а� �� id�� ���Ź��� id�� �˻�
    int str_cursor;

    while(fgets(info, sizeof(info), fp) != NULL)
    // ������ ���پ� EOF���� �о �ű⼭ id�� �����ؼ� �˻�
    {
        memset(temp_id, 0, sizeof(temp_id)); // �ʱ�ȭ

        for(str_cursor = 0; info[str_cursor] != ' '; str_cursor++)
        {
            temp_id[str_cursor] = info[str_cursor];
            // �� ���ٿ��� id�� ����
        }
        temp_id[str_cursor] = '\0';
        /*����׿�*/
        printf("temp_id : %s\n",temp_id);

        if(!strcmp(temp_id,clnt_id))
        {
            printf("[%d]'s new ID [%s] is NOT Unique!\n",clnt_socket,clnt_id);
            seek_result = 0;
        }

    }

    if(seek_result == 1)
        printf("[%d]'s new ID [%s] is Unique!\n",clnt_socket,clnt_id);

    // 4. �ش����� Ŭ�󿡰� ��ȯ
    write(clnt_socket, &seek_result, sizeof(int));

    fclose(fp);
}

void save_Info(int clnt_socket, char packet[])
{
    FILE *fp;
    char info[INFOSIZE] = {0};
    int result = 0;

    // 1. ���Ͽ���
    fp = fopen("testDB.txt","a");
    if(fp == NULL)
    {
        printf("cannot open file!\n");
        return;
    }

    // 2. ��Ŷ���� ������ ����
    int idx;
    for(idx = 4; packet[idx] != '\0'; idx++)
    // ��Ŷ���� -> new "ID" "PW"
    {
        info[idx-4] = packet[idx];
    }
    info[idx-4] = '\0';

    // 3. �������� ����
    fprintf(fp, "%s\n", info);
    printf("[%d]'s account info has been written at DB!\n",clnt_socket);

    // 4. �ۼ��Ǿ��ٰ� �˸���
    write(clnt_socket, &result, sizeof(int));

    fclose(fp);
}

#else // ��������

// ���ο��� ä���

#endif