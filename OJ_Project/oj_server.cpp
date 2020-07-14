#include"httplib.h"
#include"oj_model.hpp"
#include<stdio.h>
#include<string.h>
#include<string>
#include"Oj_view.hpp"
#include"oj_log.hpp"
#include"compile.hpp"
int main()
{
    using namespace httplib;
    Server svr;
    //1,获取试题的信息（从文件中获取）
    oj_Model ojmode;
    svr.Get("/all_questions",[&ojmode](const Request& req , Response& resp)
            {
            std::vector<Question> ques;
            ojmode.GetAllQuestions(&ques);

//            char buf[10240]={'\0'};
//           printf("%d\n",ques.size());
//           if(ques.size()==1)
//           {
//           snprintf(buf,sizeof(buf)-1,"<html>%s.%s %s</html>",ques[0].id_.c_str(),ques[0].name_.c_str(),ques[0].star_.c_str());
//           }
//           std::string html;
//           html.assign(buf,strlen(buf));
            
            std::string html;
            oj_view::ExpandAllQuestionshtml(&html,ques);
            //LOG(INFO,html);
            resp.set_content(html,"text/html;charset=UTF-8");
            });
    //正则表达式
    //    \b单词的分界
    //    * 匹配任意字符串
    //    \d 匹配一个数字
    //    源码转义：特殊字符按照特殊字符字面源码来进行编译 R"(stdr)"
    svr.Get(R"(/question/(\d+))",[&ojmode](const Request& req , Response& resp)
            {
            //1,去实体模块去查找对应题号的具体题目信息（map当中）
            std::string desc;
            std::string header;
            //从querystr中获取id

            LOG(INFO,"req.matches");std::cout<<req.matches[0]<<":"<<req.matches[1]<<std::endl;
            //2,在题目地址的路径下去加载题目的具体描述信息
            struct Question ques;
            ojmode.GetOneQuestion(req.matches[1].str(),&desc,&header,&ques);
           //3,进行组织，并返回给浏览器             
            std::string html;
            oj_view::ExpandOneQuestion(ques,desc,header,&html);

            resp.set_content(html,"text/html;charset=UTF-8");
            });

    svr.Post(R"(/question/(\d+))",[&ojmode](const Request& req , Response& resp){
            //key:value
            //1,从正文中提取出来用户提交的code字段，提交的内容当中有URL
            std::unordered_map<std::string,std::string> pram;
            UrlUtil::PraseBody(req.body,&pram);
 //           for(const auto& pr:pram)
 //           {
 //           LOG(INFO,"code");
 //           std::cout<<pr.second;
 //           std::cout<<std::endl;
 //           }
            //2,编译运行
            // 2.1需要给提交的代码增加头文件，测试用例，main函数
            std::string code;
            ojmode.SplicingCode(pram["code"],req.matches[1].str(),&code);
            //LOG(INFO,"code");
            //std::cout<<code<<std::endl;
            Json::Value req_json;
            req_json["code"]=code;
            Json::Value Resp_json;
            Compiler::CompileAndRun(req_json,&Resp_json);
            //3,构造响应，
            const std::string errorno=Resp_json["errorno"].asString();
            const std::string reason=Resp_json["reason"].asString();
            const std::string stdout_reason=Resp_json["stdout"].asString();
            std::string html;
            oj_view::ExpandReason(errorno,reason,stdout_reason,&html);
            resp.set_content(html,"text/html;charset=UTF-8");
            });

    LOG(INFO,"listen for 39.101.171.114:18888");std::cout<<std::endl;
    LOG(INFO,"Server ready");std::cout<<std::endl;
    //listen函数会阻塞
    svr.listen("0.0.0.0",18888);
    return 0;
}
