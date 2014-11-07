/*****

  It Is Client!!!!!!

  *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>

#define max(a,b)    ((a) > (b) ? (a) : (b))
#define MAX 100
#define SA struct sockaddr
#define BUF_SIZE 1024

void Load(int sockfd)//登录系统
{
	int n;
	char name[MAX], passwd[MAX], buf1[MAX], buf2[MAX];

	sprintf(buf1, "~");
	write(sockfd, buf1, strlen(buf1));
	printf("Put in your name:");
	fgets(name, MAX, stdin);
	name[strlen(name)-1] = '\0';
	printf("Put in your passwd:");
	fgets(passwd, MAX, stdin);
	passwd[strlen(passwd)-1] = '\0';
	
	//send the name and password to server
	write(sockfd, name, strlen(name));
	write(sockfd, passwd, strlen(passwd));

	bzero(buf2,sizeof(buf2));
	if((n = read(sockfd, buf2, MAX)) < 0)
	{
		printf("read error\n");
		exit(1);
	}
	//buf2[strlen(buf2)-1] = '\0';
	//printf("~~~~~%s\n", buf2);
	if(strcmp(buf2, name) == 0) 
	{
		printf("The name is error\n");
	}
	else if(strcmp(buf2, passwd) == 0)
	{
		printf("The passwd is error\n");
	}
	else if(strcmp(buf2, "~") == 0)
	{
		printf("Loading success\n");
		return;
	}
	bzero(passwd, sizeof(passwd));
	exit(1);
}

void Send(int sockfd)
{
	char filename[MAX];
	int fd;
	ssize_t nbyte;
	char buffer[BUF_SIZE], buf1[MAX], buf2[MAX];
	
	//initialize
	bzero(buf1, sizeof(buf1));
	bzero(buf2, sizeof(buf2));
	bzero(filename, sizeof(filename));
	bzero(buffer, sizeof(buffer));

	//***********一套文件传输系统*************
	read(sockfd, buf1, MAX);
	fputs(buf1, stdout);
	
	fgets(filename, MAX, stdin);
	filename[strlen(filename)-1] = '\0';
	write(sockfd, filename, strlen(filename));
	
	read(sockfd, buf1, MAX);
	fputs(buf1, stdout);

	if((fd = open(filename,O_RDONLY)) == -1)
	{
		printf("open error\n");
		exit(1);
	}

	while(1)
	{
		if((nbyte = read(fd, buffer, BUF_SIZE)) <= 0)
		{
			sprintf(buffer,"@");
			send(sockfd, buffer, strlen(buffer), 0);
			bzero(buffer, sizeof(buffer));
			break;
		}
		send(sockfd, buffer, strlen(buffer), 0);
		bzero(buffer,sizeof(buffer));
	}
	printf("Send ever\n");
	//************一套文件传输系统*************
}

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1;
	fd_set rset;
	char sendline[MAX], recvline[MAX], buf[MAX];
	int n;

	FD_ZERO(&rset);
	for(;;)
	{
		FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;//??
		if(select(maxfdp1, &rset, NULL, NULL, NULL) == -1)
		{
			printf("select error\n");
			exit(1);
		}
		if(FD_ISSET(sockfd, &rset))//接收服务器的消息
		{
			if((n = read(sockfd, buf, MAX)) > 0)
			{
				//printf("server terminated prematurely\n");
				//exit(1);
				buf[n] = '\0';
				printf("The server say :");
				fputs(buf, stdout);
				bzero(buf,sizeof(buf));
			}
		//	fputs(recvline, stdout);
		}
		if(FD_ISSET(fileno(fp), &rset))//给服务器发送消息
		{
			if(fgets(sendline, MAX, fp) == NULL)
				return;
			sendline[strlen(sendline)] = '\0';
			write(sockfd, sendline, strlen(sendline));
			if(strcmp(sendline,"@\n") == 0)//send the file to server
			{
				//printf("~~\n");
				Send(sockfd);
			}
			else
			{
				printf("You say :%s", sendline);
			}
			//bzero(&sockfd, sizeof(sockfd));
			bzero(sendline, sizeof(sendline));
		}
	}
}

int main()
{
	int sockfd, port;
	char ip[MAX], buf[MAX];
	struct sockaddr_in servaddr;

	printf("Please put in the ip:");
	fgets(ip,MAX,stdin);
	printf("Please put in the port:");
	scanf("%d",&port);
	getchar();

	//**************Socket 固定操作*************************
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)//socket检测
	{
		printf("Can't create socket\n");
		exit(1);
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if(inet_aton(ip,&servaddr.sin_addr) == -1)
	{
		printf("addr convert error\n");
		exit(1);
	}

	if(connect(sockfd,(SA *)&servaddr,sizeof(SA)) == -1)
	{
		printf("Connect error\n");
		exit(1);
	}
	//*******************************************************
	Load(sockfd);//登录系统

	printf("Connect success!\nPlease put in your sentences\nPut '@' to send file\n");
	
	//fgets(buf,MAX,stdin);
	//write(sockfd,buf,strlen(buf));
		
		str_cli(stdin, sockfd);
	
	close(sockfd);
	printf("Good bye!\n");
	return 0;
}
