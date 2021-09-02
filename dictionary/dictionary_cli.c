#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include<unistd.h>
#include<stdlib.h>

#define ERR_MSG(msg) do{\
	printf("__%d__ %s\n", __LINE__, __func__);\
	perror(msg);\
}while(0)

#define N 128
#define PORT 8888
#define IP "0.0.0.0"

//用户注册信息的结构体           
typedef struct imformation
{
	char username[N];
	char password[N];
}USER;

//用户登录信息的结构体
typedef struct imformation1
{
	char username1[N];
	char password1[N];
}USER1;

USER1 user2;
int connet_ser(int sfd);
int user_registration_and_login(int sfd);
int do_register(int sfd);
int do_login(int sfd);
int do_quit(int sfd);
int do_find(int sfd);
int do_go_back(int sfd);
int do_history(int sfd);

int main(int argc, const char *argv[])
{
	//创建套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//绑定(非必须)

	//连接服务器
	connet_ser(sfd);

	//注册登录
	user_registration_and_login(sfd);

	//关闭套接字
	close(sfd);    
	return 0;
}

//连接服务器
///{{{
int connet_ser(int sfd)
{
	struct sockaddr_in sin;
	sin.sin_family 		= AF_INET;
	sin.sin_port 		= htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);

	if(connect(sfd, (struct sockaddr*)&sin, sizeof(sin))<0)
	{
		ERR_MSG("connect");
		return -1;
	}

	printf("连接服务器成功\n");
}
///}}}

//用户注册登录
///{{{
int user_registration_and_login(int sfd)
{
	char buf[N] = "";
	ssize_t res = 0;
	char choose = 0;
	char choose1 = 0;
	int ret = 0;
BACK:	while(1)
	{
		system("clear");
		printf("--------------------------------\n");
		printf("--------------------------------\n");
		printf("--------欢迎使用电子辞典--------\n");
		printf("--------------------------------\n");
		printf("------------1.注册--------------\n");
		printf("------------2.登录--------------\n");
		printf("------------3.退出--------------\n");
		printf("--------------------------------\n");
		printf("请输入>>>");
		scanf("%c", &choose);
		while(getchar()!=10);

		switch(choose)
		{
		case '1':
			//注册,发送协议'R'
			do_register(sfd);
			break;
		case '2':
			//登录,发送协议'L'
			ret = do_login(sfd);
			if(ret == 2)
			{
				goto NEXT;
			}
			break;
		case '3':
			//退出,发送协议'Q'
			do_quit(sfd);
			goto END;
			break;
		default:
			printf("输入错误,请重新输入\n");
		}

		printf("输入任意字符,清屏>>>");
		while(getchar()!=10);
	}

NEXT:
	while(1)
	{
		system("clear");
		printf("------------------------------\n");
		printf("------------------------------\n");
		printf("----------1.查询--------------\n");
		printf("----------2.历史记录----------\n");
		printf("----------3.返回上一级--------\n");
		printf("------------------------------\n");
		printf("请输入>>>");
		scanf("%c", &choose1);
		while(getchar()!=10);

		switch(choose1)
		{
		case '1':
			//查询,发送协议'F'
			do_find(sfd);
			break;
		case '2':
			//历史记录,发送协议'H'
			 do_history(sfd);
			break;
		case '3':
			//返回上一级,发送协议'G'
			do_go_back(sfd);
				goto BACK;
			break;
		default:
			printf("输入错误,请重新输入\n");
		}
	
		printf("输入任意字符,清屏>>>");
		while(getchar()!=10);
	}

END:
	close(sfd);
	return 0;
}
///}}}

//注册
///{{{
int do_register(int sfd)
{
	//向服务器发送注册协议'L',用户开始注册
	//服务器判断用户是否存在，如果存在，给客户端提示存在
	//如果不存在，就注册成功
	char buf[N] = "R";

	//发送协议
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	USER user;
	//开始注册
	printf("请输入用户名>>>");
	scanf("%s",user.username);
	while(getchar() != 10);
	printf("请输入密码>>>");
	scanf("%s",user.password);
	while(getchar() != 10);

	//发送数据
	if(send(sfd, &user, sizeof(user), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//接收服务器返回的信息
	bzero(buf, sizeof(buf));
	if(recv(sfd, buf, sizeof(buf), 0) <=0)
	{
		ERR_MSG("recv");
		return -1;
	} 

	if(strcasecmp(buf,"ERROR") == 0)
	{
		printf("该用户已被注册\n");
		return 0;
	}

	if(strcasecmp(buf,"OK") == 0)
	{  
		printf("用户注册成功\n");
		return 0;
	}
}
///}}}

//登录
///{{{
int do_login(int sfd)
{
	//向服务器发送登录协议
	//如果用户名和密码出现错误则无法登录，别的客户段已登录也无法登录
	//成功登录则进入新的界面
	char buf[N] = "L";
	USER1 user1;

	//发送协议
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//开始登陆
	printf("用户名>>>");
	scanf("%s",user1.username1);
	while(getchar() != 10);
	printf("密码>>>");
	scanf("%s",user1.password1);
	while(getchar() != 10);
	user2 = user1;

	//发送数据
	if(send(sfd, &user1, sizeof(user1), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	bzero(buf, sizeof(buf));
	if(recv(sfd, buf, sizeof(buf), 0) <=0)
	{
		ERR_MSG("recv");
		return -1;
	}

	if(strcasecmp(buf,"ERROR1") == 0) 
	{
		printf("登录失败,用户已登录\n");
		return 0;
	}

	if(strcasecmp(buf,"ERROR2") == 0)
	{
		printf("登录失败，请检查用户名和密码是否正确\n");
		return 0;
	}

	if(strcasecmp(buf,"OK") == 0)
	{  
		printf("登录成功\n");
		return 2;
	}

	return 0;
}
///}}}

//退出
///{{{
int do_quit(int sfd)
{
	char buf[N] = "Q";
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	//发送数据
	if(send(sfd, &user2, sizeof(user2), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	return 0;
}
///}}}

//查询
///{{{
int do_find(int sfd)
{
    //向服务器发送查询协议'F'
	//如果用户名和密码出现错误则无法登录，别的客户段已登录也无法登录
	//成功登录则进入新的界面
	char buf[N] = "F";

	//发送协议
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//发送数据
	if(send(sfd, &user2, sizeof(user2), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}


	char buf1[N] = "";
	bzero(buf, sizeof(buf));
    printf("请输入要查询的单词>>>");
	scanf("%s", buf1);
	while(getchar() != 10);

	//发送数据
	if(send(sfd, &buf1, sizeof(buf1), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//接收服务器返回的信息
	bzero(buf, sizeof(buf));
	if(recv(sfd, buf, sizeof(buf), 0) <=0)
	{
		ERR_MSG("recv");
		return -1;
	}

	if(strcasecmp(buf,"ERROR") == 0)
	{
		printf("词典中没有该词汇\n");
		return 0;
	}
	else
	{
	printf("查询成功\n");
	printf("%s   %s\n",buf1,buf);
	return 0;
	}
    return 0;
}
///}}}

//历史记录
///{{{
int do_history(int sfd)
{
	//向服务器发送查询历史记录协议'H'
	
	char buf[N] = "H";

	//发送协议
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//发送数据
	if(send(sfd, &user2, sizeof(user2), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//接收服务器返回的信息
	while(1)
	{	
		bzero(buf, sizeof(buf));
		if(recv(sfd, buf, sizeof(buf), 0) <=0)
		{
			ERR_MSG("recv");
			return -1;
		}

		if(strcasecmp(buf,"**NO**") == 0)
		{
			printf("目前没有历史记录\n");
			break; 
		}


		if(strcasecmp(buf,"**NULL**") == 0)
		{
			break;
		}
		printf("%s",buf);

	}
	return 0;
}
///}}}

//返回上一级
///{{{
int do_go_back(int sfd)
{

	//向服务器发送查询历史记录协议'G'
	char buf[N] = "G";

	//发送协议
	if(send(sfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	//发送数据
	if(send(sfd, &user2, sizeof(user2), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	return 0;
}
///}}}
