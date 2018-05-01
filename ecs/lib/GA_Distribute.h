//
// Created by sheng on 18-5-1.
//

#ifndef ECS_GA_DISTRIBUTE_H
#define ECS_GA_DISTRIBUTE_H

#include <random>
#include "Flavor.h"
#include "Host.h"
#include "Problem.h"
#include "DataContainer.h"
#include "PredictResult.h"
#include "DistributeResult.h"

//遗传分配
class GA_Distribute
{
public:
    GA_Distribute(int max_generation_,int group_num_,float cross_rate_, float varia_rate_);//构造遗传优化问题
    DistributeResult run(Problem &problem,PredictResult &predictResult);//迭代进化

private:
    struct Entity//个体定义
    {
        DistributeResult distributeResult;
        float fitness=0;
    };
    int group_num;//群体数量
    float cross_rate;//交叉概率
    float varia_rate;//变异概率
    int max_generation;//进化代数
    std::vector<Entity> entities;//群体
    Entity bestEntity;//最优个体
    std::vector<Flavor> predict_result_for_GA;//预测结果
    bool optimzeCPU;//优化资源维度(初赛专用)
    int serverNumCPU,serverNumMEM;//主机规格(初赛专用)
    std::default_random_engine random_engine;//随机数引擎
    void initEntities(Problem &problem,PredictResult &predictResult);//生成初始种群
    void computeFitness(int cpu_lim,int mem_lim,PredictResult &predictResult);//计算适应度
    static bool entitySort(const Entity &one, const Entity &two) {return (one.fitness<two.fitness);}//升序
    void recordBestEntity();//记录最优个体
    void select();// 选择
    void GreedyFF_adjust(Problem &problem,Entity &entity,std::vector<Flavor> flavor_rest);//贪心放置调整(复赛专用)
    void FF_adjust(Entity &entity,std::vector<Flavor> flavor_rest);//首次适应放置调整(初赛专用)
    void RF_adjust(Entity &entity,std::vector<Flavor> flavor_rest);//随机适应放置调整(初赛专用)
    void cross(Problem &problem);//交叉
    void variation(Problem &problem);// 变异
};


#endif //ECS_GA_DISTRIBUTE_H
