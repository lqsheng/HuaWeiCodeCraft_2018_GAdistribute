//
// Created by sheng on 18-5-1.
//

#ifndef ECS_PREDICTRESULT_H
#define ECS_PREDICTRESULT_H

#include <algorithm>
#include "Flavor.h"
#include "Host.h"
#include "Problem.h"

//预测结果描述类
class PredictResult
{
public:
    int totalnums;//总虚拟机数量
    std::vector<Flavor> flavor_nums;//各规格虚拟机数量
    bool optimzeCPU;//资源优化维度
    PredictResult(int nums,const Problem &problem)
    {
        totalnums=nums;
        if(problem.optimize_type=="CPU")optimzeCPU=true;
        else optimzeCPU=false;
    }
    void DownSort(void)//降排序
    {
        if(optimzeCPU)
            std::sort(flavor_nums.begin(),flavor_nums.end(),DownSortFun_CPU);
        else
            std::sort(flavor_nums.begin(),flavor_nums.end(),DownSortFun_MEM);
    }
    void PredictResultPrintf(void)
    {
        std::cout<<"********************predict result start**********************"<<std::endl;
        std::cout<<"predict result:"<<std::endl;
        std::cout<<"total flavors="<<totalnums<<std::endl;
        std::cout<<"flavors list=";
        for(int i=0;i<flavor_nums.size();i++)
        {
            std::cout<<flavor_nums[i].name<<"="<<flavor_nums[i].count<<",";
        }
        std::cout<<std::endl;
        std::cout<<"********************predict result end**********************"<<std::endl;
    }
private:
    static bool DownSortFun_MEM (Flavor a,Flavor b)
    {
        if(a.numMEM>b.numMEM)return true;
        else if(a.numMEM<b.numMEM)return false;
        else return (a.numCPU>b.numCPU);
    }
    static bool DownSortFun_CPU (Flavor a,Flavor b)
    {
        if(a.numCPU>b.numCPU)return true;
        else if(a.numCPU<b.numCPU)return false;
        else return (a.numMEM>b.numMEM);
    }
};

#endif //ECS_PREDICTRESULT_H
