//
// Created by sheng on 18-5-1.
//

#include "GA_Distribute.h"

using std::cout;
using std::endl;
using std::vector;

GA_Distribute::GA_Distribute(int max_generation_,int group_num_,float cross_rate_, float varia_rate_)//构造遗传优化问题
{
    group_num=group_num_;
    cross_rate=cross_rate_;
    varia_rate=varia_rate_;
    max_generation=max_generation_;
    cout<<"*******GA_Distribute init done*******"<<endl;
}
DistributeResult GA_Distribute::run(Problem &problem,PredictResult &predictResult)//迭代进化
{
    initEntities(problem,predictResult);//初始化种群,产生足够的初始可行解集合
    cout<<"*******initEntities done*******"<<endl;
    for(int i=0;i<max_generation;i++)//进化
    {
        computeFitness(problem.serverNumCPU,problem.serverNumMEM,predictResult);//计算适应度
        recordBestEntity();//记录最优个体
        select();//自然选择
        cross(problem);//交叉
        variation(problem);//变异
        clock_t programTime = clock();//超时则停止进化,输出当前最优个体
        if((double)programTime/ CLOCKS_PER_SEC>70)break;
    }
    cout<<"*******GA_Distribute done*******"<<endl;
    return bestEntity.distributeResult;
}

void GA_Distribute::initEntities(Problem &problem,PredictResult &predictResult)//生成初始种群
{
    int flavor_order=0;
    for(auto flavor_type:predictResult.flavor_nums){//为每一台虚拟机编号,保证唯一性
        for(int i=0;i<flavor_type.count;i++){
            flavor_order++;
            predict_result_for_GA.push_back(flavor_type);
            predict_result_for_GA.back().count=flavor_order;
        }
    }
    cout<<"*******predict_result_for_GA deal done*******"<<endl;
    for(int i=0;i<group_num;i++){//随机生成初始种群
        Entity entity;
        entity.distributeResult.GreedyFF_Distribute_for_GA(problem,predict_result_for_GA);
        entities.push_back(entity);
    }
    if(problem.optimize_type=="CPU")optimzeCPU=true;
    else optimzeCPU=false;
    serverNumCPU=problem.serverNumCPU;
    serverNumMEM=problem.serverNumMEM;
}
void GA_Distribute::computeFitness(int cpu_lim,int mem_lim,PredictResult &predictResult)//计算适应度
{
    float c=optimzeCPU? cpu_lim:mem_lim;
    for(auto &entity:entities){//每个个体
        float numerator=0;
        for(auto box:entity.distributeResult.distribute_result)//每个箱子
        {

            numerator+=(box.compute_res_used_rate())*(box.compute_res_used_rate());
        }
        entity.fitness=numerator/entity.distributeResult.distribute_result.size();
    }
}
void GA_Distribute::recordBestEntity()
{
    stable_sort(entities.begin(),entities.end(),entitySort);
    Entity &en = entities.back();
    if(en.fitness > bestEntity.fitness)
    {
        bestEntity = en;
    }
    else
        entities.back()=bestEntity;//保留最优个体
}
void GA_Distribute::select()// 选择
{
    vector<Entity>  new_entities;
    // 保留群体中最优的前10%
    int reserve_num = (int)(group_num * 0.1);
    for (int i=0; i<reserve_num; i++)
    {
        new_entities.push_back( entities.back() );
        entities.pop_back();
    }
    // 计算剩余个体的累积概率
    std::vector<float>  selected_rate;
    float fitness_sum =0;
    for(auto entity:entities)fitness_sum+=entity.fitness;
    selected_rate.push_back(entities[0].fitness / fitness_sum);
    for(size_t i = 1; i < entities.size(); i++)
    {
        float cur_rate = selected_rate.back() + (entities[i].fitness / fitness_sum);
        selected_rate.push_back(cur_rate);
    }
    // 利用轮赌法选择剩下的40%
    std::uniform_real_distribution<float> random_mode(0.0, 1.0);
    int left_num = (int)(group_num/2)-reserve_num;
    for(int i = 0; i < left_num; i++)
    {
        float rand_rate = random_mode(random_engine);
        for (size_t idx = 0; idx < selected_rate.size(); idx++)
        {
            if(rand_rate <= selected_rate[idx])
            {
                new_entities.push_back(entities[idx]);
                break;
            }
        }
    }
    // 新群体赋值
    entities.clear();
    entities = new_entities;
}
void GA_Distribute::GreedyFF_adjust(Problem &problem,Entity &entity,vector<Flavor> flavor_rest)//贪心放置调整(复赛专用)
{
    vector<Flavor> flavor_for_newhost;
    for(auto flavor:flavor_rest)//在现有主机里进行贪心放置
    {
        bool need_new_host=true;
        int minum_host_index=0;
        float rate_max=0;
        minum_host_index=0;
        for(int j=0;j<entity.distributeResult.distribute_result.size();j++)//寻找是否有可以放下的主机并记录利用率最大的主机
        {
            if((entity.distributeResult.distribute_result[j].compute_res_used_rate()>rate_max)&&
               (entity.distributeResult.distribute_result[j].cpu_rest >=flavor.numCPU)&&
               (entity.distributeResult.distribute_result[j].mem_rest >=flavor.numMEM))
            {
                rate_max=entity.distributeResult.distribute_result[j].compute_res_used_rate();
                minum_host_index=j;
                need_new_host=false;
            }
        }
        if(need_new_host)
        {
            flavor_for_newhost.push_back(flavor);
        }
        else
        {
            entity.distributeResult.distribute_result[minum_host_index].put_flavor(flavor);
        }
    }
    vector<bool>flavor_putted;
    for(auto flavor:flavor_for_newhost)flavor_putted.push_back(false);
    for(int i=0;i<flavor_for_newhost.size();)//放置
    {
        bool flavor_used[problem.host_types][flavor_for_newhost.size()];
        for(int k=0;k<problem.host_types;k++) {
            for (int j = 0; j < flavor_for_newhost.size(); j++) {
                flavor_used[k][j]=flavor_putted[j];
            }
        }
        Host host[problem.host_types];
        float host_res_rate[problem.host_types];
        for(int j=0;j<problem.host_types;j++)
        {
            host[j]=problem.v_hosts_info[j];
            for(int k=i;k<flavor_for_newhost.size();k++)
            {
                if(flavor_used[j][k])continue;
                if(host[j].put_flavor(flavor_for_newhost[k]))
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
        for(int j=i;j<flavor_for_newhost.size();j++)
        {
            if(flavor_used[host_choose][j])
                flavor_putted[j]=true;
            else i_increase=false;
            if(i_increase)i++;
        }
        if(host_res_rate[host_choose]>0)
            entity.distributeResult.distribute_result.push_back(host[host_choose]);
    }
}
void GA_Distribute::FF_adjust(Entity &entity,vector<Flavor> flavor_rest)//首次适应放置调整(初赛专用)
{
    vector<int> v_cpu_rest;
    vector<int> v_mem_rest;
    for(int i=0;i<entity.distributeResult.flavor_distribute_result.size();i++) {
        int cpu_rest = serverNumCPU;//初始化剩余CPU数
        int mem_rest = serverNumMEM;//初始化剩余内存
        for (auto flavor:entity.distributeResult.flavor_distribute_result[i]) {
            cpu_rest -= flavor.numCPU;
            mem_rest -= flavor.numMEM;
        }
        v_cpu_rest.push_back(cpu_rest);
        v_mem_rest.push_back(mem_rest);
    }
    for(auto flavor:flavor_rest)//进行first fit 放置
    {
        bool need_new_host=true;
        int minum_host_index=0;
        if(optimzeCPU)
        {
            minum_host_index=0;
            for(int j=0;j<entity.distributeResult.total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((v_cpu_rest[j] >=flavor.numCPU)&&
                   (v_mem_rest[j] >=flavor.numMEM))
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
            for(int j=0;j<entity.distributeResult.total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((v_cpu_rest[j] >=flavor.numCPU)&&
                   (v_mem_rest[j] >=flavor.numMEM))
                {
                    minum_host_index=j;
                    need_new_host=false;
                    break;
                }
            }
        }
        if(need_new_host)
        {
            entity.distributeResult.total_hosts++;
            v_cpu_rest.push_back(serverNumCPU);
            v_mem_rest.push_back(serverNumMEM);
            vector<Flavor> distribute_method;//对应这台主机的分配方案
            entity.distributeResult.flavor_distribute_result.push_back(distribute_method);//保存分配方案
            v_cpu_rest[entity.distributeResult.total_hosts-1]-=flavor.numCPU;
            v_mem_rest[entity.distributeResult.total_hosts-1]-=flavor.numMEM;
            entity.distributeResult.flavor_distribute_result[entity.distributeResult.total_hosts-1].push_back(flavor);
        }
        else
        {
            v_cpu_rest[minum_host_index]-=flavor.numCPU;
            v_mem_rest[minum_host_index]-=flavor.numMEM;
            entity.distributeResult.flavor_distribute_result[minum_host_index].push_back(flavor);
        }
    }
}
void GA_Distribute::RF_adjust(Entity &entity,vector<Flavor> flavor_rest)//随机适应放置调整(初赛专用)
{
    vector<int> v_cpu_rest;
    vector<int> v_mem_rest;
    for(int i=0;i<entity.distributeResult.flavor_distribute_result.size();i++) {
        int cpu_rest = serverNumCPU;//初始化剩余CPU数
        int mem_rest = serverNumMEM;//初始化剩余内存
        for (auto flavor:entity.distributeResult.flavor_distribute_result[i]) {
            cpu_rest -= flavor.numCPU;
            mem_rest -= flavor.numMEM;
        }
        v_cpu_rest.push_back(cpu_rest);
        v_mem_rest.push_back(mem_rest);
    }
    for(auto flavor:flavor_rest)//进行random fit 放置
    {
        bool need_new_host=true;
        int host_chosing_index=0;
        vector<int> host_candidates;
        if(optimzeCPU)
        {
            for(int j=0;j<entity.distributeResult.total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((v_cpu_rest[j] >=flavor.numCPU)&&
                   (v_mem_rest[j] >=flavor.numMEM))
                {
                    need_new_host=false;
                    host_candidates.push_back(j);
                }
            }
        }
        else
        {
            for(int j=0;j<entity.distributeResult.total_hosts;j++)//寻找是否有可以放下的主机
            {
                if((v_cpu_rest[j] >=flavor.numCPU)&&
                   (v_mem_rest[j] >=flavor.numMEM))
                {
                    host_candidates.push_back(j);
                    need_new_host=false;
                }
            }
        }
        if(need_new_host)
        {
            entity.distributeResult.total_hosts++;
            v_cpu_rest.push_back(serverNumCPU);
            v_mem_rest.push_back(serverNumMEM);
            vector<Flavor> distribute_method;//对应这台主机的分配方案
            entity.distributeResult.flavor_distribute_result.push_back(distribute_method);//保存分配方案
            v_cpu_rest[entity.distributeResult.total_hosts-1]-=flavor.numCPU;
            v_mem_rest[entity.distributeResult.total_hosts-1]-=flavor.numMEM;
            entity.distributeResult.flavor_distribute_result[entity.distributeResult.total_hosts-1].push_back(flavor);
        }
        else
        {
            std::uniform_int_distribution<int> host_index_random(0,host_candidates.size()-1);
            host_chosing_index=host_candidates[host_index_random(random_engine)];
            v_cpu_rest[host_chosing_index]-=flavor.numCPU;
            v_mem_rest[host_chosing_index]-=flavor.numMEM;
            entity.distributeResult.flavor_distribute_result[host_chosing_index].push_back(flavor);
        }
    }
}
void GA_Distribute::cross(Problem &problem)//交叉
{
    std::bernoulli_distribution random_cross(cross_rate);
    size_t src_group_num = entities.size();
    for(int i=0;i<src_group_num-1;i+=2)
    {
        if(random_cross(random_engine))//发生交叉
        {
            Entity father=entities[i];
            Entity mother=entities[i+1];
            std::uniform_int_distribution<int> father_cross_pattern
                    (0,father.distributeResult.distribute_result.size()-1);
            std::uniform_int_distribution<int> mother_cross_pattern
                    (0,mother.distributeResult.distribute_result.size()-1);
            int father_cross_pos1=father_cross_pattern(random_engine);//随机选定两个交叉位置
            int father_cross_pos2=father_cross_pattern(random_engine);
            int tmp=father_cross_pos1;
            if(father_cross_pos1>father_cross_pos2){
                father_cross_pos1=father_cross_pos2;
                father_cross_pos2=tmp;
            }
            int mother_cross_pos1=mother_cross_pattern(random_engine);
            int mother_cross_pos2=mother_cross_pattern(random_engine);
            tmp=mother_cross_pos1;
            if(mother_cross_pos1>mother_cross_pos2){
                mother_cross_pos1=mother_cross_pos2;
                mother_cross_pos2=tmp;
            }
            vector<Host> father_cross,mother_cross;
            //**************取出待交叉片段***********
            for(int j=father_cross_pos1;j<father_cross_pos2;j++)
                father_cross.push_back(father.distributeResult.distribute_result[j]);
            for(int j=mother_cross_pos1;j<mother_cross_pos2;j++)
                mother_cross.push_back(mother.distributeResult.distribute_result[j]);
            //**************消去待插入母基因中的重复片段***********
            vector<Flavor> mother_rest;
            for(auto box:father_cross)//检查交叉部分基因中的没一个箱子
            {
                for(auto flavor_cross:box.flavor_index)//对于箱子里装的虚拟机
                {
                    bool alread_erased=false;
                    for(int i=0;i<mother_rest.size();i++)//查看是不是在之前的操作中已经被消除了
                    {
                        if(mother_rest[i].count==flavor_cross.count)
                        {
                            alread_erased=true;
                            mother_rest.erase(mother_rest.begin()+i);
                        }
                    }
                    if(alread_erased==false) {//还没被消除,需要寻找包含他的箱子
                        for (int i = 0; i < mother.distributeResult.distribute_result.size(); i++)//考虑其中一个箱子
                        {
                            bool need_erase = false;
                            for (auto flavor:mother.distributeResult.distribute_result[i].flavor_index) {
                                if (flavor.count == flavor_cross.count) {//这个箱子中存在待交叉基因片段,因而需要被消除
                                    need_erase = true;
                                    break;
                                }
                            }
                            if (need_erase) {
                                for (auto flavor:mother.distributeResult.distribute_result[i].flavor_index) {
                                    if (flavor.count != flavor_cross.count) {
                                        mother_rest.push_back(flavor);
                                    }
                                }
                                if(i<mother_cross_pos1)mother_cross_pos1--;
                                mother.distributeResult.distribute_result.erase
                                        (mother.distributeResult.distribute_result.begin() + i);
                                i--;
                            }
                        }
                    }
                }

            }
            //**************插入基因***********
            for(int i=father_cross.size()-1;i>-1;i--)
                mother.distributeResult.distribute_result.insert(mother.distributeResult.
                        distribute_result.begin()+mother_cross_pos1,father_cross[i]);
            GreedyFF_adjust(problem,mother,mother_rest);
            entities.push_back(mother);
            //**************消去待插入基因中的重复片段***********
            vector<Flavor> father_rest;
            for(auto box:mother_cross)
            {
                for(auto flavor_cross:box.flavor_index)
                {
                    bool alread_erased=false;
                    for(int i=0;i<father_rest.size();i++)
                    {
                        if(father_rest[i].count==flavor_cross.count)
                        {
                            alread_erased=true;
                            father_rest.erase(father_rest.begin()+i);
                        }
                    }
                    if(alread_erased==false) {
                        for (int i = 0; i < father.distributeResult.distribute_result.size(); i++)//考虑其中一个箱子
                        {
                            bool need_erase = false;
                            for (auto flavor:father.distributeResult.distribute_result[i].flavor_index) {
                                if (flavor.count == flavor_cross.count) {
                                    need_erase = true;
                                    break;
                                }
                            }
                            if (need_erase) {
                                for (auto flavor:father.distributeResult.distribute_result[i].flavor_index) {
                                    if (flavor.count != flavor_cross.count) {
                                        father_rest.push_back(flavor);
                                    }
                                }
                                if(i<father_cross_pos1)father_cross_pos1--;
                                father.distributeResult.distribute_result.erase
                                        (father.distributeResult.distribute_result.begin() + i);
                                i--;
                            }
                        }
                    }
                }

            }
            //**************插入基因***********
            for(int i=mother_cross.size()-1;i>-1;i--)
                father.distributeResult.distribute_result.insert(father.distributeResult.
                        distribute_result.begin()+father_cross_pos1,mother_cross[i]);
            GreedyFF_adjust(problem,father,father_rest);
            entities.push_back(father);
        }
        else//不交叉
        {
            entities.push_back(entities[i]);
            entities.push_back(entities[i+1]);
        }
    }
}

void GA_Distribute::variation(Problem &problem)// 变异
{
    std::bernoulli_distribution random_varia(varia_rate);
    for(size_t i = 0; i < entities.size(); i++)
    {
        if(random_varia(random_engine))
        {
            Entity &en = entities[i];
            std::uniform_int_distribution<int> varia_pattern
                    (0,en.distributeResult.distribute_result.size()-1);
            int varia_pos=varia_pattern(random_engine);//随机选定变异位置
            vector<Flavor> flavor_rest=en.distributeResult.distribute_result[varia_pos].flavor_index;
            en.distributeResult.distribute_result.erase
                    (en.distributeResult.distribute_result.begin() + varia_pos);
            GreedyFF_adjust(problem,en,flavor_rest);
        }
    }
}