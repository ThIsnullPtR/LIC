//
// Created by Lenovo on 2023/9/25.
//

#include "TD_Appro_LIC.h"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <ctime>

int TD_Appro_LIC::numLIC2 = 0;
int TD_Appro_LIC::numLIC1 = 0;

std::vector<std::pair<std::unordered_set<int>, float>>* TD_Appro_LIC::tDApproLIC(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree) {
    // todo 1.删除权值小于等于的0 的顶点
    deleteVertexByWeight(graph, isDelete , degree);
    //todo 2.计算k-core
    std::vector<std::unordered_set<int>> *kCore = computeKCore(graph, isDelete , degree);

    std::cout<<"KCore size : "<<kCore->size()<<std::endl;

    clock_t start,end;
    start = clock();
    //todo 3.得到第一个候选集
    std::vector<std::pair<std::unordered_set<int>, float>>* CAnd1 = computeCandidate1(graph, isDelete, degree,kCore);
    end = clock();

    std::cout<<"finnish candidate1\n";
    std::cout<<"use time:"<<double(end-start)/CLOCKS_PER_SEC<<"s\n";
    start = clock();
    //todo 4.得到第二个候选集
    std::vector<std::pair<std::unordered_set<int>, float>>* CAnd2 = computeCandidate2(graph, isDelete, degree,kCore);
    end = clock();
    std::cout<<"finnish candidate2\n";
    std::cout<<"use time:"<<double(end-start)/CLOCKS_PER_SEC<<"s\n";

    delete kCore;



    std::vector<std::pair<std::unordered_set<int>, float>> *R = unionCandidate(CAnd1, CAnd2);
    delete CAnd2;
    delete CAnd1;

    return R;
    
}


/**
 * 计算出第一个候选
 * @param kCore
 * @return
 */
std::vector<std::pair<std::unordered_set<int>, float >>*
TD_Appro_LIC::computeCandidate1(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> isDelete , std::vector<int> degree,const std::vector<std::unordered_set<int>> *kCore) {

    auto *CAnd = new std::vector<std::pair<std::unordered_set<int>,float>>();
    // todo 1.枚举所有的KCore
    for(auto core : *kCore)
    {
        std::cout<<"cur core size : "<<core.size()<<std::endl;
        auto *newGraph = new std::vector<std::vector<std::pair<int,float>>>(n);
        std::unordered_map<int,std::pair<int,int>> mp;
        //构建子图
        constructGraph(newGraph,graph,core,mp,degree);
        std::cout<<"finish constructGraph!\n";
        //排序
        auto *core_ = new std::vector<int>(core.size());
        int i = 0;
        for(auto &c : core)
            (*core_)[i++] = c;
        sortVertex(core_ , mp);
        std::unordered_map<int,std::pair<int,int>> ().swap(mp);
        for(auto &vec : *core_)
        {
            if(core.empty())
                break;
            if(core.find(vec) != core.end())
            {
                //todo 判断当前的核的influential是否大于0
                std::unordered_set<int> curCore = bfsFindCurCore(core, newGraph, isDelete,degree,vec);  //实际上是core的子集  改成计算k-core才行
                float influential = subgraphInfluential(curCore , graph);
                // todo 2.将局部影响力大于0的加入到候选集
//                std::cout<<influential<<std::endl;
                if (influential > 0) {
                    numLIC1++;
                    updateCAnd(CAnd,curCore,influential);   //保留前r就可以了，避免内存不够的情况；
                }
                // todo 4.dfs删除顶点
                std::unordered_map<int,std::unordered_map<int,bool>> edgeDelete;
                dfsDeleteVertex(newGraph,curCore,core,edgeDelete,isDelete,degree,vec);
                //释放空间
                std::unordered_map<int,std::unordered_map<int,bool>>().swap(edgeDelete);
                std::unordered_set<int>().swap(curCore);
            }
        }
        delete newGraph;
        delete core_;
    }

    return CAnd;
}
/**
 * 计算出第二个候选
 * @param kCore
 * @return
 */
std::vector<std::pair<std::unordered_set<int>, float>>*
TD_Appro_LIC::computeCandidate2(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> isDelete , std::vector<int> degree,const std::vector<std::unordered_set<int>> *kCore) {

    auto *CAnd = new std::vector<std::pair<std::unordered_set<int>,float>>();
    for(auto core : *kCore)
    {
        std::cout<<"cur core size : "<<core.size()<<std::endl;
        auto *newGraph = new std::vector<std::vector<std::pair<int,float>>>(n);
        std::unordered_map<int,std::pair<float,int>> mp;
        constructGraph(newGraph,graph,core,mp,degree);

        auto *core_ = new std::vector<int>(core.size());
        int i = 0;
        for(auto &c : core)
            (*core_)[i++] = c;
        sortVertex(core_ , mp);
        std::unordered_map<int,std::pair<float,int>> ().swap(mp);
        for(auto &vec : *core_)
        {
            if(core.empty())
                break;
            if(core.find(vec) != core.end())
            {
                std::unordered_set<int> curCore = bfsFindCurCore(core, newGraph, isDelete, degree,vec);  //实际上是core的子集 需要计算一下是不是k-core
                float influential = subgraphInfluential(curCore, newGraph);
                // todo 2.将局部影响力大于0的加入到候选集
//                std::cout<<influential<<std::endl;
                if (influential > 0) {
                    numLIC2++;
                    updateCAnd(CAnd, curCore, influential);   //保留前r就可以了，避免内存不够的情况；
                }
                // todo 4.dfs删除顶点
                std::unordered_map<int, std::unordered_map<int, bool>> edgeDelete;
                dfsDeleteVertex(newGraph, curCore, core, edgeDelete, isDelete, degree, vec);

                std::unordered_map<int, std::unordered_map<int, bool>> ().swap(edgeDelete);
                std::unordered_set<int> ().swap(curCore);

            }
        }
        delete core_;
        delete newGraph;

    }

    return CAnd;
}
/**
 * 求出k-core
 * @return
 */
std::vector<std::unordered_set<int>>*
TD_Appro_LIC::computeKCore(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree) const {
    int re_len = n;
    std::queue<int> que;
    for(int i = 0 ; i < n ; i++)
    {
        if(!isDelete[i] && degree[i] < k){
            re_len--;
            que.push(i);
            isDelete[i] = true;
        }
    }

    while(!que.empty())
    {
        int cur = que.front();
        que.pop();
        for(auto &curNode : (*graph)[cur])
        {
            if(!isDelete[curNode.first])
            {
                degree[curNode.first]--;
                if(degree[curNode.first] < k) {
                    re_len--;
                    que.push(curNode.first);
                    isDelete[curNode.first] = true;
                }
            }
        }
    }
    std::unordered_map<int,bool> vis;
    auto *ans = new std::vector<std::unordered_set<int>>();
    for(int i = 0 ; i < n ; i++)
    {
        if(!isDelete[i] && !vis[i])
        {
            std::unordered_set<int> path;
            path.reserve(re_len);
            que.push(i);
            vis[i] = true;
            while (!que.empty())
            {
                int cur = que.front();
                que.pop();
                path.insert(cur);
                for(auto& curNode : (*graph)[cur])
                {
                    if(!isDelete[curNode.first] && !vis[curNode.first])
                    {
                        que.push(curNode.first);
                        vis[curNode.first] = true;
                    }
                }

            }
            ans->push_back(path);
        }
    }
    return ans;
}
/**
 * 计算顶点上届
 * @param vertexIndex
 * @return
 */
float TD_Appro_LIC::vertexWeightUpperbound(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree,int vertexIndex) const {

    int positiveEdges = 0;
    for(auto& cur : (*graph)[vertexIndex])
    {
        if(isDelete[cur.first])
            continue;
        else if(cur.second > 0)
        {
            positiveEdges++;
        }
    }

    float theVertexWeightUpperBound = 0;
    if(positiveEdges < k)
    {
        for(int i = 0 , j = 0; i < k && i < degree[vertexIndex] && j < (*graph)[vertexIndex].size(); j++ )
        {
            if (!isDelete[(*graph)[vertexIndex][j].first]) {
                theVertexWeightUpperBound += (*graph)[vertexIndex][j].second;
                i++;
            }
        }

    }
    else
    {
        /**
         * 对大于0的调整
         */
        for(int i = 0 , j = 0; i < positiveEdges && i < degree[vertexIndex] && j < (*graph)[vertexIndex].size(); j++)
        {
            if (!isDelete[(*graph)[vertexIndex][j].first]) {
                theVertexWeightUpperBound += (*graph)[vertexIndex][j].second;
                i++;
            }
        }
    }

    return theVertexWeightUpperBound;
}
/**
 * 删除上届小于等于0的顶点
 */
void TD_Appro_LIC::deleteVertexByWeight(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree) {
    for(int i = 0 ; i < n; i++)
    {
        std::sort((*graph)[i].begin(),(*graph)[i].end(),cmp);
        float  vertexWeight = vertexWeightUpperbound(graph, isDelete , degree,i);
        if(vertexWeight <= 0)
        {
            //todo 2.删除顶点
            isDelete[i] = true;
            for(auto& cur :(*graph)[i])
            {
                if(!isDelete[cur.first])
                    degree[cur.first]--;
            }
        }
    }
}


/**
 * 在构建子图的时候顺便记录顶点的负权条数
 * @param newGraph
 * @param graph
 * @param core
 */
void TD_Appro_LIC::constructGraph(std::vector<std::vector<std::pair<int, float>>> *newGraph,
                                  const std::vector<std::vector<std::pair<int, float>>> *graph,
                                  const std::unordered_set<int> &core , std::unordered_map<int,std::pair<int,int>> &mp , const std::vector<int> &degree) {

    newGraph->push_back({});
    for(auto& cur : core)
    {
        mp[cur].second = degree[cur];
        for(auto &c : (*graph)[cur]) {
            if (core.find(c.first) != core.end()) {
                if(c.second < 0)
                    mp[cur].first++;
                (*newGraph)[cur].push_back(c);
            }
        }
        (*newGraph)[cur].shrink_to_fit();
    }
    std::cout<<"out for circle\n";
}

void TD_Appro_LIC::sortVertex(std::vector<int> *core_, std::unordered_map<int, std::pair<int, int>> &mp) {

    std::sort(core_->begin() , core_->end() ,[&mp](int v1 , int v2){

        if(mp.find(v1)->first == mp.find(v2)->first)
            return mp[v1].second < mp[v2].second;
        return mp[v1].first > mp[v2].first;
    });
}

/**
 * 计算子图的局部影响力
 * @param connected
 * @param graph
 * @return
 */
float TD_Appro_LIC::subgraphInfluential(const std::unordered_set<int> &connected,
                                        const std::vector<std::vector<std::pair<int, float>>> *graph) {
    float influential = 0;
    bool flag = false;
    std::cout<<"=========================================================================\n";
    for(auto con : connected)
    {
        float curInfluential = 0;
        for(auto cur : (*graph)[con])
        {
            if(connected.find(cur.first) != connected.end())
            {

                curInfluential += cur.second;
            }
        }
//        std::cout<<"curInfluential : "<<curInfluential<<std::endl;
        if(!flag)
        {
            influential = curInfluential;
            flag = true;
        }
        else
        {
            influential = influential < curInfluential ? influential : curInfluential;
        }
    }
    std::cout<<"the influential : "<<influential<<std::endl;
    std::cout<<"=============================================================================================================\n";
    return influential;
}
/**
 * 找出当前关联的社区
 * @param core
 * @param graph
 * @param isDelete
 * @param vertex
 * @return
 */
std::unordered_set<int> TD_Appro_LIC::bfsFindCurCore(std::unordered_set<int> &core,
                                                     std::vector<std::vector<std::pair<int, float>>> *graph,
                                                     std::vector<bool> &isDelete  ,std::vector<int> &degree, int vertex) {

    std::unordered_map<int,bool> vis;
    std::queue<int> que;
    for(auto &con : core)
    {
        if(!isDelete[con] && degree[con] < k)
        {
            que.push(con);
            isDelete[con] = true;
        }
    }

    while(!que.empty())
    {
        int cur = que.front();
        que.pop();
        core.erase(cur);
        for(auto &curNode : (*graph)[cur])
        {
            if(!isDelete[curNode.first] && core.find(curNode.first) != core.end())
            {
                degree[curNode.first]--;
                if(degree[curNode.first] < k)
                {
                    isDelete[curNode.first] = true;
                    que.push(curNode.first);
                }
            }
        }
    }
    std::unordered_set<int> ans;
    if(isDelete[vertex])
        return ans;

   que.push(vertex);
   vis[vertex] = true;
   ans.insert(vertex);
   while(!que.empty())
   {
       int cur = que.front();
       que.pop();
       for(auto& curNode : (*graph)[cur])
       {
           if(!isDelete[curNode.first] && core.find(curNode.first) != core.end() && !vis[curNode.first])
           {
               que.push(curNode.first);
               vis[curNode.first] = true;
               ans.insert(curNode.first);
           }
       }
   }

    return ans;
}

/**
 * 删除顶点；
 * @param graph
 * @param curCore
 * @param core
 * @param mp
 * @param isDelete
 * @param degree
 */
void
TD_Appro_LIC::dfsDeleteVertex(std::vector<std::vector<std::pair<int, float>>> *graph, std::unordered_set<int> &curCore,
                              std::unordered_set<int> &core, std::unordered_map<int, std::unordered_map<int, bool>> &mp,
                              std::vector<bool> &isDelete, std::vector<int> &degree , int vertex) {

    if(isDelete[vertex])
        return;
    for(auto &cur : (*graph)[vertex])
    {
        //邻居顶点还没被删除，当前边也还没被删除
        if(!isDelete[cur.first] && !mp[vertex][cur.first] && curCore.find(cur.first) != curCore.end())
        {
            degree[cur.first]--;
            degree[vertex]--;
            mp[vertex][cur.first] = true;  //true表示删除了
            mp[cur.first][vertex] = true;
            if(degree[cur.first] < k)
            {
                dfsDeleteVertex(graph,curCore,core,mp,isDelete,degree,cur.first);
            }
        }
    }
    isDelete[vertex] = true;
    core.erase(vertex);
    curCore.erase(vertex);
}
/**
 * 更新候选集；
 * @param CAnd
 */
void TD_Appro_LIC::updateCAnd(std::vector<std::pair<std::unordered_set<int>, float>> *CAnd , const std::unordered_set<int> &core , float influential) const {
    int len = CAnd->size();
    if(len == r)
    {
        if((*CAnd)[r-1].second < influential)
        {
            //应该先释放r-1这个位置的空间？
            std::pair<std::unordered_set<int>, float>().swap((*CAnd)[r-1]);

            (*CAnd)[r-1] = {core,influential};
        }
    }
    else
        CAnd->emplace_back(core,influential);

    for(int i = len-2 ; i >= 0 ; i--)
    {
        if((*CAnd)[i].second < (*CAnd)[i+1].second)
            std::swap(CAnd[i] , CAnd[i+1]);
        else
            break;
    }
}
/**
 * 为候选2进行排序
 * @param core_
 * @param mp
 */
void TD_Appro_LIC::sortVertex(std::vector<int> *core_, std::unordered_map<int, std::pair<float, int>> &mp) {

    std::sort(core_->begin(),core_->end(),[&mp](int v1 , int v2){
        if(mp[v1].first == mp[v2].first)
            return mp[v1].second < mp[v2].second;
        return mp[v1].first < mp[v2].first;
    });
}
/**
 * 构建图为候选2
 * @param newGraph
 * @param graph
 * @param core
 * @param mp
 * @param degree
 */
void TD_Appro_LIC::constructGraph(std::vector<std::vector<std::pair<int, float>>> *newGraph,
                                  const std::vector<std::vector<std::pair<int, float>>> *graph,
                                  const std::unordered_set<int> &core,
                                  std::unordered_map<int, std::pair<float, int>> &mp, const std::vector<int> &degree) {
    int len = core.size();
    for(auto& cur : core)
    {
        mp[cur].second = degree[cur];
//        newGraph[cur].reserve(core.size());   //预留n个空间
        float weight = 0;
        for(auto& c : (*graph)[cur]) {
            if (core.find(c.first) != core.end()) {
                weight += c.second;
                (*newGraph)[cur].push_back(c);
            }
        }
        mp[cur].first = weight;
        (*newGraph)[cur].shrink_to_fit();
    }

}

std::vector<std::pair<std::unordered_set<int>, float>>*
TD_Appro_LIC::unionCandidate(std::vector<std::pair<std::unordered_set<int>, float>> *cAnd1,
                             std::vector<std::pair<std::unordered_set<int>, float>> *cAnd2) {
    auto *R = new std::vector<std::pair<std::unordered_set<int>,float>>(cAnd1->begin(),cAnd1->end());
    for(auto &c : *cAnd2)
    {
        bool flag = false;
        for(auto &c1 : *R)
        {
            if(c1 == c)
            {
                flag = true;
                break;
            }
        }
        if(!flag)
            R->push_back(c);
    }
    R->shrink_to_fit();
    return R;
}

bool TD_Appro_LIC::cmp(const std::pair<int, float> &a, const std::pair<int, float> &b) {
    return a.second > b.second;
}


