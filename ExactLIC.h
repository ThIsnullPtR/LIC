//
// Created by Lenovo on 2023/9/19.
//

#ifndef LIC_EXACTLIC_H
#define LIC_EXACTLIC_H
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
class ExactLIC {
public:
    std::vector<std::pair<std::vector<int>,float>> *exactLIC(std::vector<std::vector<std::pair<int,float>>> *graph , std::vector<int>&degree, std::vector<bool> &isDelete,int k , int r , int n);
//    ExactLIC(){}
    static int numLIC;
private:

    void enumLIC(std::vector<std::vector<std::pair<int, float>>> *graph, std::vector<int> &degree, std::unordered_set<int> &C,
                 std::unordered_set<int> &M,std::vector<bool> &isDelete, std::vector<std::pair<std::vector<int>, float>> *R, float maxBranch,
                 int k, int r, int n);

    static std::vector<std::unordered_set<int>>* computeKCore(const std::vector<std::vector<std::pair<int,float>>> *graph, std::vector<int>& degree,std::vector<bool>& isDelete, int k , int n);

    void deleteVertexByWeight(std::vector<std::vector<std::pair<int,float>>>*graph,std::vector<int>& degree , std::vector<bool> & isDelete , int k);

    static float vertexWeightUpperbound(const std::vector<std::vector<std::pair<int,float>>> *graph,std::vector<bool>& isDelete,std::vector<int>&degree, int vertexIndex , int k);

    static bool cmp(const std::pair<int,float>& a , const std::pair<int,float>& b);

    static bool isConnected(const std::unordered_set<int>& M ,const std::unordered_set<int>& connected , const std::vector<std::vector<std::pair<int,float>>> *graph);

    float subGraphWeightUpperBound(const std::unordered_set<int>& connected , const std::vector<std::vector<std::pair<int,float>>>* graph , int k);

    static float computeVertexWeight(const std::unordered_set<int>& connected , const std::vector<std::vector<std::pair<int,float>>>* graph , int vertex);

    float findTheKthWeight(std::vector<float>& weights , int k , int n);

    float quickSelect(std::vector<float> &weights , int left , int right , int k);

    static int partition(std::vector<float> &weights , int left , int right);

    static void constructGraph(std::vector<std::vector<std::pair<int,float>>>* newGraph , const std::vector<std::vector<std::pair<int,float>>> *graph , std::unordered_set<int> MWithC);

    static bool isSubset(const std::unordered_set<int> &A , const std::unordered_set<int> &B);

    static float subgraphInfluential(const std::unordered_set<int> &connected , const std::vector<std::vector<std::pair<int,float>>> *graph);

    static void updateR(const float &influential , const std::unordered_set<int> &core , std::vector<std::pair<std::vector<int>, float>> *R , int r);

    static void sortVec(std::vector<int> *coreVec ,const std::unordered_set<int> &core, const std::vector<std::vector<std::pair<int,float>>> *graph ,
                 const std::vector<int> &degree);

    static void updateVecDegree1(const std::vector<int> *core , int vec , const std::vector<std::vector<std::pair<int,float>>> *graph , std::vector<int> &degree);

    static void updateVecDegree2(const std::vector<int> *core , int vec , const std::vector<std::vector<std::pair<int,float>>> *graph , std::vector<int> &degree);

    static std::vector<std::unordered_set<int>>* computeKCore1(std::vector<std::vector<std::pair<int,float>>>* graph, std::vector<int>& degree,std::vector<bool>& isDelete,std::unordered_set<int> &connected, int k , int n);


};


#endif //LIC_EXACTLIC_H
