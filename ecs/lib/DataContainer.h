//
// Created by sheng on 18-5-1.
//

#ifndef ECS_DATACONTAINER_H
#define ECS_DATACONTAINER_H

#include "Flavor.h"
#include "Host.h"
#include "Problem.h"

time_t convert(int year,int month,int day);//底层函数,无需关注
int get_days(const char* from, const char* to,bool need_increase=false);//底层函数,计算两个日期间的间隔天数

//训练数据描述类
class DataContainer
{
public:
    std::string start_time,end_time;//训练数据起止时间
    std::vector<std::vector<int> > v_requests;//训练数据起止时间内每一天各类型虚拟机的申请数量
    int total_days;//训练数据跨越的天数
    int flavor_types;//需考虑的虚拟机类型总数
    DataContainer(char **const data,int data_num,std::vector<Flavor> v_flavors_info);//读入并解析训练数据
    //均值滤波
    void data_smooth();
    //核弹,不得外泄
    void data_smooth2();
    //核弹,不得外泄
    void data_smooth3();
    //核弹,不得外泄
    void data_smooth_chusai();
    //核弹,不得外泄
    void data_smooth4();
    //核弹,不得外泄
    void data_smooth_3sigma();
    //训练数据打印
    void DataPrintf();
};


#endif //ECS_DATACONTAINER_H
