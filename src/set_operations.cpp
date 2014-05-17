#include"set_operations.h"
#include<set>

void Set_Union(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2, it3;
    Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
        Set3.insert(*it1);
    for(it2=Set2.begin(); it2!=Set2.end(); ++it2)
        Set3.insert(*it2);
}

void Set_Intersect(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2;
    Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 != Set2.end())
            Set3.insert(*it1);
    }
}

void MultiSet_Intersect(std::multiset<int> Set1, std::multiset<int> Set2, std::multiset<int> & Set3){
    std::multiset<int>::iterator it1, it2;
    Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 != Set2.end()){
            Set3.insert(*it1);
            Set2.erase(it2);
        }
    }
}

int MultiSet_MOD(std::multiset<int> & Set1){
    return Set1.size();
}

int SET_MOD(std::set<int> & Set1){
    return Set1.size();
}

void Set_Difference(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
	std::set<int>::iterator it1, it2;
	Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 == Set2.end())
            Set3.insert(*it1);
    }
}
