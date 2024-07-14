CC = gcc 
# 리눅스 컴파일러

# 만들 타겟

clnt_out = client.out 
# 만들어야 하는 타겟 리눅스용

OBJS = clnt_shell2.o clnt_commu.o clnt_socket.o
# 컴파일에 필요한 obj 파일

CFLAGS = -lpthread

# 실행파일 만들기
$(clnt_out) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@
	rm *.o

# obj 파일만들기
clnt_shell.o : clnt_shell2.c
	$(CC) $(CFLAGS) -c clnt_shell2.c

clnt_commu.o : clnt_commu.c
	$(CC) $(CFLAGS) -c clnt_commu.c

clnt_socket.o : clnt_socket.c
	$(CC) $(CFLAGS) -c clnt_socket.c

clean_o :
	rm *.o

clean_out : 
	rm *.out

test : 
	echo makefile_OK
