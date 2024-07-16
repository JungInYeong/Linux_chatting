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

		if (!(0 <= charDiff && charDiff <= 9)) // 0~9까지의 수인지 확인
			return false;
	}

	if (dotCnt != 3) // ip 주소에 .이 3개가 아님 -> 유효하지 않은 ip
		return false;

	return true;
}

bool login_process(int serv_socket, int* errCode, char* userID)
{
	char inputID[IDSIZE];
	char inputPW[IDSIZE];
	char inputPW2[IDSIZE];
	char DBmsg[DBPKSIZE];

	int IDsuccess = 0;	// 성공여부 (bool 을 쓰기에는 께름칙해서)
	int PWsuccess = 0;
	int length;			// 수신받은 패킷의 길이

	// ID 확인

	while (!IDsuccess)
	{
		//system("clear");
		memset(DBmsg, 0, DBPKSIZE);
		memset(inputID, 0, IDSIZE);

		printf("'q' is quit this sequence\ninput your ID : ");
		fgets(inputID, IDSIZE - 1, stdin);	// 20자까지만 입력받음
		inputID[strcspn(inputID, "\n")] = 0; // 개행 문자 제거

		if (strcmp(inputID, "q") == 0 || strcmp(inputID, "Q") == 0)
			// 오류 1 : 사용자가 포기하고 꺼버리는 경우
		{
			printf("\n\033[0;31mSYS : quitting this sequence...\n\033[0m");
			*errCode = 1;
			return false;
		}

		sprintf(DBmsg, "ID %s", inputID);	// 채팅이 아니라면, 첫번째 문자가 [ 가 아니다
		printf("\n\033[0;31mSYS : the server will verify that ID is correct\n\033[0m");

		if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0)// 서버에 ID 보내기
		{
			printf("\033[0;31mSYS : there is a problem with sending ID process!\n\033[0m"); // 오류 2 : 제대로 ID 전송못함
			*errCode = 2;
			return false; // 바로 꺼버리기
		}

		/*------------------------*/
		// read 함수는 수신받을 때까지 기다린다
		/*-------------------------*/

		if ((length = read(serv_socket, &IDsuccess, sizeof(int))) != sizeof(int)) // 제대로 수신받지 못함
		{
			printf("\033[0;31mSYS : there is a problem with receiving result process!\n\033[0m"); // 오류 3 : 제대로 ID 결과 수신못받음
			*errCode = 3;
			return false; // 바로 꺼버리기
		}

		if (!IDsuccess)
			printf("\033[0;31mSYS : Your ID is Invalid or already connected, Try again\n\033[0m");
	}

	// PW 확인

	while (!PWsuccess)
	{
		//system("clear");
		printf("\033[0;32m\n\nID is correct!\n\n\033[0m");
		memset(DBmsg, 0, DBPKSIZE);
		memset(inputPW, 0, IDSIZE);
		memset(inputPW2, 0, IDSIZE);

		printf("'q' is quit this sequence\ninput your PW : ");
		fgets(inputPW, IDSIZE - 1, stdin);	// 20자까지만 입력받음
		inputPW[strcspn(inputPW, "\n")] = 0;			

		if (strcmp(inputPW, "q") == 0 || strcmp(inputPW, "Q") == 0)
			// 오류 1 : 사용자가 포기하고 꺼버리는 경우
		{
			printf("\n\033[0;31mSYS : quitting this sequence...\n\033[0m");
			*errCode = 1;
			return false;
		}

		
		sprintf(DBmsg, "PW %s %s", inputID, inputPW);
		printf("\n\033[0;32mthe server will verify that PW is correct\n\033[0m");

		if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // 서버에 PW 보내기
		{
			printf("\033[0;31mSYS : there is a problem with sending PW process!\n\033[0m"); // 오류 4 : 제대로 PW 전송못함
			*errCode = 4;
			return false;
		}

		/*------------------------*/
		// read 함수는 수신받을 때까지 기다린다
		/*-------------------------*/

		if ((length = read(serv_socket, &PWsuccess, sizeof(int))) != sizeof(int)) // 제대로 수신받지 못함
		{
			printf("\033[0;31mSYS : there is a problem with receiving PW result process!\n\033[0m"); // 오류 5 : 제대로 PW 결과 수신못받음
			*errCode = 5;
			return false; // 바로 꺼버리기
		}

		if (!PWsuccess)
			printf("\033[0;31m");
			printf("SYS : Invalid PW, Try again\n");
			printf("\033[0m");
	}
	
	printf("\033[;32mPW is correct!\nmoving to chatting screen...\n\033[0m");

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

	// ID 입력
	while (!IDsuccess)
	{
		memset(inputID, 0, IDSIZE);
		//system("clear");

		printf("the number of characters is limited to 20 characters\n");
		printf("'q' is quit this sequence\ninput your Penguin's ID : ");

		fgets(inputID, IDSIZE - 1, stdin);	// 20자까지만 입력받음
		inputID[strcspn(inputID, "\n")] = 0;
		
		if (strcmp(inputID, "q") == 0 || strcmp(inputID, "Q") == 0)
			// 오류 1 : 사용자가 포기하고 꺼버리는 경우
		{
			printf("\n\033[0;31mSYS : quitting this sequence...\n\033[0m");
			*errCode = 1;
			return false;
		}

		if(!follow_rules(inputID))      // 규칙 : 7자이상 && 띄어쓰기 금지
			continue;

		printf("\nplease double check if your ID is correct [%s]\n", inputID);	// 21자이상은 안들어 가기에 재확인
		printf("is that right? : [y/n] : ");
		scanf("%c", &ans);
		while (getchar() != '\n');

		if (ans == 'y' || ans == 'Y') // 사용자의 id가 결정됨
		{
			printf("\033[0;32m");
			printf("your ID has been determined\n");
			printf("the server will see if it is a unique id...\n");
			printf("\033[0m");
		}
		else // 처음부터 다시
		{
			printf("\033[0;31mSYS : try again!\n\033[0m");
			continue;
		}

		memset(DBmsg, 0, DBPKSIZE);
		sprintf(DBmsg, "id %s", inputID);

		if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // 서버에 ID 보내기
		{
			printf("\033[0;31mSYS : there is a problem with sending ID process!\n\033[0m"); // 오류 6 : 제대로 ID 전송못함
			*errCode = 6;
			return false;
		}

		if ((length = read(serv_socket, &IDsuccess, sizeof(int))) != sizeof(int))
		{
			printf("\033[0;31mSYS : there is a problem with receiving result process!\n\033[0m"); // 오류 7 : ID가 고유한지 제대로 수신받지 못함
			*errCode = 7;
			return false;
		}

		if (!IDsuccess)
			printf("\033[0;31mSYS : ID already exists, try again\n\033[0m");

		// 여기서 id가 서버에 입력되는 것이 아닌, pw까지 입력받고, 한번에 합쳐서 서버에 전송된다.
		// 현재는 그냥 id가 고유한지 확인할 뿐이다.
	}

	// PW 입력

	while (!PWsuccess)
	{
		//system("clear");
		memset(inputPW, 0, IDSIZE);
		memset(inputPW2, 0, IDSIZE);

		printf("the number of characters is limited to 20 characters\n");
		printf("'q' is quit this sequence\n");
		printf("input your Penguin accounts[%s]'s PW : ", inputID);

		fgets(inputPW, IDSIZE - 1, stdin);	// 20자까지만 입력받음
		inputPW[strcspn(inputPW, "\n")] = 0;

		if (strcmp(inputPW, "q") == 0 || strcmp(inputPW, "Q") == 0)
			// 오류 1 : 사용자가 포기하고 꺼버리는 경우
		{
			printf("\n\033[0;31mSYS : quitting this sequence...\n\033[0m");
			*errCode = 1;
			return false;
		}

		if(!follow_rules(inputPW))      // 규칙 : 7자이상 && 띄어쓰기 금지
             		continue;

		printf("please input your PW again : "); // pw 다시 입력받기
		fgets(inputPW2, IDSIZE - 1, stdin);
		inputPW2[strcspn(inputPW2, "\n")] = 0;
		
		if(!follow_rules(inputPW2))      // 규칙 : 7자이상 && 띄어쓰기 금지
            		continue;

		if (strcmp(inputPW, inputPW2)) // 두개가 같은지 확인
			printf("\n\033[0;31mSYS : your passwords do not match, try again\n\033[0m");
		else
		{
			printf("\n\033[0;32myour passwords match\n\033[0m");
			PWsuccess = 1;
		}
	}

	// 중간에 사용자가 꺼버릴수도 있기 때문에, ID와 비번을 합쳐서 서버에 전송
	printf("\033[0;32msending the account information to the server...\n\033[0m");

	memset(DBmsg, 0, DBPKSIZE);
	sprintf(DBmsg, "new %s %s", inputID, inputPW);

	if (write(serv_socket, DBmsg, strlen(DBmsg) + 1) < 0) // 서버에 계정정보 보내기
	{
		printf("\033[0;31mSYS : there was a problem sending account information\n\033[0m"); // 오류 8 : 제대로 계정정보 전송못함
		*errCode = 8;
		return false;
	}

	if ((length = read(serv_socket, &Savesuccess, sizeof(int))) != sizeof(int))
	{
		printf("\033[0;31mthe server encountered a problem saving the account\033[0m\n"); // 서버에서 제대로 저장을 못했을 때,
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
	else // 회원가입완료
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
    // 1. 글자수가 7이상일것
    if(strlen(input) < 7)
    {
        printf("\033[0;31mSYS : please enter more than 7 characters, try again!\n\033[0m");
        return false;
    }

    // 2. 입력 문자열에 띄어쓰기가 있는가?
    for(int i=0; i<strlen(input); i++)
    {
        if(input[i] == ' ')
        {
            printf("\033[0;31mSYS : when entering, do not use space, try again!\n\033[0m");
            return false;
        }
    }

    return true;
}
