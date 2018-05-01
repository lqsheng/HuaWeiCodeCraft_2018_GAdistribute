//
// Created by sheng on 18-5-1.
//

#ifndef ECS_HOST_H
#define ECS_HOST_H

#include <string>
#include <sstream>
#include <vector>
#include "Flavor.h"

//物理机规格类
class Host
{
public:
    int numCPU,numMEM;//cpu数和内存数,内存单位为G
    int cpu_rest,mem_rest;//剩余内存及cpu
    int count;//此类型主机数量
    std::string name;//主机名称
    std::vector<Flavor> flavor_index;//放置方案
    Host(char *const info)
    {
        std::istringstream hostInfo(info);
        hostInfo>>name>>numCPU>>numMEM;
        cpu_rest=numCPU;
        mem_rest=numMEM;
        count=0;
    }
    Host(int numcpu,int nummem,std::string strname,int cpurest,int memrest)
    {
        numCPU=numcpu;
        numMEM=nummem;
        cpu_rest=cpurest;
        mem_rest=memrest;
        count=0;
        name=strname;
    }
    Host(){}
    float compute_res_used_rate()//利用率计算
    {
        float mem_rate=(float)(numMEM-mem_rest)/(float)numMEM;
        float cpu_rate=(float)(numCPU-cpu_rest)/(float)numCPU;
        return (mem_rate+cpu_rate)/2;
    }
    bool put_flavor(Flavor flavor)//放置一台虚拟机
    {
        if((cpu_rest>=flavor.numCPU)&&(mem_rest>=flavor.numMEM))
        {
            flavor_index.push_back(flavor);
            cpu_rest-=flavor.numCPU;
            mem_rest-=flavor.numMEM;
            return true;
        }
        return false;
    }
};

#endif //ECS_HOST_H
