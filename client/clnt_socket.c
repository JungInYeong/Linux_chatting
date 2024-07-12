#include "clnt.h"

extern bool session_down;
extern bool user_down;
extern pthread_mutex_t mtx;

bool is_correctIP(char IP[], int length)
{
        int dotCnt = 0;
        int charDiff = 0;

        for (int i = 0; i < length; i++)
        {
                if (IP[i] == '.')
                {
                        dotCnt++;
                        continue;
                }

                charDiff = IP[i] - '0';

                if (!(0 <= charDiff && charDiff <= 9)) // 0~9������ ������ Ȯ��
                        return false;
        }

        if (dotCnt != 3) // ip �ּҿ� .�� 3���� �ƴ� -> ��ȿ���� ���� ip
                return false;

        return true;
}

bool login_process(int serv_socket, int* errCode, char* userID)
{
        char inputID[IDSIZE];
        char inputPW[IDSIZE];
        char inputPW2[IDSIZE];
        char DBmsg[DBPKSIZE];

        int IDsuccess = 0;      // �������� (bool �� ���⿡�� ����Ģ�ؼ�)
        int PWsuccess = 0;
        int length;                     // ���Ź��� ��Ŷ�� ����

        // ID Ȯ��

        while (!IDsuccess)
        {
                //system("clear");
                memset(DBmsg, 0, DBPKSIZE);
                memset(inputID, 0, IDSIZE);

                printf("'q' is quit this sequence\ninput your ID : ");
                fgets(inputID, IDSIZE - 1, stdin);      // 20�ڱ����� �Է¹���
                inputID[strcspn(inputID, "\n")] = 0; // ���� ���� ����

                if (strcmp(inputID, "q") == 0 || strcmp(inputID, "Q") == 0)
                        // ���� 1 : ����ڰ� �����ϰ� �������� ���
                {
                        printf("\nquitting this sequence...\n");
                        *errCode = 1;
                        return false;
                }

                sprintf(DBmsg, "ID %s", inputID);       // ä���� �ƴ϶��, ù��° ���ڰ� [ �� �ƴϴ�
                printf("\nthe server will verify that ID is correct\n");

                if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0)// ������ ID ������
                {
                        printf("there is a problem with sending ID process!\n"); // ���� 2 : ����� ID ���۸���
                        *errCode = 2;
                        return false; // �ٷ� ��������
                }

                /*------------------------*/
                // read �Լ��� ���Ź��� ������ ��ٸ���
                /*-------------------------*/

                if ((length = read(serv_socket, &IDsuccess, sizeof(int))) != sizeof(int)) // ����� ���Ź��� ����
                {
                        printf("there is a problem with receiving result process!\n"); // ���� 3 : ����� ID ��� ���Ÿ�����
                        *errCode = 3;
                        return false; // �ٷ� ��������
                }

                if (!IDsuccess)
                        printf("Invalid ID, Try again\n");
        }

        // PW Ȯ��

        while (!PWsuccess)
        {
                //system("clear");
                printf("\n\nID is correct!\n\n");
                memset(DBmsg, 0, DBPKSIZE);
                memset(inputPW, 0, IDSIZE);
                memset(inputPW2, 0, IDSIZE);

                printf("'q' is quit this sequence\ninput your PW : ");
                fgets(inputPW, IDSIZE - 1, stdin);      // 20�ڱ����� �Է¹���
                inputPW[strcspn(inputPW, "\n")] = 0;

                if (strcmp(inputPW, "q") == 0 || strcmp(inputPW, "Q") == 0)
                        // ���� 1 : ����ڰ� �����ϰ� �������� ���
                {
                        printf("\nquitting this sequence...\n");
                        *errCode = 1;
                        return false;
                }


                sprintf(DBmsg, "PW %s %s", inputID, inputPW);
                printf("\nthe server will verify that PW is correct\n");

                if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // ������ PW ������
                {
                        printf("there is a problem with sending PW process!\n"); // ���� 4 : ����� PW ���۸���
                        *errCode = 4;
                        return false;
                }

                /*------------------------*/
                // read �Լ��� ���Ź��� ������ ��ٸ���
                /*-------------------------*/

                if ((length = read(serv_socket, &PWsuccess, sizeof(int))) != sizeof(int)) // ����� ���Ź��� ����
                {
                        printf("there is a problem with receiving PW result process!\n"); // ���� 5 : ����� PW ��� ���Ÿ�����
                        *errCode = 5;
                        return false; // �ٷ� ��������
                }

                if (!PWsuccess)
                        printf("Invalid PW, Try again\n");
        }

        printf("PW is correct!\nmoving to chatting screen...\n");

        *errCode = 0;
        strcpy(userID, inputID);
        return true;
}

bool signup_process(int serv_socket, int* errCode, char* userID)
{
        char inputID[IDSIZE];
        char inputPW[IDSIZE];
        char inputPW2[IDSIZE];
        char DBmsg[DBPKSIZE];
        char ans;

        int IDsuccess = 0;
        int PWsuccess = 0;
        int Savesuccess = 0;
        int length = 0;

restart:

        // ID �Է�
        while (!IDsuccess)
        {
                memset(inputID, 0, IDSIZE);
                //system("clear");

                printf("the number of characters is limited to 20 characters\n");
                printf("'q' is quit this sequence\ninput your Penguin's ID : ");

                fgets(inputID, IDSIZE - 1, stdin);      // 20�ڱ����� �Է¹���
                inputID[strcspn(inputID, "\n")] = 0;

                if (strcmp(inputID, "q") == 0 || strcmp(inputID, "Q") == 0)
                        // ���� 1 : ����ڰ� �����ϰ� �������� ���
                {
                        printf("\nquitting this sequence...\n");
                        *errCode = 1;
                        return false;
                }

                if(!follow_rules(inputID))      // ��Ģ : 7���̻� && ���� ����
                        continue;

                printf("\nplease double check if your ID is correct [%s]\n", inputID);  // 21���̻��� �ȵ�� ���⿡ ��Ȯ��
                printf("is that right? : [y/n] : ");
                scanf("%c", &ans);
                while (getchar() != '\n');

                if (ans == 'y' || ans == 'Y') // ������� id�� ������
                {
                        printf("your ID has been determined\n");
                        printf("the server will see if it is a unique id...\n");
                }
                else // ó������ �ٽ�
                {
                        printf("try again!\n");
                        continue;
                }

                memset(DBmsg, 0, DBPKSIZE);
                sprintf(DBmsg, "id %s", inputID);

                if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // ������ ID ������
                {
                        printf("there is a problem with sending ID process!\n"); // ���� 6 : ����� ID ���۸���
                        *errCode = 6;
                        return false;
                }

                if ((length = read(serv_socket, &IDsuccess, sizeof(int))) != sizeof(int))
                {
                        printf("there is a problem with receiving result process!\n"); // ���� 7 : ID�� �������� ����� ���Ź��� ����
                        *errCode = 7;
                        return false;
                }

                if (!IDsuccess)
                        printf("ID already exists, try again\n");

                // ���⼭ id�� ������ �ԷµǴ� ���� �ƴ�, pw���� �Է¹ް�, �ѹ��� ���ļ� ������ ���۵ȴ�.
                // ����� �׳� id�� �������� Ȯ���� ���̴�.
        }

        // PW �Է�

        while (!PWsuccess)
        {
                //system("clear");
                memset(inputPW, 0, IDSIZE);
                memset(inputPW2, 0, IDSIZE);

                printf("the number of characters is limited to 20 characters\n");
                printf("'q' is quit this sequence\n");
                printf("input your Penguin accounts[%s]'s PW : ", inputID);

                fgets(inputPW, IDSIZE - 1, stdin);      // 20�ڱ����� �Է¹���
                inputPW[strcspn(inputPW, "\n")] = 0;

                if (strcmp(inputPW, "q") == 0 || strcmp(inputPW, "Q") == 0)
                        // ���� 1 : ����ڰ� �����ϰ� �������� ���
                {
                        printf("\nquitting this sequence...\n");
                        *errCode = 1;
                        return false;
                }

                if(!follow_rules(inputPW))      // ��Ģ : 7���̻� && ���� ����
                        continue;

                printf("please input your PW again : "); // pw �ٽ� �Է¹ޱ�
                fgets(inputPW2, IDSIZE - 1, stdin);
                inputPW2[strcspn(inputPW2, "\n")] = 0;

                if(!follow_rules(inputPW2))      // ��Ģ : 7���̻� && ���� ����
                        continue;

                if (strcmp(inputPW, inputPW2)) // �ΰ��� ������ Ȯ��
                        printf("\nyour passwords do not match, try again\n");
                else
                {
                        printf("\nyour passwords match\n");
                        PWsuccess = 1;
                }
        }

        // �߰��� ����ڰ� ���������� �ֱ� ������, ID�� ����� ���ļ� ������ ����
        printf("sending the account information to the server...\n");

        memset(DBmsg, 0, DBPKSIZE);
        sprintf(DBmsg, "new %s %s", inputID, inputPW);

        if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // ������ �������� ������
        {
                printf("there was a problem sending account information\n"); // ���� 8 : ����� �������� ���۸���
                *errCode = 8;
                return false;
        }

        if ((length = read(serv_socket, &Savesuccess, sizeof(int))) != sizeof(int))
        {
                printf("the server encountered a problem saving the account\n"); // �������� ����� ������ ������ ��,
                printf("wanna restart sign_up process? [y/n] : ");
                scanf("%c", &ans);
                while (getchar() != '\n');

                if (ans == 'y' || ans == 'Y')
                {
                        IDsuccess = 0;
                        PWsuccess = 0;
                        goto restart;
                }

                else
                {
                        printf("\nquitting this sequence...\n");
                        return false;
                }
        }
        else // ȸ�����ԿϷ�
        {
                printf("sign up process has been completed!\n");
                printf("you can now enter the server with this account information!\n");
                printf("moving to chatting screen...\n");
                strcpy(userID, inputID);
                *errCode = 0;
                return true;
        }
}

bool follow_rules(char input[])
{
    // 1. ���ڼ��� 7�̻��ϰ�
    if(strlen(input) < 7)
    {
        printf("please enter more than 7 characters, try again!\n");
        return false;
    }

    // 2. �Է� ���ڿ��� ���Ⱑ �ִ°�?
    for(int i=0; i<strlen(input); i++)
    {
        if(input[i] == ' ')
        {
            printf("when entering, do not use space, try again!\n");
            return false;
        }
    }

    return true;
}