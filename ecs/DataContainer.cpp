//
// Created by sheng on 18-5-1.
//

#include <stdio.h>
#include <algorithm>
#include <ctgmath>
#include <cmath>
#include "DataContainer.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::istringstream;

//底层函数,无需关注
time_t convert(int year,int month,int day)
{
    tm info={0};
    info.tm_year=year-1900;
    info.tm_mon=month-1;
    info.tm_mday=day;
    return mktime(&info);
}
//底层函数,计算两个日期间的间隔天数
int get_days(const char* from, const char* to,bool need_increase)
{
    int year,month,day;
    sscanf(from,"%d-%d-%d",&year,&month,&day);
    int fromSecond=(int)convert(year,month,day);
    sscanf(to,"%d-%d-%d",&year,&month,&day);
    int toSecond=(int)convert(year,month,day);
    if(need_increase)
        return ((toSecond-fromSecond)/24/3600)+1;
    else
        return ((toSecond-fromSecond)/24/3600);
}

DataContainer::DataContainer(char **const data, int data_num, std::vector<Flavor> v_flavors_info)
{
    total_days=1;
    start_time="";
    string last_time="";
    flavor_types=v_flavors_info.size();
    vector<int> flavor_nums_init;
    for(int i=0;i<v_flavors_info.size();i++)flavor_nums_init.push_back(0);
    v_requests.push_back(flavor_nums_init);
    for(int i=0;i<data_num;i++)//对每行数据进行归类处理
    {
        istringstream data_line(data[i]);
        string flavor_type_name,time;
        data_line>>flavor_type_name>>flavor_type_name>>time;//得到本条数据的虚拟机类型名和申请时间
        if(start_time=="")start_time=time;//得到训练数据起始时间
        else if((time!=last_time)&&(last_time!=""))//本条数据与前一条数据不在同一天
        {
            int days=get_days(last_time.c_str(),time.c_str());//获取间隔天数
            total_days+=days;//总天数对应增加
            for(int i=0;i<days;i++)//间隔期间每种规格的虚拟机申请数量均为0
            {
                vector<int> flavor_nums;
                for (int i = 0; i < v_flavors_info.size(); i++)flavor_nums.push_back(0);
                v_requests.push_back(flavor_nums);
            }
        }
        last_time=time;//更新历史时间
        int flavor_type=0;//以下代码是判断此条数据是否需要考虑,以及对应的虚拟机类型
        bool valid=false;
        for(;flavor_type<v_flavors_info.size();flavor_type++)
        {
            if(flavor_type_name==v_flavors_info[flavor_type].name)
            {
                valid= true;
                break;
            }
        }
        if(valid)//需要考虑,则将当天对应的虚拟机申请数量加1
        {
            v_requests[total_days-1][flavor_type]++;
        }

    }
    end_time=last_time;//得到训练数据结束时间
}

void DataContainer::DataPrintf()
{
    cout<<"********************data_file_start**********************"<<endl;
    cout<<"start_time="<<start_time<<" end_time="<<end_time<<" total_days="<<total_days<<endl;
    for(int i=0;i<v_requests.size();i++)
    {
        cout<<"day["<<i<<"]=";
        for(int j=0;j<v_requests[i].size();j++)
        {
            cout<<v_requests[i][j]<<",";
        }
        cout<<endl;
    }
    cout<<"********************data_file_end**********************"<<endl;
}

//均值滤波
void DataContainer::data_smooth()
{}
//核弹,不得外泄
void DataContainer::data_smooth2()
{}
//核弹,不得外泄
void DataContainer::data_smooth3()
{}
//核弹,不得外泄
void DataContainer::data_smooth_chusai()
{}
//核弹,不得外泄
void DataContainer::data_smooth4()
{}
//核弹,不得外泄
void DataContainer::data_smooth_3sigma()
{}