//
// Created by sheng on 18-5-1.
//

#ifndef ECS_FLAVOR_H
#define ECS_FLAVOR_H

#include <string>
#include <sstream>

//虚拟机规格类
class Flavor
{
public:
    int numCPU,numMEM;//cpu数和内存数,内存单位为G
    int count;//此类型虚拟机数量
    std::string name;//虚拟机名称
    Flavor(char *const info)
    {
        std::istringstream hostInfo(info);
        hostInfo>>name>>numCPU>>numMEM;
        numMEM/=1024;//MB单位换GB
        count=0;
    }
};


#endif //ECS_FLAVOR_H
