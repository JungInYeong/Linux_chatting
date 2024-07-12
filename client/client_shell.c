#include "clnt.h"

bool session_down = false;
bool user_down = false;
pthread_mutex_t mtx;

int main(int argsNum, char *args[])
{
    // �ϴ� ���� �������� DB ������ ���� ��Ÿ� �����ϱ�

    /*-------------------------------------*/
    // 1. ��ſ� �ʿ��� ������ ���� +  ip�˻�
    /*-------------------------------------*/

    int serv_socket;
    struct sockaddr_in serv_addr;
    char serv_ip[16] = {0};
    bool login_success = false;
    int DB_Err = 0;
    int choose;
    char userID[IDSIZE];
    pthread_t recv_thread;
    pthread_t send_thread;
    Args send_args = { 0, NULL, {0}}; // ������ �����Լ��� �ѱ� �Ķ����
    Args recv_args = { 0, NULL, {0}};

    // ������ ip�� ����� �ԷµǾ����� �˻�
    if(argsNum < 2) // ���� �����Լ��� �Ķ���Ͱ� ���Դ��� �˻�
    {
        printf("Input server IP address correctly!\n");
        return 0;
    }
    else
    {
        strncpy(serv_ip, args[1], IDSIZE-1);
        serv_ip[sizeof(serv_ip) - 1] = '\0';

        if(!is_correctIP(serv_ip, strlen(serv_ip)))
        {
                printf("Input server IP address correctly!\n");
                return 0;
        }
        else
                printf("start communication with the appropriate ip you entered : %s\n",serv_ip);
    }

    /*-------------------------------------*/
    // 2. ������ ����� ���ϻ���
    /*-------------------------------------*/
return_menu:

    // �ּұ���ü �ʱ�ȭ
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);
    serv_addr.sin_port = htons(PORT);

    printf("socket is creating for server...\n");

    serv_socket = socket(PF_INET, SOCK_STREAM, 0); // TCP��
    if (serv_socket == -1)
    {
        printf("socket creation error!\n");
        return 0;
    }

    /*-------------------------------------*/
    // 3. connect ����
    /*-------------------------------------*/

    printf("socket is trying to connect with server...\n");

    if (connect(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("connection error!\n");
        return 0;
    }
    else
        printf("connecting success!\n");

    /*-------------------------------------*/
    // 4. ���θ޴� ȭ�� ����
    /*-------------------------------------*/

    memset(userID, 0, IDSIZE);

    while (!login_success)
    {
        //system("clear"); // ȭ�� ���� �����

        printf("-----Penguin version 1.0----\n");
        printf("1. login\n");
        printf("2. sign up\n");
        printf("3. quit\n");
        printf("----------------------------\n");
        printf("input : _\b");

        scanf("%d", &choose);        // �Է¹ޱ�
        while (getchar() != '\n');   // ���ͺ���
        putchar('\n');

        switch (choose)
        {
        case 1:
            login_success = login_process(serv_socket, &DB_Err, userID);     // id�� pw�� �� �¹޾Ƽ� ������ �ѱ�� -> ��ȿ�ϸ� success = 1
            break;

        case 2:
            login_success = signup_process(serv_socket, &DB_Err, userID);    // id�� pw�� �� �¹޾Ƽ� ������ �Ѱܼ� ��ȿ���� Ȯ��
            break;

        case 3:
            printf("quit sequence\n");
            //clean_process();                                     /* --- Ȥ�� ���� ������� ���� + ������μ��� ���� --- */
            return 0;
            break;

        default:
            printf("incorrect input, try again\n");
            break;
        }

        printf("DB errorCode : %d\n", DB_Err);
    }

    /*-------------------------------------*/
    // 5. ��ſ� �ʿ��� ������ + �Ķ���� ����ü + ���ؽ� �����
    /*-------------------------------------*/

    pthread_mutex_init(&mtx, NULL); // ���ķ� ������ ���涧���� ������ ���ؽ� �ı��Լ�ȣ��   

    //system("clear");
    static int commuErr = 0;  // ���θ޴��� ���ƿ��� �������� �� ����Ǵ°� ���� -> ������ commu.c���� ���ؽ��� ��ȣ��

    send_args.serv_socket = serv_socket; // �Ķ���� ����ü �ʱ�ȭ
    recv_args.serv_socket = serv_socket;
    send_args.errCode = &commuErr;
    recv_args.errCode = &commuErr;
    strcpy(send_args.userID, userID);
    strcpy(recv_args.userID, userID);

    /*--- ���� : ������ �Լ��� ������ ���ڴ� �ϳ��ۿ� ���� ���Ѵ� ---*/

    pthread_create(&recv_thread, NULL, recv_msg, (void*)&recv_args); // ���ſ� ������        
    pthread_create(&send_thread, NULL, send_msg, (void*)&send_args); // �۽ſ� ������        
    printf("threads were made for communication right now\n");

    /*-------------------------------------*/
    // 6. ����Ȯ�� + ���ҽ� ��ȯ
    /*-------------------------------------*/

    pthread_join(send_thread, NULL);
    printf("send_thread : down\n");

    if(user_down)
    {
            pthread_cancel(recv_thread);
            printf("force to terminate recv_thread!\n");
    }

    pthread_join(recv_thread, NULL);
    printf("recv_thread : down\n");
    pthread_mutex_destroy(&mtx);

    // system("clear");
    printf("threads and mutex were terminated!\n");

    if (commuErr == 1)
    {
        login_success = 0;
        DB_Err = 0;
        goto return_menu;
    }
    else if (2 <= commuErr && commuErr <= 4)
        printf("communication errorCode : %d\n", commuErr);

    close(serv_socket); // ������ ����
    printf("good bye~!\n");
    return 0;
}