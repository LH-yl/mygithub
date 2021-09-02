//头文件
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>                                                                             
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>


//定义错误信息打印的宏
#define ERR_MSG(msg) do{\
	printf("__%d__ %s\n", __LINE__, __func__);\
	perror(msg);\
}while(0)


#define N 128
#define PORT 8888
#define IP "0.0.0.0"


typedef struct
{                                                                                                   
	int newfd;
	struct sockaddr_in cin;
}__msg;


//用户登录信息的结构体
typedef struct imformation1
{
	char username1[N];
	char password1[N];
}USER1;

//用户注册信息的结构体
typedef struct imformation
{
	char username[N];
	char password[N];
}USER;

sqlite3* db = NULL;

int create_table(sqlite3 *db);
int	import_words(sqlite3 *db);
int quick_reuse(int sfd);
void* recv_cli_msg(void* arg);
int fill_port_IP(int sfd);
int do_register(int newfd,sqlite3* db);
int do_login(int newfd,sqlite3* db,USER1 *);
int do_quit(int newfd,sqlite3* db);
int do_find(int newfd,sqlite3* db);
int do_history(int newfd,sqlite3* db);
int do_go_back(int newfd,sqlite3* db,USER1);


int main(int argc, const char *argv[])
{
	//打开数据库
	if(sqlite3_open("./dictionary.db",&db) != 0)
	{
		//打印错误信息
		printf("%d\n",sqlite3_errcode(db));//错误码
		printf("%s\n",sqlite3_errmsg(db));
	}
	printf("数据库打开成功\n");

	//创建表格
	create_table(db);

	//打开dict.txt文件,并将dict.txt文本循环导入单词表中
	import_words(db);

	//创建套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//允许本地端口快速重用
	quick_reuse(sfd);


	//绑定服务器的ip和端口
	fill_port_IP(sfd);


	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);

	//创建线程
	pthread_t tid;

	while(1)
	{
		//等待获取新的套接字                                                                        
		int newfd = accept(sfd, (struct sockaddr*)&cin, &addrlen);
		if(newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}
		printf("newfd = %d\n", newfd);
		printf("[%s:%d]已上线\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

		__msg cliInfo;
		cliInfo.newfd = newfd;
		cliInfo.cin = cin;

		if(pthread_create(&tid, NULL, recv_cli_msg, (void*)&cliInfo) != 0)
		{
			ERR_MSG("pthread_create");
			return -1;
		}
	}

	//关闭套接字
	close(sfd);  

	//关闭数据库
	sqlite3_close(db);
	return 0;

}

//创建表格
///{{{
int create_table(sqlite3 *db)
{

	char sql[N] = "create table if not exists dictionary(word char,meaning char)";
	char sql1[N] = "create table if not exists information(username char primary key,password char)";
	char sql2[N] = "create table if not exists history(username char,word char,meaning char,time char)";
	char sql3[N] = "create table if not exists state_table(username char)";
	char* errmsg = NULL;

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("单词表创建成功\n");


	if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("用户信息表创建成功\n");


	if(sqlite3_exec(db,sql2,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("历史记录表创建成功\n");

	if(sqlite3_exec(db,sql3,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("用户登录状态表创建成功\n");
}
///}}}

//打开dict.txt文件,并将dict.txt文本循环导入单词表中
///{{{
int	import_words(sqlite3 *db)
{
	FILE* fp = fopen("./dict.txt","r");
	if(NULL == fp)
	{
		ERR_MSG("open");
		return -1;
	}
	printf("dict.txt文件打开成功\n");
    printf("正在导入词典......\n");

	//使用sqlite3_get_table函数获取行数row,如果行数等于单词表行数，则不会重复导入
	char sql[N] = "select * from dictionary";   
	char** pres = NULL;
	int row, column;
	char* errmsg = NULL;

	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{   
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return -1; 
	}      

	if(7987 == row)
	{
		printf("dict.txt文件导入成功\n");
		printf("**单词表载入成功**\n");
		sqlite3_free_table(pres);		
		return 0;
	}

	//往单词表中循环导入单词
	char buf[N] = "";
	char word_buf[256] = "";
	int i = 0;
	while(1)
	{
		i = 1;
		bzero(buf,sizeof(buf));
		char * res = fgets(buf+1,sizeof(buf),fp);
		if(NULL == res)
		{
			printf("dict.txt文件导入成功\n");
			break;
		}
		while(buf[i] != ' ')
			i++;
		if(buf[2] == '\'')
		{
			buf[2] = '`';
		}

		buf[0] = '\'';
		buf[i] = '\'';
		buf[i+1] = ',';
		buf[i+2] = '\'';
		buf[strlen(buf)-1] = '\'';

		sprintf(word_buf,"insert into dictionary values(%s)",buf);

		char* errmsg = NULL;
		if(sqlite3_exec(db,word_buf,NULL,NULL,&errmsg) != 0)
		{
			printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
			return -1;
		}
	}
	printf("**单词表载入成功**\n");
	return 0;
}
///}}}

//允许本地端口快速重用
///{{{
int quick_reuse(int sfd)
{
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))<0)
	{
		ERR_MSG("setsockopt");
		return 0;
	}
}                                                                                                   
///}}}

//填充服务器的IP和端口
///{{{
int fill_port_IP(int sfd)
{
	struct sockaddr_in sin;
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);   //ifconfig查询

	if(bind(sfd, (struct sockaddr*)&sin, sizeof(sin)) <0)
	{
		ERR_MSG("bind");
		return -1;
	}

	//将套接字设置为被动监听状态
	if(listen(sfd, 5) <0)
	{
		ERR_MSG("listen");
		return -1;
	}
	printf("监听成功\n");                                                                           
	return 0;

}
///}}}

//线程分离函数，线程退出后自动回收线程资源
///{{{
void* recv_cli_msg(void* arg)
{
	//线程分离函数，线程退出后自动回收线程资源
	pthread_detach(pthread_self());

	__msg cliInfo = *(__msg*)arg;
	int newfd = cliInfo.newfd;
	struct sockaddr_in cin = cliInfo.cin;

	USER1 user2;
	char buf[N];
	ssize_t res = 0;
	while(1)
	{
		bzero(buf, sizeof(buf));
		//循环接收客户端的请求                                                                      
		res = recv(newfd, buf, sizeof(buf), 0);
		if(res < 0)
		{
			ERR_MSG("recv");
			break;
		}
		else if(0 == res)
		{
			do_go_back(newfd,db,user2);
			printf("客户端%d退出\n", newfd);
			break;
		}

		printf("客户端%d发送协议:buf[0] = %c __%d__\n",newfd, buf[0], __LINE__);

		switch(buf[0])                                                                              
		{
		case 'R':
			//注册,协议为'R'
			do_register(newfd,db);
			break;
		case 'L':
			//登录,协议为'L'
			do_login(newfd,db,&user2);
			break;
		case 'F':
			//查询,协议为'F'
			do_find(newfd,db);
			break;                                                                                  
		case 'H':
			//历史记录,协议为'H'
			do_history(newfd,db);
			break; 
		case 'Q':
			//客户端退出
			do_quit(newfd,db);
			printf("客户端退出\n");
			goto END;
			break;                                                               
		case 'G':
			//客户端退出
			do_go_back(newfd,db,user2);
			printf("客户端退出登录,返回上一级\n");
			break;
		}
	}

END:
	close(newfd);                                                                                   
	pthread_exit(NULL);
}
///}}}

//注册
///{{{
int do_register(int newfd,sqlite3* db)
{
	//接受注册信息并与用户信息表比较
	//如果用户信息已存在则发送信息给客户端账户已存在，否则注册成功
	ssize_t res = 0;
	USER user;

	res = recv(newfd, &user, sizeof(user), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}

	char sql[516] = "";
	char* errmsg = NULL;
	char** pres = NULL;
	int row, column;
	char buf[N] = "ERROR";

	sprintf(sql, "insert into information values('%s', '%s')",user.username,user.password);

	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{

		//重复注册，发送注册失败信息
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("用户重复注册\n");
		sqlite3_free_table(pres);
		return -1;
	}

	//发送注册成功信息
	bzero(buf,sizeof(buf));
	strcpy(buf,"OK");
	if(send(newfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("sql = %s __%d__\n", sql, __LINE__);
	printf("**用户信息插入成功**\n");
	return 0;

}
///}}}

//登录
///{{{
int do_login(int newfd,sqlite3 *db,USER1* user1)
{
	//判断用户名和密码是否正确
	//判断是否有客户端已经登录
	ssize_t res = 0;

	res = recv(newfd, user1, sizeof(*user1), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}
	char sql[516] = "";
	char* errmsg = NULL;
	char** pres = NULL;
	int row = 0, column = 0;
	char buf[N] = "ERROR1";

	printf("%d__%s\n",__LINE__,user1->username1);
	printf("%d__%s\n",__LINE__,user1->password1);
	sprintf(sql, "select * from information where username = '%s' and password = '%s'",user1->username1,user1->password1);
	printf("%s__%d\n",sql,__LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}

	bzero(sql,sizeof(sql));
	int row1 = 0, column1 = 0;
	sprintf(sql, "select * from state_table where username = '%s'",user1->username1);
	printf("%s__%d\n",sql,__LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row1, &column1, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}

	if(row1 == 1)
	{
		//登录失败,用户已登录
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}

		printf("登录失败,已有用户登录\n");
		return 0;
	}

	if(row != 1)
	{
		//登录失败,用户名或密码错误
		bzero(buf,sizeof(buf));
		strcpy(buf,"ERROR2");
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}

		printf("登录失败,用户名或密码错误\n");
		return 0;
	}

	if(row == 1)
	{
		//发送登录成功信息
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK");
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("登录成功\n");
	}

	bzero(sql,sizeof(sql));
	sprintf(sql, "insert into state_table values('%s')",user1->username1);

	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}
	printf("客户端%d处于在线状态\n",newfd);

	return 0;
}
///}}}

//退出
//{{{
int do_quit(int newfd,sqlite3* db)
{
	//删除状态栏中退出词典的用户名
	ssize_t res = 0;
	USER1 user2;

	res = recv(newfd, &user2, sizeof(user2), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}

	char* errmsg = NULL;
	char sql[516] = "";
	sprintf(sql,"delete from state_table where username = '%s'",user2.username1);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("客户端%d已下线\n ",newfd);
	return 0;
}
///}}}

//查询
///{{{
int do_find(int newfd,sqlite3* db)
{
	//接收要查询的单词
	//去单词表中对比查找,如果没有就给客户端发消息没找到该词汇
	//如果有就给客户端返回该单词和单词的意思
	USER1 user2;
	ssize_t res = 0;

	res = recv(newfd, &user2, sizeof(user2), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}
	printf("%s正准备查询单词__%d__\n",user2.username1,__LINE__);

	char buf[N] = "";
	res = recv(newfd, &buf, sizeof(buf), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}
	printf("客户端%d想查询的单词是%s__%d__\n",newfd,buf,__LINE__);

	char sql[516] = ""; 
	char** pres = NULL;
	int row = 0, column = 0;
	char* errmsg = NULL;
	sprintf(sql, "select * from dictionary where word  = '%s'",buf);
	printf("sql = %s __%d__\n", sql, __LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}

	if(row == 0)
	{
		//发送信息未找到该单词
		char buf1[N] = "ERROR";
		if(send(newfd, buf1, sizeof(buf1), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("未找到该单词\n");

	}
	if(row > 0)
	{
		//查询成功
		bzero(sql,sizeof(sql));
		sprintf(sql, "select meaning from dictionary where word  = '%s'",buf);
		printf("sql = %s __%d__\n", sql, __LINE__);
		if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		puts(pres[1]);
		char buf2[N] = "";
		strcpy(buf2,pres[1]);
		if(send(newfd, buf2, sizeof(buf2), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("查询成功\n");

		//获取查询单词时的时间
		char buf3[N];
		time_t t;
		struct tm* info = NULL;
		t = time(NULL);
		info = localtime(&t);
		if(info == NULL)
		{
			perror("localtime");
			return -1;
		}

		sprintf(buf3, "%d-%02d-%02d %02d-%02d-%02d\n", info->tm_year+1900, info->tm_mon+1, info->tm_mday, \
				info->tm_hour, info->tm_min, info->tm_sec);

		printf("%s\n",buf3);

		//信息保存到历史记录中去
		bzero(sql,sizeof(sql));
		sprintf(sql, "insert into history values('%s','%s','%s','%s')",user2.username1,buf,buf2,buf3);
		printf("sql = %s __%d__\n", sql, __LINE__);
		if(sqlite3_exec(db, sql, NULL,NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
	}

	return 0;
}
///}}}

//历史记录
///{{{
int do_history(int newfd,sqlite3* db)
{	
	USER1 user2;
	ssize_t res = 0;
	res = recv(newfd, &user2, sizeof(user2), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
		printf("客户端关闭\n");
		return 0;
	}
	printf("%s__%d\n",user2.username1,__LINE__);

	char sql[516] = "";
	char** pres = NULL;
	int row = 0, column = 0;
	char* errmsg = NULL;
	char buf[N] = "**NO**";
	sprintf(sql, "select word,meaning,time from history where username  = '%s'",user2.username1);
	printf("sql = %s __%d__\n", sql, __LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}

	if(row == 0)
	{
		printf("%d",row);
		//发送信息没有记录
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("该用户没有历史记录\n");
		return 0;
	}

	printf("获取newfd = %d历史记录\n",newfd);
	if(row > 0)
	{
		int i = 0;
		char buf1[1024] = "";
		bzero(buf1,sizeof(buf1));
		for(i = column ; i < (row+1)*column ; i+=column)
		{
			sprintf(buf1,"%-15s   %-40s   %-15s",pres[i],pres[i+1],pres[i+2]);
			printf("词意:  %s",buf1); 
			if(send(newfd, buf1, sizeof(buf1), 0) < 0)
			{
				ERR_MSG("send");
				return -1;
			}
		}
	}

	bzero(buf,sizeof(buf));
	strcpy(buf,"**NULL**");
	if(send(newfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("历史记录传输完毕\n");
	return 0;
}
///}}}

//返回上一级
///{{{
int do_go_back(int newfd,sqlite3* db,USER1 user2)
{
	//删除状态栏中退出词典的用户名
	ssize_t res = 0;
	USER1 user1;
	char* errmsg = NULL;
	char sql[516] = "";

	res = recv(newfd, &user1, sizeof(user1), 0);
	if(res < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	else if(0 == res)
	{
	printf("%s\n",user2.username1);
	sprintf(sql,"delete from state_table where username = '%s'",user2.username1);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("即将返回上一级\n ");
		printf("客户端关闭\n");
		return 0;
	}

	printf("%s\n",user2.username1);
	sprintf(sql,"delete from state_table where username = '%s'",user2.username1);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("即将返回上一级\n ");
	return 0;

}
///}}}

