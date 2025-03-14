#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "ExactLIC.h"
#include <ctime>
#include "TD_Appro_LIC.h"
#include <string>


const int numNodes = 2394385;
std::vector<std::vector<std::pair<int,float>>> *graph = new std::vector<std::vector<std::pair<int,float>>>(numNodes);
std::vector<int> degree(numNodes,0);
std::vector<bool> isDelete(numNodes, false);
void readGraph()
{


    std::ifstream fin;
    fin.open("D:\\work-code\\LIC\\WikiTalk1-weighted.txt",std::ios::in);
    if(!fin.is_open())
    {
        std::cout<<"文件打开失败"<<std::endl;
        exit(1);
    }

    int u , v;
    float w;
    std::string buf;
    graph->push_back({});
    while(getline(fin, buf))
    {
        std::vector<std::string> data;
        std::string path;
        // 切割字符串，按照空格，或者制表符
        for(int i = 0 ; i < buf.size() ; i++)
        {
            if(buf[i]==' ' || buf[i]=='\t')
            {
                data.push_back(path);
                path.clear();
            }
            else if((buf[i]>='0' && buf[i]<='9') || buf[i] == '-' || buf[i] =='.')
            {
                path+=buf[i];
            }
        }
        if(path.empty())
            continue;
        data.push_back(path);
         u = atoi(data[0].c_str());
         v = atoi(data[1].c_str());
         w = atof(data[2].c_str());
         std::cout<<u<<" "<<v<<" "<<w<<" "<<std::endl;
        (*graph)[u].push_back({v,w});
        (*graph)[v].push_back({u,w});
         degree[u]++;
         degree[v]++;
    }
    //休整过剩的空间；

    fin.close();
}

int main() {


    readGraph();

    ExactLIC exactLic ;
//    TD_Appro_LIC tdApproLic(16,1,numNodes);

    std::cout<<"start running!\n";
    clock_t start,end;
    start = clock();
    std::vector<std::pair<std::vector<int>,float>> *kCore = exactLic.exactLIC(graph,degree,isDelete,16 ,1,numNodes);


//    auto *kCore = tdApproLic.tDApproLIC(graph, isDelete,degree);
    end = clock();

    std::ofstream outPutFile("D:\\work-code\\WikiTalk1-weighted-2394385-EXACT-16.txt");
    if(!outPutFile.is_open())
        std::cout<<"can not open file!\n";
//    std::cout<<"当前的k-core为："<<std::endl;
    outPutFile<<"EXACT-k-16 R-1\n";
    for(auto& cur : *kCore)
    {
        std::cout<<"the influential : \n" << cur.second<<std::endl;
        outPutFile<<"the influential : \n" << cur.second<<std::endl;
        std::cout<<"node's size: "<<cur.first.size()<<std::endl;
        outPutFile<<"node's size: "<<cur.first.size()<<std::endl;
        std::cout<<"nodes : "<<std::endl;
        outPutFile<<"nodes : "<<std::endl;
        for(auto& c : cur.first)
        {
            std::cout<<c<<" ";
            outPutFile<<c<<" ";
        }
        std::cout<<std::endl;
        outPutFile<<std::endl;
    }
    delete graph;
    delete kCore;
    std::cout<<"use time:"<<double(end-start)/CLOCKS_PER_SEC<<"s\n";
    outPutFile<<"use time:"<<double(end-start)/CLOCKS_PER_SEC<<"s\n";
    outPutFile.close();
    return 0;
}
