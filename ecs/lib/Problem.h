//
// Created by sheng on 18-5-1.
//

#ifndef ECS_PROBLEM_H
#define ECS_PROBLEM_H

#include <iostream>
#include <sstream>
#include "Flavor.h"
#include "Host.h"


//问题描述类
class  Problem
{
public:
    bool predict_time_need_increase=false;//适应预测结束时间的两种表达方式
    int serverNumCPU,serverNumMEM;//主机规格,内存单位为G(初赛专用)
    std::vector<Flavor> v_flavors_info;//本次优化需考虑的虚拟机规格数据
    std::vector<Host> v_hosts_info;//本次优化需考虑的物理机规格数据
    int flavor_types;//本次优化需考虑的虚拟机种类数
    int host_types;//本次优化需考虑的主机种类数
    std::string optimize_type;//优化利用率维度:CPU/MEM(初赛专用)
    std::string predict_start_time,predict_end_time,predict_end_minute;//预测起止时间
    Problem(char **const info)//问题描述类构造
    {
        int line_num=0;
        std::istringstream *problemInfo;
        problemInfo=new std::istringstream(info[0]);
        *problemInfo>>host_types;
        for(line_num=1;line_num<host_types+1;line_num++)
        {
            Host host(info[line_num]);
            v_hosts_info.push_back(host);
        }
        problemInfo=new std::istringstream(info[++line_num]);
        *problemInfo>>flavor_types;
        int flavor_info_start_line=line_num+1;
        for(line_num=flavor_info_start_line;line_num<flavor_types+flavor_info_start_line;line_num++)
        {
            Flavor flavor(info[line_num]);
            v_flavors_info.push_back(flavor);
        }
        problemInfo=new std::istringstream(info[++line_num]);
        *problemInfo>>predict_start_time;
        problemInfo=new std::istringstream(info[++line_num]);
        *problemInfo>>predict_end_time>>predict_end_minute;
        if(predict_end_minute.find("59")==std::string::npos)predict_time_need_increase=false;
        else predict_time_need_increase=true;

        optimize_type="CPU";//以下为兼容初赛代码
        serverNumCPU=56;
        serverNumMEM=128;
    }

    void Problem_printf()//打印输出问题信息
    {
        std::cout<<"host_types="<<host_types<<std::endl;
        for(auto host_type:v_hosts_info)
            std::cout<<"host_type_name="<<host_type.name<<" cpu="
                <<host_type.numCPU<<" mem="<<host_type.numMEM<<std::endl;
        std::cout<<"flavor_types="<<flavor_types<<std::endl;
        for(auto flavor_type:v_flavors_info)
            std::cout<<"flavor_type_name="<<flavor_type.name<<" cpu="
                <<flavor_type.numCPU<<" mem="<<flavor_type.numMEM<<std::endl;
        std::cout<<"predict_start_time="<<predict_start_time<<std::endl;
        std::cout<<"predict_end_time="<<predict_end_time<<std::endl;
    }
};

#endif //ECS_PROBLEM_H
