//
// Created by Lenovo on 2023/9/25.
//
#include <vector>
#include <unordered_set>
#include <unordered_map>
#ifndef LIC_TD_APPRO_LIC_H
#define LIC_TD_APPRO_LIC_H


class TD_Appro_LIC {
public:
    std::vector<std::pair<std::unordered_set<int>, float>>* tDApproLIC(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree);

    TD_Appro_LIC(){}
    TD_Appro_LIC(int k_ , int r_ , int n_):k(k_),r(r_),n(n_){}

    static int numLIC1;
    static int numLIC2;


private:
    std::vector<std::unordered_set<int>>* computeKCore(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree) const;

    void deleteVertexByWeight(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree);

    float vertexWeightUpperbound(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> &isDelete , std::vector<int> &degree,int vertexIndex) const;

    std::vector<std::pair<std::unordered_set<int>,float >>* computeCandidate1(std::vector<std::vector<std::pair<int , float>>> *graph, std::vector<bool> isDelete
                                                                      , std::vector<int> degree , const std::vector<std::unordered_set<int>>* kCore);

    std::vector<std::pair<std::unordered_set<int>,float>>* computeCandidate2(std::vector<std::vector<std::pair<int , float>>> *graph , std::vector<bool> isDelete
                                                                     , std::vector<int> degree , const std::vector<std::unordered_set<int>>* kCore);

    static void constructGraph(std::vector<std::vector<std::pair<int,float>>> *newGraph , const std::vector<std::vector<std::pair<int,float>>> *graph
                         , const std::unordered_set<int> &core , std::unordered_map<int,std::pair<int,int>> &mp , const std::vector<int> &degree);

    static void sortVertex(std::vector<int> *core_ , std::unordered_map<int,std::pair<int,int>>& mp);

    static float subgraphInfluential(const std::unordered_set<int> &connected , const std::vector<std::vector<std::pair<int,float>>> *graph);

    std::unordered_set<int> bfsFindCurCore(std::unordered_set<int> &core ,  std::vector<std::vector<std::pair<int,float>>> *graph , std::vector<bool> &isDelete, std::vector<int> &degree , int vec);

    void dfsDeleteVertex(std::vector<std::vector<std::pair<int,float>>> *graph,std::unordered_set<int> &curCore
                         ,std::unordered_set<int> &core,std::unordered_map<int,std::unordered_map<int,bool>> &mp,
                         std::vector<bool> &isDelete, std::vector<int> &degree , int vertex);
    void updateCAnd(std::vector<std::pair<std::unordered_set<int>,float>> *CAnd , const std::unordered_set<int> &core , float influential) const;

    static void sortVertex(std::vector<int> *core_ , std::unordered_map<int,std::pair<float,int>>& mp);

    static void constructGraph(std::vector<std::vector<std::pair<int,float>>> *newGraph , const std::vector<std::vector<std::pair<int,float>>> *graph
            , const std::unordered_set<int> &core , std::unordered_map<int,std::pair<float,int>> &mp , const std::vector<int> &degree);

    static std::vector<std::pair<std::unordered_set<int>,float>>* unionCandidate(std::vector<std::pair<std::unordered_set<int>,float>> *cAnd1 , std::vector<std::pair<std::unordered_set<int>,float>> *cAnd2);

    static bool cmp(const std::pair<int, float> &a, const std::pair<int, float> &b);




    int r;
    int n;
    int k;
};


#endif //LIC_TD_APPRO_LIC_H
