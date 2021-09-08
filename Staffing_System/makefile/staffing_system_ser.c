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


#define N 64                                                                           
#define SIZE 512                                                                           
#define PORT 8888  //端口号
#define IP "0.0.0.0"  //自动分配IP

//用户注册登录信息结构体                                                                        
typedef struct login
{
	char job_number[N];
	char password[N];
}SIGNIN; 


typedef struct                                                                           
{                                                                                        
	int newfd;
	struct sockaddr_in cin;
}__msg;


//用户信息结构体
typedef struct information
{
	char section[N];
	char job_number[N];
	char name[N];
	int  age;
	char sex[N];
	char phone_number[N];
	char address[N];
	int  salary;
}USER;

sqlite3* db = NULL;

//创建表格	
int create_table(sqlite3 *db);

//允许本地端口快速重用
int quick_reuse(int sfd);         

//绑定服务器的IP和端口
int bind_port_IP(int sfd);                    

//线程分离函数，线程退出后自动回收线程资源    
void* recv_cli_msg(void* arg);

//客户端登录
int do_login(int newfd,sqlite3 *db,SIGNIN* staff);

//管理员添加员工信息
int do_add(int newfd,sqlite3 *db,SIGNIN* staff,USER* user);

//管理员删除员工信息	
int do_delete(int newfd,sqlite3 *db);

//管理员查看员工信息
int do_examine(int newfd,sqlite3 *db);

//管理员修改员工信息			
int do_modification(int newfd,sqlite3 *db);

//普通员工查看个人信息
int do_show(int newfd,sqlite3 *db,SIGNIN* staff);

//普通员工查询同事基本信息
int do_browse(int newfd,sqlite3 *db);

//普通员工修改个人信息
int do_change(int newfd,sqlite3 *db);

//返回上一级
int do_go_back(int newfd,sqlite3 *db,SIGNIN staff);

//退出
int do_quit(int newfd,sqlite3* db,SIGNIN staff); 

int main(int argc, const char *argv[])
{
	//打开数据库
	if(sqlite3_open("./staffing_system.db",&db) != 0)
	{                                                                                   
		//打印错误信息
		printf("%d\n",sqlite3_errcode(db));//错误码
		printf("%s\n",sqlite3_errmsg(db));
	}   
	printf("员工信息数据库打开成功\n");

	//创建表格	
	create_table(db);

	//创建流式套接字TCP
	int sfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(sfd < 0)
	{                                                                
		ERR_MSG("socket");
		return -1; 
	}   

	//允许本地端口快速重用
	quick_reuse(sfd);

	//绑定服务器的IP和端口
	bind_port_IP(sfd);             

	//定义结构体cin，里面存放的是转化为网络字节序的IP、PORT等信息
	struct sockaddr_in cin;
	//struct sockaddr_in的结构中struct in_addr结构体的大小
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

		//recv_cli_msg是线程分离函数
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
	SIGNIN administrator;
	char* errmsg = NULL;
	char sql[SIZE] = "";
  	char** pres = NULL;
    int row = 0, column = 0;                                                                           
	
	char sql1[SIZE] = "create table if not exists Informaton(section char,\
					   job_number char primary key,name char,age int,sex char,\
					   phone_number char,address char,salary int)";
	char sql2[SIZE] = "create table if not exists Login(job_number char \
					   primary key,password char)";
	char sql3[SIZE] = "create table if not exists State_Table(job_number char)";


	//创建员工公共信息表
	if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);         
		return -1;
	}
	printf("员工个人信息表创建成功\n");


	//创建登录信息表
	if(sqlite3_exec(db,sql2,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);         
		return -1;
	}
	printf("员工登录信息表创建成功\n");


	//创建用户登录状态信息表
	if(sqlite3_exec(db,sql3,NULL,NULL,&errmsg) != 0)
	{
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);         
		return -1;
	}
	printf("员工在线状态表创建成功\n");

	
	//初始化管理员工号和密码
	strcpy(administrator.job_number,"2021092101");
	strcpy(administrator.password,"liuhao");
	printf("administrator'job_number = %s\n",administrator.job_number);
	printf("administrator'password = %s\n",administrator.password);

	//为了使管理员工号密码不会反复初始化
	sprintf(sql, "select * from Login where job_number  = '%s'",administrator.job_number);
	//printf("sql = %s __%d__\n", sql, __LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}
	if(row == 0)
	{
		sprintf(sql, "insert into Login values('%s', '%s')",administrator.job_number,\
				administrator.password);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			return -1;
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
		}
		printf("管理员工号和密码初始化成功\n");
	}

	if(row > 0)
	{
		printf("管理员工号和密码初始化成功\n");
	}
	return 0;
}
///}}}

//允许本地端口快速重用
///{{{
int quick_reuse(int sfd)
{                                                                    
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	return 0;
}  
///}}}

//绑定服务器的IP和端口
///{{{
int bind_port_IP(int sfd)
{
	struct sockaddr_in sin;
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);   //ifconfig查询,这里是自动分配的IP

	//绑定服务器的IP和端口
	if(bind(sfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		ERR_MSG("bind");
		return -1;
	}

	//将套接字设置为被动监听状态
	if(listen(sfd, 10) < 0)                                                                
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

	SIGNIN staff;
	USER user;

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
			do_go_back(newfd,db,staff);
			printf("客户端%d退出\n", newfd);
			break;
		}

		printf("客户端%d发送协议:buf[0] = %c __%d__\n",newfd, buf[0], __LINE__);

		switch(buf[0])
		{
		case 'L':
			//员工登录,协议为'L'
			do_login(newfd,db,&staff);
			break;

		case 'S':
			//普通员工查看个人信息,协议为'S'
			do_show(newfd,db,&staff);
			break;  

		case 'B':
			//普通员工查询同事基本信息,协议为'B'
			do_browse(newfd,db);
			break;

		case 'C':
			//普通员工修改个人信息,协议为'C'
			do_change(newfd,db);
			break;

		case 'G':
			//返回上一级,协议为'G'
			do_go_back(newfd,db,staff);
			printf("客户端退出登录,返回上一级\n");
			break;

		case 'A':
			//管理员添加员工信息,协议为'A'
			do_add(newfd,db,&staff,&user);
			break;

		case 'D':
			//管理员删除员工信息,协议为'D'
			do_delete(newfd,db);
			break;

		case 'M':
			//管理员修改员工信息,协议为'M'
			do_modification(newfd,db);
			break;

		case 'E':
			//管理员查看员工信息,协议为'E'
			do_examine(newfd,db);
			break;

		case 'Q':
			//客户端退出,协议为'Q'
			do_quit(newfd,db,staff);
			printf("客户端退出\n");
			goto END;
			break;                         
		}
	}

END:                                                                                     
	close(newfd);                                                                        
	pthread_exit(NULL);
	return 0;
}
///}}}

//用户登录
///{{{
int do_login(int newfd,sqlite3 *db,SIGNIN* staff)
{
	//判断用户名和密码是否正确
	//判断是否有客户端已经登录
	ssize_t res = 0;

	res = recv(newfd, staff, sizeof(*staff), 0); 
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
	char sql[SIZE] = ""; 
	char* errmsg = NULL;
	char** pres = NULL;
	int row = 0, column = 0;
	char buf[N] = "ERROR1";

	printf("%d__%s\n",__LINE__,staff->job_number);
	printf("%d__%s\n",__LINE__,staff->password);
	sprintf(sql, "select * from Login where job_number = '%s' and password = '%s'",staff->job_number,staff->password);
	printf("%s__%d\n",sql,__LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{   
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1; 
	}   

	bzero(sql,sizeof(sql));
	int row1 = 0, column1 = 0;
	sprintf(sql, "select * from State_Table where job_number = '%s'",staff->job_number);
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
	sprintf(sql, "insert into State_Table values('%s')",staff->job_number);

	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}
	if(strcasecmp(staff->job_number,"2021092101") == 0)
	{
		printf("欢迎系统管理员上线\n");
	}
	else
	{
		printf("客户端%d处于在线状态\n",newfd);
	}
	return 0;

}
///}}}

//管理员添加员工信息
///{{{
int do_add(int newfd,sqlite3 *db,SIGNIN* staff,USER* user)
{
	//接收员工信息结构体和工号密码结构体，分别写入对应的表中
	ssize_t res = 0;
	char sql[SIZE] = ""; 
	char* errmsg = NULL;
	char** pres = NULL;
	char buf[N] = "";

	res = recv(newfd, staff, sizeof(*staff), 0); 
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
	printf("%d__%s\n",__LINE__,staff->job_number);
	printf("%d__%s\n",__LINE__,staff->password);

	res = recv(newfd, user, sizeof(*user), 0); 
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
	printf("%d__%s\n",__LINE__,user->section);
	printf("%d__%s\n",__LINE__,user->job_number);
	printf("%d__%s\n",__LINE__,user->name);
	printf("%d__%d\n",__LINE__,user->age);
	printf("%d__%s\n",__LINE__,user->sex);
	printf("%d__%s\n",__LINE__,user->phone_number);
	printf("%d__%s\n",__LINE__,user->address);
	printf("%d__%d\n",__LINE__,user->salary);

	bzero(sql,sizeof(sql));
	sprintf(sql, "insert into Informaton values('%s', '%s','%s', %d ,'%s','%s','%s', %d)",user->section,\
			user->job_number,user->name,user->age,user->sex,user->phone_number,\
			user->address,user->salary);
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		bzero(buf,sizeof(buf));
		strcpy(buf,"add_error");
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("员工已存在\n");
		printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}
	printf("员工%s个人信息录入成功\n",user->name);

	bzero(sql,sizeof(sql));
	sprintf(sql, "insert into Login values('%s','%s')",staff->job_number,staff->password);
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}
	printf("员工%s工号密码初始化成功\n",user->name);
	
	//发送添加成功信息
	bzero(buf,sizeof(buf));
	strcpy(buf,"add_ok");
	if(send(newfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	printf("添加成功\n");

}
///}}}

//管理员删除员工信息
///{{{
int do_delete(int newfd,sqlite3 *db)
{
	//接收员工的工号，去对应的表中删除员工信息
	ssize_t res = 0;
	char sql[SIZE] = ""; 
	char* errmsg = NULL;
	char** pres = NULL;
	char buf[N] = "";
    int row = 0, column = 0;
	USER user;
    SIGNIN staff;

	//为了打印具体某个员工被删除
	USER user1;

	res = recv(newfd,&staff, sizeof(staff), 0); 
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
	strcpy(user1.job_number,user.job_number);

	sprintf(sql, "select * from Informaton where job_number  = '%s'",user.job_number);
    printf("sql = %s __%d__\n", sql, __LINE__);
    if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
    {
        printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
        return -1;
    }

    if(row == 0)
    {
        //发送信息系统中没有该员工
		bzero(buf,sizeof(buf));
		strcpy(buf,"ERROR");
        if(send(newfd, buf, sizeof(buf), 0) < 0)
        {
            ERR_MSG("send");
            return -1;
        }
        printf("系统中没有该员工，无法删除\n");

    }

	if(row > 0)
	{
		//删除信息表中员工所有信息
		bzero(sql,sizeof(sql));
		sprintf(sql, "delete from Informaton where job_number = '%s'",user.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}

		//删除该员工工号密码表中所有信息
		bzero(sql,sizeof(sql));
		sprintf(sql, "delete from Login where job_number = '%s'",staff.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}

		bzero(buf,sizeof(buf));
		strcpy(buf,"OK");
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}

		printf("工号为%s的员工信息已删除\n",user1.job_number);
		
		//销毁临时用到的结构体
		memset(&user1,0,sizeof(user1));
	}

}
///}}}

//管理员查看员工信息
///{{{
int do_examine(int newfd,sqlite3 *db)
{
	USER user;
	char sql[4096] = "";
    char** pres = NULL;                                                                
    int row = 0, column = 0, i = 0;
    char* errmsg = NULL;
	char buf[N];
	time_t t;
	struct tm* info = NULL;
	t = time(NULL);
	info = localtime(&t);

    sprintf(sql, "select * from Informaton");
    printf("sql = %s __%d__\n", sql, __LINE__);
	printf("正在查询所有员工信息\n");
    if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
    {
        printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
        return -1;
    }

	if(row == 0)
    {
		//printf("%d\n",row);打印第几行
		memset(&user,0,sizeof(user));
		strcpy(user.section,"NULL");
        //发送信息没有员工信息
        if(send(newfd,&user, sizeof(user), 0) < 0)
        {
            ERR_MSG("send");
            return -1;
        }
        printf("没有员工信息\n");
        return 0;
    }

	if(row > 0)
	{
		for(i = column ; i < (row+1)*column ; i += column)
		{
			printf("%s\n",pres[i]);
			strcpy(user.section,pres[i]);

			printf("%s\n",pres[i+1]);
			strcpy(user.job_number,pres[i+1]);
			
			printf("%s\n",pres[i+2]);
			strcpy(user.name,pres[i+2]);
			
			printf("%d\n",atoi(pres[i+3]));
			user.age = atoi(pres[i+3]);

			printf("%s\n",pres[i+4]);
			strcpy(user.sex,pres[i+4]);

			printf("%s\n",pres[i+5]);
			strcpy(user.phone_number,pres[i+5]);
			
			printf("%s\n",pres[i+6]);
			strcpy(user.address,pres[i+6]);

			printf("%d\n",atoi(pres[i+7]));
			user.salary = atoi(pres[i+7]);
			
			if(send(newfd, &user, sizeof(user), 0) < 0)
			{
					ERR_MSG("send");
					return -1;
			}
			memset(&user,0,sizeof(user));
		}
		
	}

	strcpy(user.section,"**NULL**");
	if(send(newfd,&user, sizeof(user),0) < 0)
	{
		ERR_MSG("send");
		return -1;                                                                  
	}

	printf("员工信息传输完毕\n");
	
	//获取查询员工信息时的时间
		if(info == NULL)
	{
		perror("localtime");
		return -1;
	}

	bzero(buf,sizeof(buf));
	sprintf(buf, "%d-%02d-%02d %02d-%02d-%02d\n", info->tm_year+1900,\
			info->tm_mon+1, info->tm_mday,info->tm_hour, info->tm_min, info->tm_sec);

	printf("%s\n",buf);
    if(send(newfd, buf, sizeof(buf), 0) < 0)
    {
        ERR_MSG("send");
        return -1;
    } 
	printf("查询时间发送完毕\n");
	return 0;

}
///}}}

//管理员修改员工信息
///{{{
int do_modification(int newfd,sqlite3 *db)
{
    char buf[N] = "";
	bzero(buf,sizeof(buf));
	USER user;
    SIGNIN staff;
	USER user1;
	ssize_t res = 0;
	char sql[SIZE] = ""; 
	char* errmsg = NULL;
	char** pres = NULL;
    int row = 0, column = 0;

	//判断是否有员工
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

	sprintf(sql, "select * from Informaton where job_number  = '%s'",user.job_number);
	printf("sql = %s __%d__\n", sql, __LINE__);
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
		return -1;
	}

	if(row == 0)
	{
		//发送信息系统中没有该员工
		bzero(buf,sizeof(buf));
		strcpy(buf,"ERROR");
		if(send(newfd, buf, sizeof(buf), 0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("系统中没有该员工，无法修改\n");

	}
	bzero(buf,sizeof(buf));
	strcpy(buf,"GO");
	if(send(newfd, buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}


	//接收数据                                                                                          
	if(recv(newfd, buf, sizeof(buf), 0) <=0)
	{   
		ERR_MSG("recv");
		return -1; 
	}


	switch(buf[0])
	{
		case '1':
			res = recv(newfd,&staff, sizeof(staff), 0); 
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
			strcpy(user1.job_number,user.job_number);
			sprintf(sql, "select * from Informaton where job_number  = '%s'",user.job_number);
			printf("sql = %s __%d__\n", sql, __LINE__);
			if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
			{
				printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
				return -1;
			}

			if(row > 0)
			{
				//修改信息表中员工所有信息
				bzero(sql,sizeof(sql));
				sprintf(sql, "delete from Informaton where job_number = '%s'",user.job_number);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
					return -1;
				}


				bzero(sql,sizeof(sql));
				sprintf(sql, "insert into Informaton values('%s', '%s','%s', %d ,'%s','%s','%s', %d)",user.section,\
						user.job_number,user.name,user.age,user.sex,user.phone_number,\
						user.address,user.salary);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					bzero(buf,sizeof(buf));
					strcpy(buf,"add_error");
					if(send(newfd, buf, sizeof(buf), 0) < 0)
					{
						ERR_MSG("send");
						return -1;
					}
					printf("员工已存在\n");
					printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
					return -1;
				}
				printf("员工%s个人信息修改成功\n",user.name);

				//修改该员工密码
				bzero(sql,sizeof(sql));
				sprintf(sql, "delete from Login where job_number = '%s'",staff.job_number);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
					return -1;
				}

				bzero(sql,sizeof(sql));
				sprintf(sql, "insert into Login values('%s','%s')",staff.job_number,staff.password);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
				{
					printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
					return -1;
				}
				printf("员工%s密码修改成功\n",user.name);


				bzero(buf,sizeof(buf));
				strcpy(buf,"OK");
				if(send(newfd, buf, sizeof(buf), 0) < 0)
				{
					ERR_MSG("send");
					return -1;
				}

				printf("工号为%s的员工信息已修改\n",user1.job_number);

				//销毁临时用到的结构体
				memset(&user1,0,sizeof(user1));
			}
			break;
		default:
			break;

	}
	return 0;
}
///}}}

//普通员工查看个人信息
///{{{
int do_show(int newfd,sqlite3 *db,SIGNIN* staff)
{
	USER user;
	char sql[SIZE] = "";
    char** pres = NULL;                                                                
    int row = 0, column = 0, i = 8;
    char* errmsg = NULL;
	char buf[N];
	time_t t;
	struct tm* info = NULL;
	t = time(NULL);
	info = localtime(&t);

	printf("%s\n",staff->job_number);
	sprintf(sql, "select * from Informaton where job_number = '%s'",staff->job_number);
    printf("sql = %s __%d__\n", sql, __LINE__);
	printf("正在查询该员工信息\n");
    if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
    {
        printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
        return -1;
    }

	if(row == 0)
    {
		//printf("%d\n",row);打印第几行
		memset(&user,0,sizeof(user));
		strcpy(user.section,"NULL");
        //发送信息没有员工信息
        if(send(newfd,&user, sizeof(user), 0) < 0)
        {
            ERR_MSG("send");
            return -1;
        }
        printf("没有员工信息\n");
        return 0;
    }

	if(row > 0)
	{
			printf("%s\n",pres[i]);
			strcpy(user.section,pres[i]);

			printf("%s\n",pres[i+1]);
			strcpy(user.job_number,pres[i+1]);
			
			printf("%s\n",pres[i+2]);
			strcpy(user.name,pres[i+2]);
			
			printf("%d\n",atoi(pres[i+3]));
			user.age = atoi(pres[i+3]);

			printf("%s\n",pres[i+4]);
			strcpy(user.sex,pres[i+4]);

			printf("%s\n",pres[i+5]);
			strcpy(user.phone_number,pres[i+5]);
			
			printf("%s\n",pres[i+6]);
			strcpy(user.address,pres[i+6]);

			printf("%d\n",atoi(pres[i+7]));
			user.salary = atoi(pres[i+7]);
		
			if(send(newfd, &user, sizeof(user), 0) < 0)
			{
					ERR_MSG("send");
					return -1;
			}

	}
	printf("工号%s的员工信息传输完毕\n",user.job_number);
	memset(&user,0,sizeof(user));
	
	bzero(buf,sizeof(buf));
	strcpy(buf,"**NULL**");
	if(send(newfd,buf, sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;                                                                  
	}

	
	//获取查询员工信息时的时间
	if(info == NULL)
	{
		perror("localtime");
		return -1;
	}

	bzero(buf,sizeof(buf));
	sprintf(buf, "%d-%02d-%02d %02d-%02d-%02d\n", info->tm_year+1900,\
			info->tm_mon+1, info->tm_mday,info->tm_hour, info->tm_min, info->tm_sec);

	printf("%s\n",buf);
    if(send(newfd, buf, sizeof(buf), 0) < 0)
    {
        ERR_MSG("send");
        return -1;
    } 
	printf("查询时间发送完毕\n");


}
///}}}

//普通员工查询同事基本信息
///{{{
int do_browse(int newfd,sqlite3 *db)
{
	USER user;
	ssize_t res = 0;
	char sql[SIZE] = "";
    char** pres = NULL;                                                                
    int row = 0, column = 0, i = 8;
    char* errmsg = NULL;
	char buf[N];
	time_t t;
	struct tm* info = NULL;
	t = time(NULL);
	info = localtime(&t);

	//接收数据
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

	printf("%s\n",user.job_number);
	sprintf(sql, "select * from Informaton where job_number = '%s'",user.job_number);
    printf("sql = %s __%d__\n", sql, __LINE__);
	printf("正在查询该同事基本信息\n");
    if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != 0)
    {
        printf("__%d__ %s sqlite3_get_table:%s\n", __LINE__, __func__, errmsg);
        return -1;
    }

	if(row == 0)
    {
		//printf("%d\n",row);打印第几行
		memset(&user,0,sizeof(user));
		strcpy(user.section,"NULL");
        //发送信息没有员工信息
        if(send(newfd,&user, sizeof(user), 0) < 0)
        {
            ERR_MSG("send");
            return -1;
        }
        printf("没有员工信息\n");
        return 0;
    }
	if(row > 0)
	{
			printf("%s\n",pres[i]);
			strcpy(user.section,pres[i]);

			printf("%s\n",pres[i+1]);
			strcpy(user.job_number,pres[i+1]);
			
			printf("%s\n",pres[i+2]);
			strcpy(user.name,pres[i+2]);
			
			printf("%d\n",atoi(pres[i+3]));
			user.age = atoi(pres[i+3]);

			printf("%s\n",pres[i+4]);
			strcpy(user.sex,pres[i+4]);

			printf("%s\n",pres[i+5]);
			strcpy(user.phone_number,pres[i+5]);
			
			printf("%s\n",pres[i+6]);
			strcpy(user.address,pres[i+6]);

			printf("%d\n",atoi(pres[i+7]));
			user.salary = atoi(pres[i+7]);
		
			if(send(newfd, &user, sizeof(user), 0) < 0)
			{
					ERR_MSG("send");
					return -1;
			}

	}
	printf("工号%s的员工信息传输完毕\n",user.job_number);
	memset(&user,0,sizeof(user));
	
	bzero(buf,sizeof(buf));
	strcpy(buf,"**NULL**");
	if(send(newfd,buf, sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;                                                                  
	}

	//获取查询员工信息时的时间
	if(info == NULL)
	{
		perror("localtime");
		return -1;
	}

	bzero(buf,sizeof(buf));
	sprintf(buf, "%d-%02d-%02d %02d-%02d-%02d\n", info->tm_year+1900,\
			info->tm_mon+1, info->tm_mday,info->tm_hour, info->tm_min, info->tm_sec);

	printf("%s\n",buf);
    if(send(newfd, buf, sizeof(buf), 0) < 0)
    {
        ERR_MSG("send");
        return -1;
    } 
	printf("查询时间发送完毕\n");
}
///}}}

//普通员工修改个人信息
///{{{
int do_change(int newfd,sqlite3 *db)
{
	char buf[N] = "";
	char buf1[N] = "";
	bzero(buf,sizeof(buf));
	USER user;
    SIGNIN staff;
	ssize_t res = 0;
	char sql[SIZE] = ""; 
	char* errmsg = NULL;
	char** pres = NULL;
    int row = 0, column = 0;
	res = recv(newfd, &staff, sizeof(staff), 0); 
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
	printf("%s\n",staff.job_number);
	strcpy(user.job_number,staff.job_number);
	


	//接收数据                                                                                          
	if(recv(newfd, buf, sizeof(buf), 0) <=0)
	{   
		ERR_MSG("recv");
		return -1; 
	}
	switch(buf[0])
	{
	case '1':
		bzero(buf,sizeof(buf));
		res = recv(newfd, buf, sizeof(buf), 0); 
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
		bzero(sql,sizeof(sql));
		sprintf(sql, "update Informaton set name ='%s' where job_number ='%s'",buf,user.job_number);
		printf("%s\n",buf);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK1");
		if(send(newfd, buf, sizeof(buf), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("员工%s姓名修改成功\n",user.job_number);         
		break;

	case '2':
		bzero(buf,sizeof(buf));
		res = recv(newfd, buf, sizeof(buf), 0); 
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
		bzero(sql,sizeof(sql));
		sprintf(sql, "update Informaton set age ='%d' where job_number ='%s'",atoi(buf),user.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK2");
		if(send(newfd, buf, sizeof(buf), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("员工%s年龄修改成功\n",user.job_number);         
		break;

	case '3':
		bzero(buf,sizeof(buf));
		res = recv(newfd, buf, sizeof(buf), 0); 
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
		bzero(sql,sizeof(sql));
		sprintf(sql, "update Informaton set phone_number ='%s' where job_number ='%s'",buf,user.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK3");
		if(send(newfd, buf, sizeof(buf), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("员工%s电话号码修改成功\n",user.job_number);         

		break;

	case '4':
		bzero(buf,sizeof(buf));
		res = recv(newfd, buf, sizeof(buf), 0); 
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
		bzero(sql,sizeof(sql));
		sprintf(sql, "update Informaton set address ='%s' where job_number ='%s'",buf,user.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK4");
		if(send(newfd, buf, sizeof(buf), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("员工%s地址修改成功\n",user.job_number);         

		break;

	case '5':
		bzero(buf,sizeof(buf));
		res = recv(newfd, buf, sizeof(buf), 0); 
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
		bzero(sql,sizeof(sql));
		sprintf(sql, "update Login set password ='%s' where job_number ='%s'",buf,user.job_number);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
		{
			printf("__%d__ %s sqlite3_exec:%s\n", __LINE__, __func__, errmsg);
			return -1;
		}
		strcpy(buf1,buf);
		bzero(buf,sizeof(buf));
		strcpy(buf,"OK5");
		if(send(newfd, buf, sizeof(buf), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("员工%s密码修改成功\n",user.job_number);         
		if(send(newfd, buf1, sizeof(buf1), 0) < 0) 
		{   
			ERR_MSG("send");
			return -1;  
		}
        printf("新密码为:%s\n",buf1);         


		break;

	default:
		break;
	}

}
///}}}

//返回上一级
///{{{
int do_go_back(int newfd,sqlite3 *db,SIGNIN staff)
{
	//删除状态栏中退出系统的工号
    ssize_t res = 0;
    char* errmsg = NULL;
    char sql[SIZE] = ""; 

    res = recv(newfd, &staff, sizeof(staff), 0);                                                           
    if(res < 0)
    {   
        ERR_MSG("recv");
        return -1; 
    }   
    else if(0 == res)
    {   
    printf("%s\n",staff.job_number);
    sprintf(sql,"delete from State_Table where job_number = '%s'",staff.job_number);
    printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{   
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1; 
	}   
	printf("即将返回到上一级\n");
	if(strcasecmp(staff.job_number,"2021092101") == 0)
	{
		printf("管理员已下线\n");
	}
	else
	{
		printf("客户端%d已下线\n",newfd);
	}

	return 0;
	}
	
	printf("%s\n",staff.job_number);
    sprintf(sql,"delete from State_Table where job_number = '%s'",staff.job_number);
    printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{   
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1; 
	}
	printf("即将返回到上一级\n");
	if(strcasecmp(staff.job_number,"2021092101") == 0)
	{
		printf("管理员已下线\n");
	}
	else
	{
		printf("客户端%d已下线\n",newfd);
	}
	return 0;
}
///}}}

//退出
///{{{
int do_quit(int newfd,sqlite3* db,SIGNIN staff)   
{
    //删除状态栏中退出系统的工号
    ssize_t res = 0;
    char* errmsg = NULL;
    char sql[SIZE] = ""; 

    res = recv(newfd, &staff, sizeof(staff), 0);                                                           
    if(res < 0)
    {   
        ERR_MSG("recv");
        return -1; 
    }   
    else if(0 == res)
    {   
    printf("%s\n",staff.job_number);
    sprintf(sql,"delete from State_Table where job_number = '%s'",staff.job_number);
    printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{   
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1; 
	}   
	if(strcasecmp(staff.job_number,"2021092101") == 0)
	{
		printf("管理员退出\n");
	}
	else
	{
		printf("客户端%d退出\n",newfd);
	}

	return 0;
	}
	
	printf("%s\n",staff.job_number);
    sprintf(sql,"delete from State_Table where job_number = '%s'",staff.job_number);
    printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
	{   
		printf("__%d__%s_sqlite3_exec:%s\n",__LINE__,__func__,errmsg);
		return -1; 
	}
	if(strcasecmp(staff.job_number,"2021092101") == 0)
	{
		printf("管理员退出\n");
	}
	else
	{
		printf("客户端%d退出\n",newfd);
	}
	return 0;

}
///}}}
