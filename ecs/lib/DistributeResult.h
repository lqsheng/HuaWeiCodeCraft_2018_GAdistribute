//
// Created by sheng on 18-5-1.
//

#ifndef ECS_DISTRIBUTERESULT_H
#define ECS_DISTRIBUTERESULT_H

#include <random>
#include "Flavor.h"
#include "Host.h"
#include "Problem.h"
#include "DataContainer.h"
#include "PredictResult.h"

//交换信息描述结构体
struct ExchangeInfo{
    int src_predict_index,src_host_index,dst_predict_index;
    float rate;
};
//交换描述类
class ExchangeEval
{
public:
    std::vector<ExchangeInfo> exchangeInfos;

    ExchangeEval() {}
    void UpSort(void)//降排序
    {
        std::sort(exchangeInfos.begin(),exchangeInfos.end(),UpSortFun);
    }

private:
    static bool UpSortFun(ExchangeInfo a,ExchangeInfo b)
    {
        if(a.rate<b.rate)return true;
        else return false;
    }
};

//分配结果描述类
class DistributeResult
{
public:
    std::vector<std::vector<Flavor> > flavor_distribute_result;//分配方案(初赛专用)
    std::vector<Host> distribute_result;//总分配方案(复赛专用)以下三个是对总分配方案的整理细分
    std::vector<Host> distribute_res_normal;//通用型分配方案(复赛专用)
    std::vector<Host> distribute_res_cpu;//高性能型分配方案(复赛专用)
    std::vector<Host> distribute_res_mem;//高内存分配方案(复赛专用)
    bool optimzeCPU;//优化维度：CPU/MEM(初赛专用)
    int total_hosts;//分配所需总的物理主机数(初赛专用)
    int normal_host_num,cpu_host_num,mem_host_num;//分配所需各型号物理主机数(复赛专用)
    std::default_random_engine random_engine;//随机数引擎
    DistributeResult(){}
    void FFD_Distribute(const Problem &problem,const PredictResult &predictResult);//降序首次适应分配算法(初赛专用)
    void BFD_Distribute(const Problem &problem,const PredictResult &predictResult);//降序最佳适应分配算法(初赛专用)
    void BF_Distribute_for_GA(const Problem &problem,std::vector<Flavor> &predictResult);//最佳适应分配算法,为遗传算法初始化种群使用(初赛专用)
    void FF_Distribute_for_GA(const Problem &problem,std::vector<Flavor> &predictResult);//首次适应分配算法,为遗传算法初始化种群使用(初赛专用)
    void RF_Distribute_for_GA(const Problem &problem,std::vector<Flavor> &predictResult);//随机适应分配算法,为遗传算法初始化种群使用(初赛专用)
    void GreedyFF_Distribute_for_GA(const Problem &problem,std::vector<Flavor> &predictResult);//贪心分配算法,为遗传算法初始化种群使用(复赛专用)
    void result_put_in_order();//整理分配结果
    void adjust(const Problem &problem,PredictResult &predictResult, bool adjust_last_host=false,float rest_rate_limit=0.9);
    void adjust_for_repecharge(const Problem &problem,PredictResult &predictResult, bool adjust_last_host=false,float rest_rate_limit=0.9);
    void adjust_greedy_exchange(const Problem &problem,PredictResult &predictResult);
    float compute_res_used_rate(const PredictResult &predictResult);//计算资源利用率
    void printfDistributeResult(void);//打印分配结果
};


#endif //ECS_DISTRIBUTERESULT_H
