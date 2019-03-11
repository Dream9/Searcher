#include<iostream>
#include<string>
#include<stdio>
#include<fstream>
#include<stream>
#include<stdlib>
#include"Serror.h"
using std::string;
using 
class log{
private:
    string service_name;
    string service_env;
    fstream file_handle;
public:
    explicit log(const string& service) {
        stringstream out;
        out<<"./logs/"<<service<<".log";
        filename=out.str();
        file_handle.open(filename,ofstream::app|ofstream::out|ofstream::in);
        if(!file_handle.is_open()){
            //不能访问文件
            throw Serror("文件不能打开",2);
        }
        //获取环境变量
        service_env=getenv("ENV_DEPLOY");
        if(service_env=="")
            service_env="TESTING";
    }

    //析够
    ～log(){
        if(file_handle.is_open())
            file_handle.close();
    }
    //
    bool is_open(){
        return file_handle.is_open();
    }
    int Close(){
        if(file_handle.is_open()){
            file_handle.close();
            return 0;
        }
        return -1;
    }
    int logging(const string& level,const string& info){
        if(is_open()){
            file_handle<<level<<":"<<info<<"\r\n";
            return 0;
        }
        return -1;
    }
    int Error(const string& info){
        return logging("ERROR",info);
    }
    int Warn(const string& info){
        return logging("WARN",info);
    }
    int Debug(const string& info){
        return logging("DEBUG",info);
    }
    int Info(const string& info){
        return logging("INFO",info);
    }
    int Fatal(const string& info){
        return logging("FATAL",info);
    }

}
