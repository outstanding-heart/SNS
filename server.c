/*****

  It Is Server!!!!!!

  *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX 100
#define MAXN 20
#define PORT 8000
#define BUF_SIZE 1024
#define SA struct sockaddr
#define max(a,b)    ((a) > (b) ? (a) : (b))
#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef struct Cnode
{
	int des;
	char name[MAXN];
}Cnode;

typedef struct Clist
{
	Cnode node[FD_SETSIZE];
	int length;
}Clist;

void Load(int sockfd, Cnode *node)
{
	int fd,n;
	char name[MAX], passwd[MAX], buf_pw[MAX], buf[MAX];

	//initialize
	bzero(name, sizeof(name));
	bzero(passwd, sizeof(passwd));
	bzero(buf_pw, sizeof(buf_pw));

	read(sockfd, name, MAX);
	read(sockfd, passwd, MAX);

	if((fd = open(name, O_RDONLY)) == -1)//the name is error
	{
		write(sockfd, name, strlen(name));
		return;
	}
	else
	{
		if((n = read(fd, buf_pw, MAX)) < 0)
		{
			printf("read error\n");
			exit(1);
		}
		buf_pw[strlen(buf_pw)-1] = '\0';
		//printf("%s\n",buf_pw);
		//fgets(buf_pw, MAX, fd);
	}
	//printf("%s\n",passwd);
	if(strcmp(passwd, buf_pw) != 0)//the password is error
	{
		write(sockfd, passwd, strlen(passwd));
	}
	else
	{
		sprintf((*node).name, name);
		//strcpy((*node).name, name);
		//node.name = name;
		printf("Loading success\n");
		sprintf(buf, "~");
		buf[strlen(buf)] = '\0';
		write(sockfd, buf, strlen(buf));
	}
	return;
}

void Recv(int sockfd)
{
	ssize_t nbyte;
	int fd, n;
	char buffer[BUF_SIZE], buf1[MAX], buf2[MAX], filename[MAX];

	//initialize
	bzero(filename, sizeof(filename));
	bzero(buf1, sizeof(buf1));
	bzero(buf2, sizeof(buf2));
	bzero(buffer, sizeof(buffer));
	
	//************一套文件传输系统**************
	sprintf(buf1,"Please send the filename\n");
	write(sockfd, buf1, MAX);
	if((n = read(sockfd, filename, MAX)) < 0)
	{
		printf("read error\n");
		return;
	}
	//printf("%s\n",filename);

	//creat the new file named "filename"
	if((fd = creat(filename, FILE_MODE)) < 0)
	{
		printf("Create error\n");
		exit(1);
	}
	sprintf(buf2,"OK\n");//you can send the file
	write(sockfd, buf2, MAX);

	//start receive !
	while((nbyte = recv(sockfd, buffer, BUF_SIZE, 0)) > 0)
	{
		if(strcmp(buffer,"@") == 0) break;
		write(fd, buffer, strlen(buffer));
		//printf("sending\n");
		bzero(buffer, sizeof(buffer));
		bzero(&sockfd, sizeof(sockfd));
	}
	bzero(buffer, sizeof(buffer));
	printf("\n");
	//receive over

	//**********一套文件传输系统*****************
	close(fd);
	printf("Recv is over\n");
}

void sig_chld(int signo)//处理僵死进程
{
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

int main()
{
	int listenfd, connfd, i, j, maxi, maxfd, sockfd;
	int nready;
	Clist client;
	ssize_t n;
	char buf[MAX], sendline[MAX];
	FILE *fp = stdin;
	fd_set rset, allset;
	socklen_t clilen;
	pid_t childpid;
	struct sockaddr_in servaddr,cliaddr;

	//************Build the socket**************

	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1)
	{
		printf("Can't create socket\n");
		exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	/*if(inet_aton(,servaddr.sin_addr.s_addr) == -1)
	{
		printf("addr convert error\n");
		exit(1);
	}*/
	if(bind(listenfd,(SA *)&servaddr,sizeof(SA)) == -1)
	{
		printf("bind error\n");
		exit(1);
	}
	if(listen(listenfd,2) == -1)
	{
		printf("listen error\n");
		exit(1);
	}
	//********************************************

	maxfd = listenfd;
	maxi = -1;
	for(i = 0; i< FD_SETSIZE; i++)
		client.node[i].des = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	//**************************

	printf("Server is begin!\n");
	signal(SIGCHLD, sig_chld);

	for(;;)
	{
		FD_SET(fileno(fp), &allset);
		rset = allset;
		if((nready = select(maxfd+1, &rset, NULL, NULL, NULL)) == -1)
		{
			printf("First select error\n");
			//exit(1);
			continue;
		}
		if(FD_ISSET(listenfd, &rset))//监听是否有新的客户端连入
		{
			clilen = sizeof(cliaddr);
			if((connfd = accept(listenfd,(SA *)&cliaddr,&clilen)) == -1)
			{
				printf("accept error\n");
				continue;
			}
			for(i = 0; i < FD_SETSIZE; i++)
			{
				if(client.node[i].des < 0)
				{
					client.node[i].des = connfd;
					printf("The client %d is connect\n", i);
					break;
				}
			}
			if(i == FD_SETSIZE)
			{
				printf("Too many clients\n");
				exit(1);
			}
			FD_SET(connfd, &allset);

			if(connfd > maxfd)
				maxfd = connfd;
			if(i > maxi)
				maxi = i;
			if(--nready <= 0)
				continue;
		}
	
		for(i = 0;i <= maxi; i++)//监听客户端是否有消息传来
		{
			if((sockfd = client.node[i].des) < 0) continue;
			if(FD_ISSET(sockfd, &rset))
			{
				if((n = read(sockfd, buf, MAX)) == 0)
				{
					close(sockfd);
					FD_CLR(sockfd,&allset);
					client.node[i].des = -1;
					printf("The client %d is over\n",i);
				}
				else
				{
					//write(sockfd, buf, n);
					//buf[strlen(buf)-1] = '\0';
					if(strcmp(buf,"@\n") == 0)//Receive the file
					{
						if((childpid = fork()) == 0)
						//if receive a file ,fork a new process
						{
							close(listenfd);
							Recv(sockfd);
						//	bzero(buf, sizeof(buf));
							exit(0);
						}
					}
					else if(strcmp(buf,"~") == 0)	
					{
						Load(sockfd, &client.node[i]);
						printf("The %s loaded!\n", client.node[i].name);
					}
					else
					{
						//bzero(buf,sizeof(buf));
						printf("The client %d say: %s", i, buf);
					}
					bzero(buf, sizeof(buf));
					bzero(&sockfd, sizeof(sockfd));
				}
				//str_ser(sockfd, i, allset, client);
				if(--nready <= 0) break;
				//bzero(&sockfd, sizeof(sockfd));
			}
		}

		if(FD_ISSET(fileno(fp), &rset))//给指定客户端发送消息
		{
			scanf("%d", &j);
			fgets(sendline, MAX, stdin);
			sendline[strlen(sendline)] = '\0';
			write(client.node[j].des, sendline, strlen(sendline));
			printf("You say %d :%s", j, sendline);
			bzero(sendline, sizeof(sendline));
		}
	}
	printf("Server is closed\n");
	exit(1);
}
