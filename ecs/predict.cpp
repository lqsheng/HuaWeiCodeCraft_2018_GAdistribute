#include "predict.h"
#include <cfloat>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctgmath>
#include <cmath>

#include "Flavor.h"
#include "Host.h"
#include "Problem.h"
#include "DataContainer.h"
#include "PredictResult.h"
#include "DistributeResult.h"
#include "GA_Distribute.h"
#include "Matrix.h"

using namespace std;


//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
    Problem problem(info);//构造问题
    problem.Problem_printf();
    DataContainer data_container(data,data_num,problem.v_flavors_info);//解析训练数据
    data_container.data_smooth3();
    data_container.DataPrintf();


    //TODO 预测
    PredictResult predictResult(0,problem);//构造预测结果
    predictResult.flavor_nums=problem.v_flavors_info;
    for(int i=0;i<problem.v_flavors_info.size();i++)
    {
        predictResult.flavor_nums[i].count=200;
        predictResult.totalnums+=200;
    }


    //TODO 根据预测结果和问题描述进行资源分配
    GA_Distribute ga_distribute(100,100,0.3,0.15);//无前面的adjust82.644,加上前面adjust84.127
    DistributeResult distributeResultGA=ga_distribute.run(problem,predictResult);
    distributeResultGA.result_put_in_order();


    // 需要输出的内容
    stringstream res;
    res<<predictResult.totalnums<<"\n";
    for(auto flavor:predictResult.flavor_nums)
        res<<flavor.name<<" "<<flavor.count<<"\n";
    if(distributeResultGA.normal_host_num>0){
        res<<"\nGeneral "<<distributeResultGA.normal_host_num<<"\n";
        for(int i=0;i<distributeResultGA.distribute_res_normal.size();i++)
        {
            res<<"General-"<<i+1<<" ";
            //res<<distributeResultGA.distribute_res_normal[i].compute_res_used_rate()<<" ";
            //if(distributeResultGA.distribute_res_normal[i].compute_res_used_rate()<0.9)return;
            for(auto flavor:distributeResultGA.distribute_res_normal[i].flavor_index)
            {
                res<<flavor.name<<" "<<flavor.count<<" ";
            }
            res<<"\n";
        }
    }
    if(distributeResultGA.mem_host_num>0){
        res<<"\nLarge-Memory "<<distributeResultGA.mem_host_num<<"\n";
        for(int i=0;i<distributeResultGA.distribute_res_mem.size();i++)
        {
            res<<"Large-Memory-"<<i+1<<" ";
            //res<<distributeResultGA.distribute_res_mem[i].compute_res_used_rate()<<" ";
            //if(distributeResultGA.distribute_res_mem[i].compute_res_used_rate()<0.9)return;
            for(auto flavor:distributeResultGA.distribute_res_mem[i].flavor_index)
            {
                res<<flavor.name<<" "<<flavor.count<<" ";
            }
            res<<"\n";
        }
    }

    if(distributeResultGA.cpu_host_num>0){
        res<<"\nHigh-Performance "<<distributeResultGA.cpu_host_num<<"\n";
        for(int i=0;i<distributeResultGA.distribute_res_cpu.size();i++)
        {
            res<<"High-Performance-"<<i+1<<" ";
            //res<<distributeResultGA.distribute_res_cpu[i].compute_res_used_rate()<<" ";
            //if(distributeResultGA.distribute_res_cpu[i].compute_res_used_rate()<0.9)return;
            for(auto flavor:distributeResultGA.distribute_res_cpu[i].flavor_index)
            {
                res<<flavor.name<<" "<<flavor.count<<" ";
            }
            res<<"\n";
        }
    }

    cout<<"********************distribute result file start**********************"<<endl;
    cout<<res.str().c_str();
    cout<<"********************distribute result file end**********************"<<endl;
    clock_t programTime = clock();
    cout << "Totle Time : " <<(double)programTime/ CLOCKS_PER_SEC << "s" << endl;

    double source_rate=distributeResultGA.compute_res_used_rate(predictResult);
    cout<<"source_rate="<<source_rate<<endl;
//    if(source_rate<0.99)
//        return;

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(res.str().c_str(), filename);
}