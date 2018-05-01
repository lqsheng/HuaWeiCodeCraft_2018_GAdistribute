//
// Created by sheng on 18-5-1.
//

#include "DistributeResult.h"

using std::vector;
using std::cout;
using std::endl;

void DistributeResult::FFD_Distribute(const Problem &problem,const PredictResult &predictResult)//降序首次适应分配算法(初赛专用)
{
    PredictResult result_for_distribute=predictResult;
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    total_hosts=0;
    while (result_for_distribute.totalnums>0)//在分配完之前循环执行以下代码
    {
        total_hosts+=1;//新增一台物理主机
        int cpu_rest=problem.serverNumCPU;//初始化剩余CPU数
        int mem_rest=problem.serverNumMEM;//初始化剩余内存
        vector<Flavor> distribute_method;//对应这台主机的分配方案
        for(int i=0;i<result_for_distribute.flavor_nums.size();i++)//在一台主机里从大到小放置虚拟机
        {
            int count=0;
            while ((cpu_rest >=result_for_distribute.flavor_nums[i].numCPU)
                   &&(mem_rest >=result_for_distribute.flavor_nums[i].numMEM)
                   &&(result_for_distribute.flavor_nums[i].count>0))//可容纳当前型号虚拟机则执行放置
            {
                count++;
                cpu_rest-=result_for_distribute.flavor_nums[i].numCPU;
                mem_rest-=result_for_distribute.flavor_nums[i].numMEM;
                result_for_distribute.flavor_nums[i].count--;
                result_for_distribute.totalnums--;
            }
            if(count>0)//发生了放置行为则将此规格虚拟机规格数量保存进这台主机的分配方案
            {
                Flavor flavor=result_for_distribute.flavor_nums[i];
                flavor.count=count;
                distribute_method.push_back(flavor);
            }
        }
        flavor_distribute_result.push_back(distribute_method);//这台物理机剩余空间已经放不下任何一台虚拟机了，保存分配方案
    }
}
void DistributeResult::BFD_Distribute(const Problem &problem,const PredictResult &predictResult)//降序最佳适应分配算法(初赛专用)
{
    PredictResult result_for_distribute=predictResult;
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    total_hosts=1;
    vector<int> cpu_rest;
    vector<int> mem_rest;
    cpu_rest.push_back(problem.serverNumCPU);
    mem_rest.push_back(problem.serverNumMEM);
    vector<Flavor> distribute_method;//对应这台主机的分配方案
    flavor_distribute_result.push_back(distribute_method);//保存分配方案
    for(int i=0;i<result_for_distribute.flavor_nums.size();i++)//在一台主机里从大到小放置虚拟机
    {
        while (result_for_distribute.flavor_nums[i].count>0)
        {
            result_for_distribute.flavor_nums[i].count--;
            result_for_distribute.totalnums--;
            bool need_new_host=true;
            int minum_host_index=0;
            if(optimzeCPU)
            {
                int cpu_minium=problem.serverNumCPU+1;
                minum_host_index=0;
                for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
                {
                    if((cpu_rest[j]<cpu_minium)&&
                       (cpu_rest[j] >=result_for_distribute.flavor_nums[i].numCPU)&&
                       (mem_rest[j] >=result_for_distribute.flavor_nums[i].numMEM))
                    {
                        cpu_minium=cpu_rest[j];
                        minum_host_index=j;
                        need_new_host=false;
                    }
                }
            }
            else
            {
                int mem_minium=problem.serverNumMEM+1;
                minum_host_index=0;
                for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
                {
                    if((mem_rest[j]<mem_minium)&&
                       (cpu_rest[j] >=result_for_distribute.flavor_nums[i].numCPU)&&
                       (mem_rest[j] >=result_for_distribute.flavor_nums[i].numMEM))
                    {
                        mem_minium=mem_rest[j];
                        minum_host_index=j;
                        need_new_host=false;
                    }
                }
            }
            if(need_new_host)
            {
                total_hosts++;
                cpu_rest.push_back(problem.serverNumCPU);
                mem_rest.push_back(problem.serverNumMEM);
                vector<Flavor> distribute_method;//对应这台主机的分配方案
                flavor_distribute_result.push_back(distribute_method);//保存分配方案
                cpu_rest[total_hosts-1]-=result_for_distribute.flavor_nums[i].numCPU;
                mem_rest[total_hosts-1]-=result_for_distribute.flavor_nums[i].numMEM;
                Flavor flavor = predictResult.flavor_nums[i];
                flavor.count = 1;
                flavor_distribute_result[total_hosts-1].push_back(flavor);
            }
            else
            {
                cpu_rest[minum_host_index]-=result_for_distribute.flavor_nums[i].numCPU;
                mem_rest[minum_host_index]-=result_for_distribute.flavor_nums[i].numMEM;
                bool flavor_exist = false;
                for (int k = 0; k < flavor_distribute_result[minum_host_index].size(); k++) {
                    if (flavor_distribute_result[minum_host_index][k].name == predictResult.flavor_nums[i].name) {
                        flavor_distribute_result[minum_host_index][k].count ++;
                        flavor_exist = true;
                    }
                }
                if (flavor_exist == false) {
                    Flavor flavor = predictResult.flavor_nums[i];
                    flavor.count = 1;
                    flavor_distribute_result[minum_host_index].push_back(flavor);
                }
            }
        }
    }
}
void DistributeResult::BF_Distribute_for_GA(const Problem &problem,vector<Flavor> &predictResult)//最佳适应分配算法,为遗传算法初始化种群使用(初赛专用)
{
    random_shuffle (predictResult.begin(), predictResult.end());//先随机打乱虚拟机顺序
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    total_hosts=1;
    vector<int> cpu_rest;
    vector<int> mem_rest;
    cpu_rest.push_back(problem.serverNumCPU);
    mem_rest.push_back(problem.serverNumMEM);
    vector<Flavor> distribute_method;//对应这台主机的分配方案
    flavor_distribute_result.push_back(distribute_method);//保存分配方案
    for(int i=0;i<predictResult.size();i++)//Best fit 放置
    {
        bool need_new_host=true;
        int minum_host_index=0;
        if(optimzeCPU)
        {
            int cpu_minium=problem.serverNumCPU+1;
            minum_host_index=0;
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((cpu_rest[j]<cpu_minium)&&
                   (cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    cpu_minium=cpu_rest[j];
                    minum_host_index=j;
                    need_new_host=false;
                }
            }
        }
        else
        {
            int mem_minium=problem.serverNumMEM+1;
            minum_host_index=0;
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((mem_rest[j]<mem_minium)&&
                   (cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    mem_minium=mem_rest[j];
                    minum_host_index=j;
                    need_new_host=false;
                }
            }
        }
        if(need_new_host)
        {
            total_hosts++;
            cpu_rest.push_back(problem.serverNumCPU);
            mem_rest.push_back(problem.serverNumMEM);
            vector<Flavor> distribute_method;//对应这台主机的分配方案
            flavor_distribute_result.push_back(distribute_method);//保存分配方案
            cpu_rest[total_hosts-1]-=predictResult[i].numCPU;
            mem_rest[total_hosts-1]-=predictResult[i].numMEM;
            Flavor flavor = predictResult[i];
            //flavor.count = 1;
            flavor_distribute_result[total_hosts-1].push_back(flavor);
        }
        else
        {
            cpu_rest[minum_host_index]-=predictResult[i].numCPU;
            mem_rest[minum_host_index]-=predictResult[i].numMEM;
//                    bool flavor_exist = false;
//                    for (int k = 0; k < flavor_distribute_result[minum_host_index].size(); k++) {
//                        if (flavor_distribute_result[minum_host_index][k].name == predictResult[i].name) {
//                            flavor_distribute_result[minum_host_index][k].count ++;
//                            flavor_exist = true;
//                        }
//                    }
//                    if (flavor_exist == false) {
            Flavor flavor = predictResult[i];
//                        flavor.count = 1;
            flavor_distribute_result[minum_host_index].push_back(flavor);
//                    }
        }
    }
}
void DistributeResult::FF_Distribute_for_GA(const Problem &problem,vector<Flavor> &predictResult)//首次适应分配算法,为遗传算法初始化种群使用(初赛专用)
{
    random_shuffle (predictResult.begin(), predictResult.end());
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    total_hosts=1;
    vector<int> cpu_rest;
    vector<int> mem_rest;
    cpu_rest.push_back(problem.serverNumCPU);
    mem_rest.push_back(problem.serverNumMEM);
    vector<Flavor> distribute_method;//对应这台主机的分配方案
    flavor_distribute_result.push_back(distribute_method);//保存分配方案
    for(int i=0;i<predictResult.size();i++)//Best fit 放置
    {
        bool need_new_host=true;
        int minum_host_index=0;
        if(optimzeCPU)
        {
            minum_host_index=0;
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    minum_host_index=j;
                    need_new_host=false;
                    break;
                }
            }
        }
        else
        {
            minum_host_index=0;
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    minum_host_index=j;
                    need_new_host=false;
                    break;
                }
            }
        }
        if(need_new_host)
        {
            total_hosts++;
            cpu_rest.push_back(problem.serverNumCPU);
            mem_rest.push_back(problem.serverNumMEM);
            vector<Flavor> distribute_method;//对应这台主机的分配方案
            flavor_distribute_result.push_back(distribute_method);//保存分配方案
            cpu_rest[total_hosts-1]-=predictResult[i].numCPU;
            mem_rest[total_hosts-1]-=predictResult[i].numMEM;
            Flavor flavor = predictResult[i];
            flavor_distribute_result[total_hosts-1].push_back(flavor);
        }
        else
        {
            cpu_rest[minum_host_index]-=predictResult[i].numCPU;
            mem_rest[minum_host_index]-=predictResult[i].numMEM;
            Flavor flavor = predictResult[i];
            flavor_distribute_result[minum_host_index].push_back(flavor);
        }
    }
}
void DistributeResult::RF_Distribute_for_GA(const Problem &problem,vector<Flavor> &predictResult)//随机适应分配算法,为遗传算法初始化种群使用(初赛专用)
{
    random_shuffle (predictResult.begin(), predictResult.end());
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    total_hosts=1;
    vector<int> cpu_rest;
    vector<int> mem_rest;
    cpu_rest.push_back(problem.serverNumCPU);
    mem_rest.push_back(problem.serverNumMEM);
    vector<Flavor> distribute_method;//对应这台主机的分配方案
    flavor_distribute_result.push_back(distribute_method);//保存分配方案
    for(int i=0;i<predictResult.size();i++)//RANDOM fit 放置
    {
        bool need_new_host=true;
        int host_chosing_index=0;
        vector<int> host_candidates;
        if(optimzeCPU)
        {
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    host_candidates.push_back(j);
                    need_new_host=false;
                }
            }
        }
        else
        {
            for(int j=0;j<total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((cpu_rest[j] >=predictResult[i].numCPU)&&
                   (mem_rest[j] >=predictResult[i].numMEM))
                {
                    host_candidates.push_back(j);
                    need_new_host=false;
                }
            }
        }
        if(need_new_host)
        {
            total_hosts++;
            cpu_rest.push_back(problem.serverNumCPU);
            mem_rest.push_back(problem.serverNumMEM);
            vector<Flavor> distribute_method;//对应这台主机的分配方案
            flavor_distribute_result.push_back(distribute_method);//保存分配方案
            cpu_rest[total_hosts-1]-=predictResult[i].numCPU;
            mem_rest[total_hosts-1]-=predictResult[i].numMEM;
            Flavor flavor = predictResult[i];
            flavor_distribute_result[total_hosts-1].push_back(flavor);
        }
        else
        {
            std::uniform_int_distribution<int> random_host_index(0,host_candidates.size()-1);
            host_chosing_index=host_candidates[random_host_index(random_engine)];
            cpu_rest[host_chosing_index]-=predictResult[i].numCPU;
            mem_rest[host_chosing_index]-=predictResult[i].numMEM;
            Flavor flavor = predictResult[i];
            flavor_distribute_result[host_chosing_index].push_back(flavor);
        }
    }
}
void DistributeResult::GreedyFF_Distribute_for_GA(const Problem &problem,vector<Flavor> &predictResult)//贪心分配算法,为遗传算法初始化种群使用(复赛专用)
{
    random_shuffle (predictResult.begin(), predictResult.end());//先随机打乱虚拟机顺序
    vector<bool>flavor_putted;
    for(auto flavor:predictResult)flavor_putted.push_back(false);
    for(int i=0;i<predictResult.size();)//放置
    {
        bool flavor_used[problem.host_types][predictResult.size()];
        for(int k=0;k<problem.host_types;k++) {
            for (int j = 0; j < predictResult.size(); j++) {
                flavor_used[k][j]=flavor_putted[j];
            }
        }
        Host host[problem.host_types];
        float host_res_rate[problem.host_types];
        for(int j=0;j<problem.host_types;j++)
        {
            host[j]=problem.v_hosts_info[j];
            for(int k=i;k<predictResult.size();k++)
            {
                if(flavor_used[j][k])continue;
                if(host[j].put_flavor(predictResult[k]))
                {
                    flavor_used[j][k]=true;
                }
            }
            host_res_rate[j]=host[j].compute_res_used_rate();
        }
        std::uniform_int_distribution<int> random_choose(0,problem.host_types-1);
        int host_choose=random_choose(random_engine);//随机给予优先选中权
        for(int j=0;j<problem.host_types;j++)//然后靠实力吃饭
        {
            if(host_res_rate[j]>host_res_rate[host_choose])
                host_choose=j;
        }
        bool i_increase=true;
        for(int j=i;j<predictResult.size();j++)
        {
            if(flavor_used[host_choose][j])
                flavor_putted[j]=true;
            else i_increase=false;
            if(i_increase)i++;
        }
        if(host_res_rate[host_choose]>0)
            distribute_result.push_back(host[host_choose]);
    }
}
void DistributeResult::result_put_in_order()//整理分配结果
{
    normal_host_num=0;
    cpu_host_num=0;
    mem_host_num=0;
    distribute_res_normal.clear();
    distribute_res_mem.clear();
    distribute_res_cpu.clear();
    for(auto host:distribute_result)
    {
        Host host_in_order(host.numCPU,host.numMEM,host.name,host.cpu_rest,host.mem_rest);
        for(auto &flavor:host.flavor_index)
        {
            if(host_in_order.flavor_index.empty())
            {
                flavor.count=1;
                host_in_order.flavor_index.push_back(flavor);
            }
            else{
                bool flavor_exist = false;
                for(int i=0;i<host_in_order.flavor_index.size();i++)
                {
                    if (flavor.name == host_in_order.flavor_index[i].name) {
                        host_in_order.flavor_index[i].count++;
                        flavor_exist = true;
                        break;
                    }
                }
                if (flavor_exist == false) {
                    flavor.count = 1;
                    host_in_order.flavor_index.push_back(flavor);
                }
            }
        }
        if(host_in_order.name=="General"){
            distribute_res_normal.push_back(host_in_order);
            normal_host_num++;
        }
        else if(host_in_order.name=="Large-Memory"){
            distribute_res_mem.push_back(host_in_order);
            mem_host_num++;
        }
        else{
            distribute_res_cpu.push_back(host_in_order);
            cpu_host_num++;
        }
    }
}
//
void DistributeResult::adjust(const Problem &problem,PredictResult &predictResult, bool adjust_last_host,float rest_rate_limit)
{}
//
void DistributeResult::adjust_for_repecharge(const Problem &problem,PredictResult &predictResult, bool adjust_last_host,float rest_rate_limit)
{}

void DistributeResult::adjust_greedy_exchange(const Problem &problem,PredictResult &predictResult)
{}

float DistributeResult::compute_res_used_rate(const PredictResult &predictResult)//计算资源利用率
{
    double source_rate=0;
    double b1=0,b2=0;
    double a1=0,a2=0;
    if(normal_host_num>0)
        b1+=normal_host_num*distribute_res_normal[0].numCPU;
    if(cpu_host_num>0)
        b1+=cpu_host_num*distribute_res_cpu[0].numCPU;
    if(mem_host_num>0)
        b1+=mem_host_num*distribute_res_mem[0].numCPU;
    a1=0;
    for(auto flavor:predictResult.flavor_nums)
    {
        a1+=flavor.count*flavor.numCPU;
    }
    if(normal_host_num>0)
        b2+=normal_host_num*distribute_res_normal[0].numMEM;
    if(cpu_host_num>0)
        b2+=cpu_host_num*distribute_res_cpu[0].numMEM;
    if(mem_host_num>0)
        b2+=mem_host_num*distribute_res_mem[0].numMEM;
    a2=0;
    for(auto flavor:predictResult.flavor_nums)
    {
        a2+=flavor.count*flavor.numMEM;
    }

    source_rate=(a1/b1+a2/b2)/2;
    return source_rate;
}
void DistributeResult::printfDistributeResult(void)//打印分配结果
{
    cout<<"********************distribute result start**********************"<<endl;
    cout<<"totalhosts="<<total_hosts<<endl;
    for(int i=0;i<flavor_distribute_result.size();i++)
    {
        cout<<i<<" ";
        for(auto flavor:flavor_distribute_result[i])
        {
            cout<<flavor.name<<" "<<flavor.count<<" ";
        }
        cout<<endl;
    }
    cout<<"********************distribute result end**********************"<<endl;
}
