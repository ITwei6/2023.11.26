
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define LEFT  "["
#define RIGHT "]"
#define LABLE "#"
#define LINE 1024
#define ARGV_SIZE 32
#define DELIM " "
#define EXIT_CODE 11
char commandline[LINE];//用户输入的命令行
char *argv[ARGV_SIZE];//命令行参数表，作为输出型参数，将分割的命令行子串保存,保存每个字串的地址
int argc=0;
int lastcode=0;//子进程的结果存这里
char pwd[LINE];//存储当前目录
int i=0;//用来计算切割的字符个数

//1.shell 一开始就是一行命令行，表明用户是谁，主机是谁,当前目录在哪等,我们可以直接从环境变量里获取
//2.命令行输入完后，就是用户要输入的命令了---->封装成interact函数
//3.当命令行输入完后，shell就要对这个命令行进行解析了，将命令行分割成一个一个字符
//ls -a -b -c -->  "ls" "-a" "-b",将每个字串的地址保存在命令行表里 ---->封装成spilt函数

const char* getusername()
{
   return getenv("USER");
}

const char* gethostname()
{
  return getenv("HOSTNAME");
}
void  getpwd()
{
   //直接调用系统接口获取当前目录,并写入字串里
   getcwd(pwd,sizeof(pwd));
}

//1.获取用户输入的命令
void interact(char* cline,int size)
{
  getpwd();//更新一下当前的目录并放入到pwd字符串里
  printf(LEFT"%s@%s%s"RIGHT""LABLE" ",getusername(),gethostname(),pwd);
 char*s= fgets(cline,size,stdin); //后面不会再用到s，这里处理一下
 assert(s);//断言声明一下，当s为null时就报错
 (void)s;//表示用过

 cline[strlen(cline)-1]='\0';
 //因为最后无论如何都要按enter，所以必定会有\换行，abcd\n\0但是我们并不想要这个换行
}
//2.子串分割问题，解析命令行
int splitstring(char _commandline[],char *argv[])
{
    //利用strtok来分割字串串，strtok的用法是第一次传字符串参数，以后就不用传，设为NULL
    argv[i++]=strtok(_commandline,DELIM);
    while(argv[i++]=strtok(NULL,DELIM));

    return i-1;//最后还会加1所以需要减1.
      
}

//4.普通命令的执行--->通过创建子进程来执行,子进程执行，退出，父进程等待
void normalexcute(char *_argv[])
{

 //对于cd命令，执行命令的是子进程关父进程什么事，所以pwd显示的还是父进程当前目录，所以cd应该是父进程执行，而不是子进程执行,其实cd是内健命令

 pid_t id =fork();
 if(id<0)
 {
   perror("fork错误");
   return;
 }
 else if(id==0)//子进程
 {
     //子进程如何执行普通命令呢？通过进程替换！exec* 借助库函数里的，需要带p的v的
    execvp(_argv[0],_argv);
    //不会返回，如果返回了那么就说明进程替换错误
    exit(EXIT_CODE);
     
 }
 else//父进程获取子进程的退出结果 
 {
 
   int status=0;
   pid_t ret=waitpid(id,&status,0);
   if(ret==id)
   {
      lastcode=WEXITSTATUS(status);
   }
 }

}

int buildcommand(char*_argv[],int _argc)
{

 if(_argc==2&&strcmp(_argv[0],"cd")==0)
 {
    chdir(_argv[1]);//直接调用系统接口，跳到指定目录
    //跳到指定目录后，环境变量里的PWD也需要更改到当前目录

    sprintf(getenv("PWD"),"%s",pwd);//获取到PWD环境变量的内容并将pwd写入到PWD里
    return 1;//执行完内建命令后，普通命令就跳过
    //对于内建命令，本质就是shell的内部的一个函数
 }
 if(strcmp(_argv[0],"ls")==0)
 {
   //如果是ls命令，我们给它添加一个命令行，可以加颜色
   _argv[i++]="--color";
   _argv[i]=NULL;
 }


}
int main()
{
  //shell本质上是一个死循环,一直在使用
  int quit=0;
  while(!quit)
  {

 interact(commandline,sizeof(commandline));
 argc=splitstring(commandline,argv);
 if(argc==0)continue;//表明是空串
 int n = buildcommand(argv,argc);

 //在执行命令之前需要判断是否是内建命令比如cd 目录
 if(!n)normalexcute(argv);
  }
 return 0;
}
