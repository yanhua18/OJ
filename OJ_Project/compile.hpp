#pragma once 
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/resource.h>
#include<sys/wait.h>
#include<string>
#include<jsoncpp/json/json.h>
#include"oj_log.hpp"
#include"tools.hpp"

enum ErrorNo
{
    OK=0,
    COMPILE_ERROR,
    RUN_ERROR,
    PRAM_ERROR,
    INTERNAL_ERROR
};
class Compiler
{
    public:
        //有可能浏览器对不同的题目提交的数据是不同的
        //code="xxxx"
        //code="xxx"&stdin="xxx"
        static void CompileAndRun(Json::Value Req,Json::Value*  Resp)
        {
            //1,判空
            if(Req["code"].empty())
            {
                (*Resp)["errorno"]=PRAM_ERROR;
                (*Resp)["reason"]="Pram error";
                LOG(ERROR,"Request code is Empth!!");
                std::cout<<std::endl;
                return;
            }
            //2,将代码写到文件当中去
            //文件名称进行约定 tmp_时间戳.cpp
            std::string code=Req["code"].asString();
            std::string tmp_filename=WriteTmpFile(code);
            if(tmp_filename=="")
            {
                (*Resp)["errorno"]=INTERNAL_ERROR;
                LOG(ERROR,"Write Source failed");
                (*Resp)["reason"]="Create file failed";
                return;
            }
            //3,编译
            if(!Compile(tmp_filename))
            {
                (*Resp)["errorno"]=COMPILE_ERROR;
                std::string reason;
                FileOperater::ReadDataFromFile(ErrorPath(tmp_filename),&reason);
                (*Resp)["reason"]=reason;
                LOG(ERROR,"compile error");
                std::cout<<std::endl;
                return;
            }
            //4，运行
            int sig=Run(tmp_filename);
            if(sig != 0)
            {
                (*Resp)["errorno"]=RUN_ERROR;
                (*Resp)["reason"]="Program exit by sig"+std::to_string(sig);
                LOG(ERROR,"Run Error");
                std::cout<<std::endl;
                return;
            }
            //5，构造响应
            (*Resp)["errorno"]=OK;
            (*Resp)["reason"]="Compile and run is okey";
            //标准输出
            std::string stdout_reason;
            FileOperater::ReadDataFromFile(StdoutPath(tmp_filename),&stdout_reason);
            (*Resp)["stdout"]=stdout_reason;
            //标准错误
            std::string stderr_reason;
            FileOperater::ReadDataFromFile(StderrPath(tmp_filename),&stderr_reason);
            (*Resp)["stderr"]=stderr_reason;

            //清理掉临时文件
            Clean(tmp_filename);

            return;
        }
    private:
        static std::string WriteTmpFile(const std::string& code)
        {
            //1,组织文件的名称，组织文件的前缀名称，用来区分源码文件，可执行文件是.一组数据
            std::string tmp_filename="/tmp_"+std::to_string(LogTime::GetTimeStamp());
            //2,写文件
            int ret= FileOperater::WriteDataToFile(SrcPath(tmp_filename),code);
            if(ret<0)
            {
                LOG(ERROR,"write code to source fail");
                return "";
            }
            return tmp_filename;
        }
        static std::string SrcPath(const std::string& filename)
        {
           return "./tmp_files"+filename+".cpp";
        }

        static std::string ExePath(const std::string& filename)
        {
           return "./tmp_files"+filename+".executable";
        }

        static std::string ErrorPath(const std::string& filename)
        {
           return "./tmp_files"+filename+".err";
        }

        static std::string StdoutPath(const std::string& filename)
        {
           return "./tmp_files"+filename+".stdout";
        }
        static std::string StderrPath(const std::string& filename)
        {
           return "./tmp_files"+filename+".stderr";
        }
        static bool Compile(const std::string& filename)
        {
            //1,构造编译命令-->g++ src -o [exec] -std=c++11
            
            const int commandcount=20;
            char buf[commandcount][50]={{0}};
            char* Command[commandcount]={0};
            for(int i=0;i<commandcount;i++)
            {
                Command[i]=buf[i];
            }
            snprintf(Command[0],49,"%s","g++");
            snprintf(Command[1],49,"%s",SrcPath(filename).c_str());
            snprintf(Command[2],49,"%s","-o");
            snprintf(Command[3],49,"%s",ExePath(filename).c_str());
            snprintf(Command[4],49,"%s","-std=c++11");
            snprintf(Command[5],49,"%s","-D");
            snprintf(Command[6],49,"%s","CompileOnline");
            Command[7]=NULL;
            //2,创建子进程
            //2.1父进程-->等待子进程退出
            //2.2子进程-->进程程序替换--->g++
            int pid = fork();
            if(pid<0)
            {
                LOG(ERROR,"创建子进程失败！！！！");
                return false;
            }
            else if(pid==0)
            {
                //child
                int fd=open(ErrorPath(filename).c_str(),O_CREAT|O_RDWR,0664);
                if(fd<0)
                {
                    LOG(ERROR,"打开compile文件失败");
                    std::cout<<ErrorPath(filename)<<std::endl;
                }
                //重定向
                dup2(fd,2);
                //程序替换
                execvp(Command[0],Command);
                perror("execvp");
                LOG(ERROR,"execvp failed");
                std::cout<<std::endl;
                exit(0);
            }
            else
            {
                //father
                waitpid(pid,NULL,0);
            }
            //3,验证是否生成可执行程序
            //int stat(const char* filename,struct stat* buf)通过文件名获取文件信息，将文件信息保存在buf中，返回成功为0，失败-1，ENOENT:文件不存在；ENOTDIR：传入的文件目录不对
           struct stat st;
           int ret=stat(ExePath(filename).c_str(),&st);
           if(ret<0)
           {
               LOG(ERROR,"编译错误！找不到可执行程序！！");
               std::cout<<ExePath(filename)<<std::endl;
               return false;
           }
           return true;
        }

        static int Run(std::string& filename)
        {
            //可执行程序
            //1，创建子进程
            //父进程   进程等待
            //子进程   替换编译出来的程序
            int pid=fork();
            if(pid<0)
            {
                LOG(ERROR,"替换程序失败，创建子进程失败！！");
                std::cout<<std::endl;
                return -1;
            }
            else if(pid==0)
            {
                //对于一个子进程执行的限制
                //1，时间限制 alarm    int setrlimit(int type,struct rlimit* rlim);
                alarm(1);
                //2, 内存大小限制
                struct rlimit rl;
                rl.rlim_cur=1024*30000;
                rl.rlim_max=RLIM_INFINITY;
                setrlimit(RLIMIT_AS,&rl);

                //child
                
                //获取：标准输出--->重定向到文件
                int stdout_fd=open(StdoutPath(filename).c_str(),O_CREAT|O_RDWR,0664);
                if(stdout_fd<0)
                {
                    LOG(ERROR,"打开输出文件失败！！！");
                    std::cout<<StdoutPath(filename)<<std::endl;
                    return -1;
                }
                dup2(stdout_fd,1);

                //标准错误--》重定向到文件
                int stderr_fd=open(StderrPath(filename).c_str(),O_CREAT|O_RDWR,0664);
                if(stdout_fd<0)
                {
                    LOG(ERROR,"打开标准错误文件失败！！！");
                    std::cout<<StderrPath(filename)<<std::endl;
                    return -1;
                }
                dup2(stderr_fd,1);

                execl(ExePath(filename).c_str(),ExePath(filename).c_str(),NULL);
                exit(1);
                
            }
            //father
            int status=-1;
            waitpid(pid,&status,0);
            //将是否收到信号的信息返回给调用者，如果调用者的判断是0，则正常运行完毕，否则收到某个信号，异常结束
            return status&0x7f;
        }
        static void Clean(std::string filename)
        {
            unlink(SrcPath(filename).c_str());
            unlink(ExePath(filename).c_str());
            unlink(ErrorPath(filename).c_str());
            unlink(StdoutPath(filename).c_str());
            unlink(StderrPath(filename).c_str());
        }
            
};
