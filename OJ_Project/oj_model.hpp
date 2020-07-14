#pragma once
#include<iostream>
using namespace std;
#include<unistd.h>
#include<unordered_map>
#include<string>
#include<fstream>
#include<algorithm>
#include"tools.hpp"
#include"oj_log.hpp"

typedef struct Question
{
    std::string id_;
    std::string name_;
    std::string path_;
    std::string star_;
}QUES;
class oj_Model
{
    public:
        oj_Model()
        {
            LoadQuestions("./config_oj.cfg");
        }
        bool GetAllQuestions(std::vector<Question>* ques)
        {
            for(const auto& kv:model_map_)
            {
                ques->push_back(kv.second);
            }
            std::sort(ques->begin(),ques->end(),[](const Question& l,const Question& r)
                    {
                   return atoi(l.id_.c_str()) < std::atoi(r.id_.c_str());
                    });
            return true;
        }

        bool GetOneQuestion(const std::string& id,std::string* desc,std::string* header,Question* ques)
        {
            //1,根据id去查找对应题目的信息，最重要的就是这个题目在哪里加载
            auto iter=model_map_.find(id);
            if(iter==model_map_.end())
            {
                LOG(ERROR,"Not Fount Question id is");std::cout<< id << std::endl;
                return false;
            }
            *ques=iter->second;
            //iter->second.path_;
            //去加载具体的单个题目信息，从保存的路径上面去加载,从具体的题目文件中去获取描述与header信息
            int ret = FileOperater::ReadDataFromFile(DescPath(iter->second.path_),desc); 
            if(ret==-1)
            {
                LOG(ERROR,"Read desc failed");std::cout<< std::endl;
                return false;
            }
            ret = FileOperater::ReadDataFromFile(HeaderPath(iter->second.path_),header); 
            if(ret==-1)
            {
                LOG(ERROR,"Read desc failed");std::cout << std::endl;
                return false;
            }
           return true; 
        }
        bool SplicingCode(std::string user_code,const std::string ques_id,std::string* code)
        {
            //1,查找对应ID的题目是否存在
           auto iter=model_map_.find(ques_id);
           if(iter == model_map_.end())
           {
               LOG(ERROR,"没有找到该题目，此题目的id是");
               std::cout<<ques_id<<std::endl;
               return false;
           }
           std::string tail_code;
           int ret =  FileOperater::ReadDataFromFile(TailPath(iter->second.path_),&tail_code); 
           if(ret<0)
           {
               LOG(ERROR,"打开tail.cpp文件失败");
               return false;
           }
           *code=user_code+tail_code;
           return true;
        }
    private:
        std::string DescPath(const std::string& ques_path)
        {
            return ques_path + "desc.txt";
        }
        std::string HeaderPath(const std::string& ques_path)
        {
            return ques_path + "header.cpp";
        }
        std::string TailPath(const std::string& ques_path)
        {
            return ques_path + "tail.cpp";
        }
    private:
        bool LoadQuestions(const std::string& configfile_path)
        {
            //使用C++中的文件流来加载文件，获取文件中的内容
            std::ifstream file(configfile_path.c_str());
            if(!file.is_open())
            {
                return false;
            }

            std::string line;
            while(std::getline(file,line))
            {
                //需要切割字符串，并将其保存到unordered_map中
                std::vector<std::string> vec;
                StringTools::Split(line," ",&vec);
                if(vec.size()!=4)
                {
                    continue;
                }
                Question ques;
                ques.id_=vec[0];
                ques.name_=vec[1];
                ques.path_=vec[2];
                ques.star_=vec[3];
                model_map_[ques.id_]=ques;
            }
            file.close();
        }
    private:
        std::unordered_map<std::string ,Question> model_map_;
};
