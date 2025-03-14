//
// Created by Lenovo on 2023/9/20.
//
#include "ExactLIC.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

int ExactLIC::numLIC = 0;
/**
 * 精确算法部分
 * @param graph
 * @param degree
 * @param k
 * @param r
 * @param n
 * @return
 */
std::vector<std::pair<std::vector<int>,float>>*
ExactLIC::exactLIC(std::vector<std::vector<std::pair<int, float>>>*graph, std::vector<int> &degree, std::vector<bool> &isDelete, int k, int r,
                   int n) {

//    std::vector<bool>isDelete(n,false);
    // 1.delete weight<0 的顶点
    deleteVertexByWeight(graph,degree,isDelete,k);
    // 2.computeKCore
    auto *kCore =  computeKCore(graph,degree,isDelete,k,n);

    std::cout<<"has "<<kCore->size()<<" k-core\n";

    //todo 3.枚举k-core中的连通分量
    auto *R = new std::vector<std::pair<std::vector<int>,float>>(r);
    std::unordered_set<int> *M = new std::unordered_set<int>;
    for(auto &core : *kCore)
    {
        auto *newGraph = new std::vector<std::vector<std::pair<int,float>>>(n);
        constructGraph(newGraph,graph,core);
        enumLIC(newGraph,degree,core,(*M),isDelete,R,-1,k,r,n);
        delete newGraph;
        M->clear();
    }
    delete M;
    delete kCore;


    //todo 4.返回结果
    return R;
}

/**
 * 枚举结果集
 * @param graph 原图
 * @param degree 顶点度
 * @param C 原K-core的顶点，是候选要删除的顶点
 * @param M 保留顶点
 * @param isDelete 因为计算k-core 和 顶点上届被删除 的顶点会被标记
 * @param R 结果集
 * @param maxBranch 当前分支最大的influential
 * @param k k-value
 * @param r top-r
 * @param n 顶点数量
 */
void
ExactLIC::enumLIC(std::vector<std::vector<std::pair<int, float>>> *graph, std::vector<int> &degree, std::unordered_set<int> &C,
                  std::unordered_set<int> &M,std::vector<bool> &isDelete, std::vector<std::pair<std::vector<int>, float>> *R, float maxBranch,
                  int k, int r, int n) {
    // todo 1.pruning 1 判断保留的顶点和删除的顶点之间有没有冲突
    for(auto & m : M)
    {
        if(isDelete[m]) {
            return;
        }
    }
    // todo 2.pruning 2 判断保留和候选的顶点的数量是否小于k 形成k-core至少需要k+1个顶点
    // C/D
    auto *del = new std::vector<int>();
    for(auto& c : C)
    {
        if(isDelete[c])
            del->push_back(c);
    }
    for(auto& d : *del)
    {
        C.erase(d);
    }
    delete del;

    if(M.size() + C.size() <= k) {
        return;
    }
    // todo 3.pruning 3 判断是否连通MUC
    std::unordered_set<int> *MWithC = new std::unordered_set<int>;
    std::set_union(M.begin(),M.end(),C.begin(),C.end(), std::inserter(*MWithC,MWithC->begin()));


    if(!isConnected(M,*MWithC,graph)) {
        return;
    }

    // todo 4.pruning 4 判断子图的影响力是否小于第r大的以及当前分支最大的
    float subgraphVertexWeight = subGraphWeightUpperBound(*MWithC,graph,k);
    if(subgraphVertexWeight <= maxBranch) {
        return;
    }
    if(R->size() == r && subgraphVertexWeight <= (*R)[r-1].second) {
        return;
    }
    // todo 5.遍历connected Component 是否满足k-LIC的条件
    auto *newKCore = computeKCore1(graph,degree,isDelete,*MWithC,k,n);
    delete MWithC;
    for(auto &core : *newKCore)
    {
//        if(!isSubset(core,M))
//            continue;
        float influential = subgraphInfluential(core,graph);
        if(influential > 0)   //子图的影响力要大于0
            {
                numLIC++;
                updateR(influential,core,R,r);
                std::cout<<"ans size:"<<core.size()<<std::endl;
                std::cout<<"cur influential:"<<influential<<std::endl;
                if (influential > maxBranch) {
                    maxBranch = influential;
                }
            }

        std::cout<<"core size:"<<core.size()<<std::endl;
        // todo 6.继续向下一个分支遍历
        auto *coreVec = new std::vector<int>(core.size());
        int j = 0;
        for(auto &co : core)
        {
            (*coreVec)[j++] = co;
        }
//        std::unordered_set<int> M1 (M.begin(),M.end());
        // todo 需要对cVec进行排序，按照顶点的权重以及顶点的度
        sortVec(coreVec,core,graph,degree);
        for(auto &cVec : *coreVec)
        {
            core.erase(cVec);

            //todo 候选顶点去除之后顶点的度应该需要进行调整
            updateVecDegree1(coreVec,cVec,graph,degree);
            isDelete[cVec] = true;

            enumLIC(graph,degree,core,M,isDelete,R,maxBranch,k,r,n);
            //todo 回溯，需要对度进行恢复
            updateVecDegree2(coreVec , cVec , graph, degree);
            isDelete[cVec] = false;

            M.insert(cVec);
        }
        delete coreVec;

    }
    delete newKCore;

}

/**
 * 删除顶点的权重小于0的顶点
 */
void ExactLIC::deleteVertexByWeight(std::vector<std::vector<std::pair<int, float>>>*graph, std::vector<int> &degree,
                                    std::vector<bool> &isDelete , int k) {

    //todo 1.计算出顶点的权上届
    for(int i = 0 ; i < graph->size(); i++)
    {
        std::sort((*graph)[i].begin(),(*graph)[i].end(),cmp);
        float vertexWeight = vertexWeightUpperbound(graph,isDelete,degree,i, k);
        if(vertexWeight <= 0)
        {
            //todo 2.删除顶点
            isDelete[i] = true;
            for(auto& cur : (*graph)[i])
            {
                if(!isDelete[cur.first])
                    degree[cur.first]--;
            }
        }

    }
}

/**
 * 计算顶点的权值上界
 */
float ExactLIC::vertexWeightUpperbound(const std::vector<std::vector<std::pair<int, float>>> *graph, std::vector<bool> &isDelete, std::vector<int> &degree,int vertexIndex,int k) {


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
        for(int i = 0 , j = 0; i < k && i < degree[vertexIndex] && j < (*graph)[vertexIndex].size(); j++)
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
 * 比较规则
 * @param a
 * @param b
 * @return
 */
bool ExactLIC::cmp(const std::pair<int, float> &a, const std::pair<int, float> &b) {
    return a.second > b.second;
}

/**
 * 找出k-core
 * @param graph
 * @param degree
 * @param isDelete
 * @param k
 * @param n
 * @return
 */
std::vector<std::unordered_set<int>>*
ExactLIC::computeKCore(const std::vector<std::vector<std::pair<int, float>>> *graph, std::vector<int> &degree,
                       std::vector<bool> &isDelete, int k , int n) {
    auto *que = new std::queue<int>();
    for(int i = 0 ; i < n ; i++)
    {
        if(degree[i] < k && !isDelete[i]){
            que->push(i);
            isDelete[i] = true;
        }
    }
    while(!que->empty())
    {
        int cur = que->front();
        que->pop();
        for(auto &curNode : (*graph)[cur])
        {
            if(!isDelete[curNode.first])
            {
                degree[curNode.first]--;
                if(degree[curNode.first] < k)
                {
                    que->push(curNode.first);
                    isDelete[curNode.first] = true;
                }
            }
        }

    }

    auto *vis = new std::vector<bool>(n,false);
    auto *ans = new std::vector<std::unordered_set<int>>();
    for(int i = 0 ; i < n ; i++)
    {
        if(!isDelete[i] && !(*vis)[i])
        {
            std::unordered_set<int> path;
            (*vis)[i] = true;
            que->push(i);
            while(!que->empty())
            {
                int cur = que->front();
                que->pop();
                path.insert(cur);
                for(auto& nb : (*graph)[cur])
                {
                    if(!isDelete[nb.first]&&!(*vis)[nb.first])
                    {
                        que->push(nb.first);
                        (*vis)[nb.first] = true;
                    }
                }
            }

            ans->push_back(path);
        }

    }
    delete vis;
    delete que;
    return ans;
}

/**
 * 判断子图是否连通
 * @param connected
 * @param graph
 * @return
 */
bool
ExactLIC::isConnected(const std::unordered_set<int>& M,const std::unordered_set<int> &connected, const std::vector<std::vector<std::pair<int, float>>> *graph) {
    if(M.empty())
        return true;
    int n = M.size();

    std::unordered_map<int,bool> vis;
    auto *que = new std::queue<int>();

    que->push(*M.begin());
    vis[*M.begin()] = true;

    while(!que->empty())
    {
        int cur = que->front();
        if(M.find(cur) != M.end())
            n--;
        que->pop();
        for(auto c : (*graph)[cur])
        {
            if(!vis[c.first] && connected.find(c.first) != connected.end())
            {
                que->push(c.first);
                vis[c.first] = true;
            }

        }

    }
    delete que;
    return n == 0;
}

/**
 * 计算子图的影响力上届
 * @param connected
 * @param graph
 * @return
 */
float ExactLIC::subGraphWeightUpperBound(const std::unordered_set<int> &connected,
                                         const std::vector<std::vector<std::pair<int, float>>> *graph , const int k) {
    if(connected.empty())
        return 0;
    int n = connected.size();
    std::vector<float> weights(n);
    // todo 1.计算出顶点的权
    int i = 0 ;
    for(auto& con : connected)
    {
        weights[i] = computeVertexWeight(connected , graph , con);
        i++;
    }
    // todo 2.找到第k+1大的顶点的权

    return findTheKthWeight(weights,k,n);
}
/**
 * 计算顶点的权
 * @param connected
 * @param graph
 * @return
 */
float ExactLIC::computeVertexWeight(const std::unordered_set<int> &connected,
                                    const std::vector<std::vector<std::pair<int, float>>> *graph , const int vertex) {
    float weight = 0;
    for(auto &cur : (*graph)[vertex])
    {
        if(connected.find(cur.first) != connected.end())
        {
            weight += cur.second;
        }
    }
    return weight;
}

/**
 * 找到第k大的权重
 * @param weights
 * @param k
 * @return
 */
float ExactLIC::findTheKthWeight(std::vector<float> &weights, const int k,const int n) {

    return quickSelect(weights,0,n-1,k);
}

/**
 * 利用快速排序实现 较为快速的查找第k+1大的权
 * @param weights
 * @param left
 * @param right
 * @param k
 * @return
 */
float ExactLIC::quickSelect(std::vector<float> &weights, const int left, const int right, const int k) {
   if(left >= right)
       return weights[left];
   int pivotIndex = partition(weights,left,right);
   if(k==pivotIndex)
       return weights[k];
   else if(k < pivotIndex)
       return quickSelect(weights,left,pivotIndex-1,k);
   else
       return quickSelect(weights,pivotIndex+1,right,k);
}
/**
 * 确定快速排序基准
 * @param weights
 * @param left
 * @param right
 * @return
 */
int ExactLIC::partition(std::vector<float> &weights, int left,int right) {
    if(left >= right)
        return left;
    float pivot = weights[left];

    int i = left , j = right;
    while(i < j)
    {
        while(i < j && weights[j] <= pivot)
            j--;
        if(i < j)
            weights[i++] = weights[j];
        while(i < j && weights[i] >= pivot)
            i++;
        if(i < j)
            weights[j--] = weights[i];
    }

    weights[i] = pivot;
    return i;
}
/**
 * 利用现存的顶点集合构建图
 * @param newGraph
 * @param graph
 * @param MWithC
 */
void ExactLIC::constructGraph(std::vector<std::vector<std::pair<int, float>>> *newGraph,
                              const std::vector<std::vector<std::pair<int, float>>> *graph, std::unordered_set<int> MWithC) {


    newGraph->push_back({});
    for(auto& cur : MWithC)
    {
            for(auto& c : (*graph)[cur]) {
                if (MWithC.find(c.first) != MWithC.end()) {
                    (*newGraph)[cur].push_back(c);
                }
            }
        (*newGraph)[cur].shrink_to_fit();
    }
    //修正空间
}
/**
 * 判单B是否是A的子集
 * @param A
 * @param B
 * @return
 */
bool ExactLIC::isSubset(const std::unordered_set<int> &A, const std::unordered_set<int> &B) {
    if(B.empty())
        return true;
    for(auto& b : B)
    {
        if(A.find(b) == A.end())
            return false;
    }
    return true;
}
/**
 * 获取子图的影响力
 * @param connected
 * @param graph
 * @return
 */
float ExactLIC::subgraphInfluential(const std::unordered_set<int> &connected,
                                    const std::vector<std::vector<std::pair<int, float>>> *graph) {
    float influential = 0;
    bool flag = false;
    for(auto& con : connected) {
        float curInfluential = 0;
        for (auto &cur: (*graph)[con]) {
            if (connected.find(cur.first) != connected.end()) {
                curInfluential += cur.second;
            }
        }
        if (!flag)
        {
            influential = curInfluential;
            flag = true;
        }
       else
           influential = std::min(curInfluential,influential);
    }

    return influential;
}
/**
 * 更行结果集
 * @param influential
 * @param core
 * @param R
 */
void ExactLIC::updateR(const float &influential, const std::unordered_set<int> &core,
                       std::vector<std::pair<std::vector<int>, float>> *R , const int r) {


    auto *core_1 = new std::vector<int>(core.size());
    int j = 0;
    for(auto &co : core){
        (*core_1)[j++] = co;

    }

    //保证R中的元素是从大到小排序的
    if (R->size() == r) {
        if ((*R)[r - 1].second < influential) {
            std::pair<std::vector<int>,float>().swap((*R)[r - 1]);  //释放空间
            (*R)[r - 1].first = *core_1;
            (*R)[r - 1].second = influential;
        }
    } else {

        R->emplace_back(*core_1,influential);
    }
    delete core_1;
    for(int i = R->size() - 2 ; i >= 0 ; i--)
    {
        if((*R)[i].second < (*R)[i+1].second)
            std::swap(R[i],R[i+1]);
        else
            break;
    }

}
/**
 * 对顶点进行排序；按照顶点的影响力，度进行排序。
 * @param coreVec
 * @param graph
 * @param degree
 * @param isDelete
 */
void ExactLIC::sortVec(std::vector<int> *coreVec , const std::unordered_set<int> &core, const std::vector<std::vector<std::pair<int, float>>> *graph,
                       const std::vector<int> &degree) {

    std::unordered_map<int,std::pair<float,int>> mp;  //记录一下顶点的影响力和度

    for(auto &vec : *coreVec)
    {
        float influential = computeVertexWeight(core, graph, vec);

        mp[vec] = {influential , degree[vec]};
    }

// todo 出问题的原因应该是 lambada表达式的引用传递出现会出现指针悬空
    std::sort(coreVec->begin(),coreVec->end() , [&mp](int v1 , int v2)
    {
        if(mp.find(v1) != mp.end() && mp.find(v2) != mp.end()) {
            if (mp[v1].first == mp[v2].first)
                return mp[v1].second <= mp[v2].second;
            return mp[v1].first <= mp[v2].first;
        }
        return false;

    });



}
/**
 * 当有顶点被移除出候选的时候要对当前的顶点的度进行向下调整
 * @param core
 * @param vec
 * @param graph
 * @param degree
 */
void ExactLIC::updateVecDegree1(const std::vector<int> *core, int vec,
                                const std::vector<std::vector<std::pair<int, float>>> *graph,
                                std::vector<int> &degree) {
    std::unordered_set<int> s(core->begin(),core->end());
    for(auto &cur : (*graph)[vec])
    {
        if(s.find(cur.first) != s.end())
            degree[cur.first]--;
    }
}
/**
 * 走完一个分支要对顶点的度进行回溯调整
 * @param core
 * @param vec
 * @param graph
 * @param degree
 */
void ExactLIC::updateVecDegree2(const std::vector<int>*core, int vec,
                                const std::vector<std::vector<std::pair<int, float>>> *graph,
                                std::vector<int> &degree) {
    std::unordered_set<int> s(core->begin(),core->end());
    for(auto &cur : (*graph)[vec])
    {
        if(s.find(cur.first) != s.end())
            degree[cur.first]++;
    }
}

std::vector<std::unordered_set<int>>*
ExactLIC::computeKCore1(std::vector<std::vector<std::pair<int, float>>> *graph, std::vector<int> &degree,
                        std::vector<bool> &isDelete, std::unordered_set<int> &connected, int k, int n) {
    std::queue<int> que;
//    for(int i = 0 ; i < n ; i++)
//    {
//        if(degree[i] < k && !isDelete[i] && connected.find(i) != connected.end()){
//            que.push(i);
//            isDelete[i] = true;
//        }
//    }
    for(auto& con : connected)
    {
        if(degree[con] < k && !isDelete[con])
        {
            que.push(con);
            isDelete[con] = true;
        }
    }
    while(!que.empty())
    {
        int cur = que.front();
        que.pop();
        for(auto &curNode : (*graph)[cur])
        {
            if(!isDelete[curNode.first] && connected.find(curNode.first) != connected.end())
            {
                degree[curNode.first]--;
                if(degree[curNode.first] < k)
                {
                    que.push(curNode.first);
                    isDelete[curNode.first] = true;
                }
            }
        }

    }

   std::unordered_map<int,bool>vis;
    std::vector<std::unordered_set<int>> *ans = new std::vector<std::unordered_set<int>>();
    for(auto &con : connected)
    {
        if(!isDelete[con] && !vis[con])
        {
            std::unordered_set<int> path;
            vis[con] = true;
            que.push(con);
            while(!que.empty())
            {
                int cur = que.front();
                que.pop();
                path.insert(cur);
                for(auto& nb : (*graph)[cur])
                {
                    if(!isDelete[nb.first]&&!vis[nb.first]&&connected.find(nb.first) != connected.end())
                    {
                        que.push(nb.first);
                        vis[nb.first] = true;
                    }
                }
            }

            ans->push_back(path);
        }
    }
    return ans;
}




