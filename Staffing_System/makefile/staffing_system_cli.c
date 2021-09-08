//头文件
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

//定义错误信息打印的宏     
#define ERR_MSG(msg) do{\
	printf("__%d__%s\n",__LINE__,__func__);\
	perror(msg);\
}while(0)


#define N 64
#define SIZE 512
#define PORT 8888  //端口
#define IP "0.0.0.0"  //IP

//用户注册登录信息结构体
typedef struct login
{
	char job_number[N];
	char password[N];
}SIGNIN; 

SIGNIN staff_login;
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

//连接服务器
int connet_ser(int sfd);  

//用户使用界面
int user_interface(int sfd);

//用户登录
int do_login(int sfd);

//管理员添加员工信息
int do_add(int sfd);

//删除员工信息
int do_delete(int sfd);

//管理员查看员工信息
int do_examine(int sfd);

//管理员修改员工信息
int do_modification(int sfd);

//普通员工查看个人信息
int do_show(int sfd);

//普通员工查询同事基本信息
int do_browse(int sfd);

//普通修改个人信息
int do_change(int sfd);

//返回上一级
int do_go_back(int sfd);

//退出
int do_quit(int sfd);

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

	//用户使用界面
	user_interface(sfd);

	//关闭套接字
	close(sfd);   
	return 0;
}


//连接服务器
///{{{
int connet_ser(int sfd)
{
	struct sockaddr_in sin;
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);

	//连接服务器
	if(connect(sfd, (struct sockaddr*)&sin, sizeof(sin))<0)
	{
		ERR_MSG("connect");
		return -1;
	}

	printf("连接服务器成功\n");
}
///}}}

//用户界面
///{{{
int user_interface(int sfd)
{
	char buf[N] = "";
	ssize_t res = 0;
	char choice1 = 0; 
	char choice2 = 0; 
	char choice3 = 0;
	int ret = 0;
BACK: while(1)
	  {
	  system("clear");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("-----------------员工信息管理系统----------------\n");
        printf("-------------------------------------------------\n");
        printf("---------------------1.登录----------------------\n");
        printf("---------------------2.退出----------------------\n");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("请输入选项>>>");
        scanf("%c", &choice1);
        while(getchar()!=10);            
        switch(choice1)
        {
        case '1':
            //登录,发送协议'L'
            ret = do_login(sfd);
            if(ret == 1){
                goto Staff;
            }
			if(ret == 2){
				goto Administrator;
			}
            break;
        case '2':
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
	  
  
Staff:
    while(1)
    {
        system("clear");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("-------------欢迎进入员工信息管理系统------------\n");
        printf("-------------------------------------------------\n");
        printf("-----------------1.查看个人信息------------------\n");
        printf("-----------------2.查询同事基本信息--------------\n");
        printf("-----------------3.修改个人信息------------------\n");
        printf("-----------------4.返回上一级--------------------\n");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("请输入选项>>>");
        scanf("%c", &choice2);
        while(getchar()!=10);

        switch(choice2)
        {
        case '1':
            //查看个人信息,发送协议'S'
            do_show(sfd);
            break;
        case '2':
            //查询同事基本信息,发送协议'B'
            do_browse(sfd);
            break;
		case '3':
            //修改个人信息,发送协议'C'
			do_change(sfd);
            break;
        case '4':
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

Administrator:
		while(1)
    {
        system("clear");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("---------------欢迎进入管理员系统----------------\n");
        printf("-------------------------------------------------\n");
        printf("-----------------1.添加员工信息------------------\n");
        printf("-----------------2.删除员工信息------------------\n");
        printf("-----------------3.修改员工信息------------------\n");
        printf("-----------------4.查看员工信息------------------\n");
        printf("-----------------5.返回上一级--------------------\n");
        printf("-------------------------------------------------\n");
        printf("-------------------------------------------------\n");
        printf("请输入选项>>>");
        scanf("%c", &choice3);
        while(getchar()!=10);                                                                   

        switch(choice3)
        {
        case '1':
            //添加员工信息,发送协议'A'
            do_add(sfd);
            break;
        case '2':
            //删除员工信息,发送协议'D'
            do_delete(sfd);
            break;
        case '3':
            //修改员工信息,发送协议'M'
            do_modification(sfd);
            break;
		case '4':
			//查看员工信息，发送协议'E'
			do_examine(sfd);
			break;
        case '5':
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

//用户登录
///{{{
int do_login(int sfd)
{
    //向服务器发送登录协议
    //如果用户名和密码出现错误则无法登录，别的客户端已登录也无法登录
    //成功登录则进入新的界面
    char buf[N] = "L";                                                                  
    SIGNIN staff;

    //发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

    //开始登陆
    printf("工号>>>");
    scanf("%s",staff.job_number);
    while(getchar() != 10);
    printf("密码>>>");
    scanf("%s",staff.password);
    while(getchar() != 10);

	//保存登录的工号,方便返回上一级时把工号从状态栏中删除
	staff_login = staff;

    //发送数据
    if(send(sfd, &staff, sizeof(staff), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

    bzero(buf, sizeof(buf));

	//接收数据
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
        printf("登录失败，请检查员工号和密码是否正确\n");
        return 0;
    }

    if(strcasecmp(buf,"OK") == 0)
    {
		if(strcasecmp(staff.job_number,"2021092101") == 0)
		{
        	printf("登录成功\n");
			return 2;
		}
        printf("登录成功\n");
        return 1;                                                                       
    }

    return 0;
}
///}}}

//管理员添加员工信息
///{{{
int do_add(int sfd)
{
    //向服务器发送添加员工信息的协议'A'
	char buf[N] = "A";
    USER user;
	SIGNIN staff;

	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }

	//开始录入信息
	printf("部门>>>");
    scanf("%s",user.section);
    while(getchar() != 10);

    printf("工号>>>");
    scanf("%s",user.job_number);
	strcpy(staff.job_number,user.job_number);
    while(getchar() != 10);

	printf("姓名>>>");
    scanf("%s",user.name);
    while(getchar() != 10);
	
	printf("年龄>>>");
    scanf("%d",&user.age);
    while(getchar() != 10);

	printf("性别>>>");
    scanf("%s",user.sex);
    while(getchar() != 10);

	printf("电话号码>>>");
    scanf("%s",user.phone_number);
    while(getchar() != 10);

	printf("居住城市>>>");
    scanf("%s",user.address);
    while(getchar() != 10);
    
	printf("薪资>>>");
    scanf("%d",&user.salary);
    while(getchar() != 10);

	printf("初始密码>>>");
    scanf("%s",staff.password);
    while(getchar() != 10);
	
	//发送数据(员工工号密码)
    if(send(sfd, &staff, sizeof(staff), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   
	//发送数据(员工信息)
    if(send(sfd, &user, sizeof(user), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
	}

    bzero(buf, sizeof(buf));
	//接收信息
    if(recv(sfd, buf, sizeof(buf), 0) <=0)
    {   
        ERR_MSG("recv");
        return -1; 
    }

	if(strcasecmp(buf,"add_error") == 0)
    {
        printf("工号为%s的员工已存在\n",user.job_number);
        return 0;
    }

	if(strcasecmp(buf,"add_ok") == 0)
    {
        printf("员工%s信息添加成功\n",user.name);
        return 0;
    }   	
}
///}}}

//删除员工信息
///{{{
int do_delete(int sfd)
{
	//向服务器发送删除员工信息的协议'D'
	char buf[N] = "D";
	int res = 0;
    USER user;
	SIGNIN staff;

	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }

	printf("请输入要删除员工的工号>>>");
    scanf("%s",user.job_number);
	strcpy(staff.job_number,user.job_number);
    while(getchar() != 10);
	
	//发送数据
    if(send(sfd, &staff, sizeof(staff), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

	//发送数据
    if(send(sfd, &user, sizeof(user), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
	}
	
	 bzero(buf, sizeof(buf));
	//接收信息
    if(recv(sfd, buf, sizeof(buf), 0) <=0)
    {   
        ERR_MSG("recv");
        return -1; 
    }

	if(strcasecmp(buf,"ERROR") == 0)
    {
        printf("工号为%s的员工部不存在\n",user.job_number);
        return 0;
    }

	if(strcasecmp(buf,"OK") == 0)
    {
        printf("工号为%s的员工已删除\n",user.job_number);
        return 0;
    }
}
///}}}

//管理员查看员工信息
///{{{
int do_examine(int sfd)
{
	//向服务器发送查看员工信息的协议'E'
	char buf[N] = "E";
	int res = 0;
	int flag = 0;
    USER user;

	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }
	
	//显示所有员工信息
	while(1)
	{
		res = recv(sfd, (void *)&user, sizeof(user), 0); 
		if(res < 0)                                                                              
		{   
			ERR_MSG("recv");                                                                     
			return -1; 
		}   
		else if(0 == res)
		{   
			printf("服务器关闭\n");
			return 0;
		}

		if(strcasecmp(user.section,"NULL") == 0)
		{
			printf("该系统没有员工信息\n");
			return 0;
		}
	
		if(flag == 0)
		{
			printf("section      ");
			printf("job_number      ");
			printf("name         ");
			printf("age      ");
			printf("sex      ");
			printf("phone_number    ");
			printf("address       ");
			printf("salary  \n");
			flag++;		
		}
		if(strcasecmp(user.section,"**NULL**") == 0)
		{
			puts("------------------------------------------------------------------------------------------------------");
			break;
		}

		puts("------------------------------------------------------------------------------------------------------");
		printf("%-16s",user.section);
		printf("%-16s",user.job_number);
		printf("%-16s",user.name);
		printf("%-10d",user.age);
		printf("%-10s",user.sex);
		printf("%-16s",user.phone_number);
		printf("%-16s",user.address);
		printf("%-8d\n",user.salary);
		memset(&user,0,sizeof(user));
	}

	bzero(buf,sizeof(buf));
	
	//接收数据
    if(recv(sfd, buf, sizeof(buf), 0) <= 0)
    {   
        ERR_MSG("recv");
        return -1; 
    }   
	printf("已显示所有员工信息        ");
	printf("查询时间：%s",buf);
	return 0;
}


///}}}

//普通员工查看个人信息
///{{{
int do_show(int sfd)
{
	//向服务器发送查看普通员工信息'S'
	char buf[N] = "S";
    USER user;
	int res = 0;

	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }
	
	//接收数据
	res = recv(sfd, (void *)&user, sizeof(user), 0); 
	if(res < 0)                                                                              
	{   
		ERR_MSG("recv");                                                                     
		return -1; 
	}   
	else if(0 == res)
	{   
		printf("服务器关闭\n");
		return 0;
	}

	if(strcasecmp(user.section,"NULL") == 0)
	{
		printf("该系统没有员工信息\n");
		return 0;
	}

    printf("-------------------------------------------------\n");
	printf("部门:%s\n",user.section);
	printf("工号:%s\n",user.job_number);
	printf("姓名:%s\n",user.name);
	printf("年龄:%d\n",user.age);
	printf("性别:%s\n",user.sex);
	printf("电话号码:%s\n",user.phone_number);
	printf("地址:%s\n",user.address);
	printf("薪水:%d\n",user.salary);
	memset(&user,0,sizeof(user));

	//接收数据
	bzero(buf,sizeof(buf));
	res = recv(sfd, buf, sizeof(buf), 0); 
	if(res < 0)                                                                              
	{   
		ERR_MSG("recv");                                                                     
		return -1; 
	}   
	else if(0 == res)
	{   
		printf("服务器关闭\n");
		return 0;
	}

	if(strcasecmp(buf,"**NULL**") == 0)
	{
        printf("-------------------------------------------------\n");
		printf("以上是您的全部个人信息    ");
	}

	bzero(buf,sizeof(buf));
	
	//接收数据
    if(recv(sfd, buf, sizeof(buf), 0) <= 0)
    {   
        ERR_MSG("recv");
        return -1; 
    }   
	printf("查询时间：%s",buf);
	return 0;

}
///}}}

//管理员修改员工信息
///{{{
int do_modification(int sfd)
{
	//向服务器发送管理员修改员工信息的协议'M'
	char buf[N] = "M";
	int res = 0;
	char choice = 0;
    USER user;
	SIGNIN staff;
	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }
	
	printf("请输入要修改员工的工号>>>");
    scanf("%s",user.job_number);
	strcpy(staff.job_number,user.job_number);
    while(getchar() != 10);

	//发送数据,判断员工是否存在
	if(send(sfd, &user, sizeof(user), 0) < 0)
	{   
		ERR_MSG("send");
		return -1; 
	}

	//接收信息
	if(recv(sfd, buf, sizeof(buf), 0) <=0)
	{   
		ERR_MSG("recv");
		return -1; 
	}

	if(strcasecmp(buf,"ERROR") == 0)
	{
		printf("系统中没有该员工，无法修改\n");
		return 0;
	}  
	if(strcasecmp(buf,"GO") == 0)
	{
		printf("系统中有该员工，马上进入修改界面\n");
	}  
	while(1)
	{
		system("clear");
		printf("-------------------------------------------------\n");
		printf("-----------------修改员工信息--------------------\n");
		printf("-------------------------------------------------\n");
		printf("-----------------1.修改信息----------------------\n");
		printf("-----------------2.返回上一级--------------------\n");
		printf("-------------------------------------------------\n");
		printf("请输入选项>>>");
		scanf("%c", &choice);
		while(getchar()!=10);                                                                   
		
		//发送协议
		bzero(buf,sizeof(buf));
		buf[0] = choice;
		if(send(sfd, buf, sizeof(buf), 0) < 0)
		{   
			ERR_MSG("send");
			return -1; 
		}

		switch(choice)
		{
		case '1':
			//修改该员工全部信息
			printf("部门>>>");
			scanf("%s",user.section);
			while(getchar() != 10);

			printf("姓名>>>");
			scanf("%s",user.name);
			while(getchar() != 10);

			printf("年龄>>>");
			scanf("%d",&user.age);
			while(getchar() != 10);

			printf("性别>>>");
			scanf("%s",user.sex);
			while(getchar() != 10);

			printf("新的电话号码>>>");
			scanf("%s",user.phone_number);
			while(getchar() != 10);

			printf("最新地址>>>");
			scanf("%s",user.address);
			while(getchar() != 10);

			printf("最新薪资>>>");
			scanf("%d",&user.salary);
			while(getchar() != 10);

			printf("新的密码>>>");
			scanf("%s",staff.password);
			while(getchar() != 10);

			//发送数据(员工新的密码)
			if(send(sfd, &staff, sizeof(staff), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}   
			//发送数据(员工新的信息)
			if(send(sfd, &user, sizeof(user), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}

			bzero(buf, sizeof(buf));
		
			//接收信息
			if(recv(sfd, buf, sizeof(buf), 0) <=0)
			{   
				ERR_MSG("recv");
				return -1; 
			}

			if(strcasecmp(buf,"OK") == 0)
			{
				printf("员工%s信息修改成功\n",user.name);
				return 0;
			}   
			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);

			break;
			case '2':
			//返回上一级
			return 0;		
		default:
			printf("输入错误,请重新输入\n");
		}
	}
}

///}}}

//普通员工查询同事基本信息
///{{{
int do_browse(int sfd)
{
	//向服务器发送普通员工查询同事基本信息，协议为'B'
	char buf[N] = "B";
    USER user;
	int res = 0;

	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }

	printf("请输入你要查询同事的工号>>>");
	scanf("%s",user.job_number);
	while(getchar() != 10);
	
	//发送数据
    if(send(sfd, &user, sizeof(user), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

	//接收数据
	res = recv(sfd, (void *)&user, sizeof(user), 0); 
	if(res < 0)                                                                              
	{   
		ERR_MSG("recv");                                                                     
		return -1; 
	}   
	else if(0 == res)
	{   
		printf("服务器关闭\n");
		return 0;
	}

	if(strcasecmp(user.section,"NULL") == 0)
	{
		printf("该系统没有该同事的信息\n");
		return 0;
	}

    printf("-------------------------------------------------\n");
	printf("部门:%s\n",user.section);
	printf("工号:%s\n",user.job_number);
	printf("姓名:%s\n",user.name);
	printf("性别:%s\n",user.sex);
	printf("电话号码:%s\n",user.phone_number);
	printf("地址:%s\n",user.address);
	memset(&user,0,sizeof(user));

	//接收数据
	bzero(buf,sizeof(buf));
	res = recv(sfd, buf, sizeof(buf), 0); 
	if(res < 0)                                                                              
	{   
		ERR_MSG("recv");                                                                     
		return -1; 
	}   
	else if(0 == res)
	{   
		printf("服务器关闭\n");
		return 0;
	}
	if(strcasecmp(buf,"**NULL**") == 0)
	{
    	printf("-------------------------------------------------\n");
		printf("以上是查询到同事的个人基本信息    ");
	}

	bzero(buf,sizeof(buf));
	
	//接收数据
    if(recv(sfd, buf, sizeof(buf), 0) <= 0)
    {   
        ERR_MSG("recv");
        return -1; 
    }   
	printf("查询时间：%s",buf);
	return 0;

}
///}}}

//普通员工修改个人信息
///{{{
int do_change(int sfd)
{
	//向服务器发送员工修改个人信息的协议'C'
	char buf[N] = "C";
	char buf1[N] = "";
	int res = 0;
	char choice;
    USER user;
	//发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }
	if(send(sfd, &staff_login, sizeof(staff_login), 0) < 0)
	{   
		ERR_MSG("send");
		return -1; 
	}


	while(1)
	{
		system("clear");
		printf("-------------------------------------------------\n");
		printf("-----------------修改个人信息--------------------\n");
		printf("-------------------------------------------------\n");
		printf("-----------------1.修改姓名----------------------\n");
		printf("-----------------2.修改年龄----------------------\n");
		printf("-----------------3.修改电话号码------------------\n");
		printf("-----------------4.修改地址----------------------\n");
		printf("-----------------5.修改密码----------------------\n");
		printf("-----------------6.返回上一级--------------------\n");
		printf("-------------------------------------------------\n");
		printf("请输入选项>>>");
		scanf("%c", &choice);
		while(getchar()!=10);        
		
		//发送协议
		bzero(buf,sizeof(buf));
		buf[0] = choice;
		if(send(sfd, buf, sizeof(buf), 0) < 0)
		{   
			ERR_MSG("send");
			return -1; 
		}
		switch(choice)
		{
		case '1':
			bzero(buf,sizeof(buf));
			printf("新的名字>>>");
			scanf("%s",buf);
			while(getchar() != 10);
			
			//发送数据
			if(send(sfd, buf, sizeof(buf), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}

			//接收数据
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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

			if(strcasecmp(buf,"OK1") == 0)
			{
				printf("姓名修改成功\n");
				return 0;
			}   

			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);

			break;
		case '2':
			bzero(buf,sizeof(buf));
			printf("新的年龄>>>");
			scanf("%s",buf);
			while(getchar() != 10);
			if(send(sfd, buf, sizeof(buf), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}
			
			//接收数据
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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

			if(strcasecmp(buf,"OK2") == 0)
			{
				printf("年龄修改成功\n");
				return 0;
			}   

			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);
			break;
		case '3':
			bzero(buf,sizeof(buf));
			printf("新的电话号码>>>");
			scanf("%s",buf);
			while(getchar() != 10);
			if(send(sfd, buf, sizeof(buf), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}

			//接收数据
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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

			if(strcasecmp(buf,"OK3") == 0)
			{
				printf("电话号码修改成功\n");
				return 0;
			}   

			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);
			break;
		case '4':
			bzero(buf,sizeof(buf));
			printf("新的地址>>>");
			scanf("%s",buf);
			while(getchar() != 10);
			if(send(sfd, buf, sizeof(buf), 0) < 0)
			{   
				ERR_MSG("send");
				return -1; 
			}
			
			//接收数据
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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

			if(strcasecmp(buf,"OK4") == 0)
			{
				printf("地址修改成功\n");
				return 0;
			}   

			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);
			break;
		case '5':
			bzero(buf,sizeof(buf));
			printf("新的密码>>>");
			scanf("%s",buf);
			while(getchar() != 10);
			printf("请再一次确认新的密码>>>");
			scanf("%s",buf1);
			while(getchar() != 10);
			if(strcasecmp(buf,buf1) == 0)
			{

				if(send(sfd, buf, sizeof(buf), 0) < 0)
				{   
					ERR_MSG("send");
					return -1; 
				}
				printf("正在向服务器发送更改密码请求....\n");
			}   
			else
			{
				printf("两次输入密码不一致，修改密码失败!\n");
				return -1;
			}
			
			//接收数据
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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

			if(strcasecmp(buf,"OK5") == 0)
			{
				printf("密码修改成功\n");
				printf("新密码为:%s,请不要忘记!\n",buf1);
				return 0;
			}   
			bzero(buf,sizeof(buf));
			res = recv(sfd, buf, sizeof(buf), 0);
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
			printf("新密码为:%s,请不要忘记\n!",buf1);
			bzero(buf,sizeof(buf));

			printf("输入任意字符,清屏>>>");
			while(getchar()!=10);
			break;
		case '6':
			return 0;
		default:
			printf("输入错误,请重新输入\n");
		}
	}


}
///}}}

//返回上一级
///{{{
int do_go_back(int sfd)
{
    //向服务器发送返回上一级协议'G'
    char buf[N] = "G";
                                                                          
    //发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

 	//发送数据
    if(send(sfd, &staff_login, sizeof(staff_login), 0) < 0)
    {
        ERR_MSG("send");
        return -1;                                                        
    }

    return 0;
}
///}}}

//退出
///{{{
int do_quit(int sfd)
{
	 //向服务器发送退出协议'Q'
    char buf[N] = "Q";
                                                                          
    //发送协议
    if(send(sfd, buf, sizeof(buf), 0) < 0)
    {   
        ERR_MSG("send");
        return -1; 
    }   

 	//发送数据
    if(send(sfd, &staff_login, sizeof(staff_login), 0) < 0)
    {
        ERR_MSG("send");
        return -1;                                                        
    }

    return 0;
}
///}}}

