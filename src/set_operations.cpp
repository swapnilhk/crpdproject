#include"set_operations.h"
#include<set>

/**
 * @brief Does union of two sets
 * @param Set1 :- Input set 1
 * @param Set3 :- Input set 2
 * @param Set3 :- Output set
 */
void Set_Union(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2, it3;
    Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
        Set3.insert(*it1);
    for(it2=Set2.begin(); it2!=Set2.end(); ++it2)
        Set3.insert(*it2);
}

/**
 * @brief Does intersection of two sets
 * @param Set1 :- Input set 1
 * @param Set3 :- Input set 2
 * @param Set3 :- Output set
 */
void Set_Intersect(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2;
    Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 != Set2.end())
            Set3.insert(*it1);
    }
}

/**
 * @brief Calculates the number of elements in the set
 * @param Set1 :- Input set whose number of elements are
 * to be calculated
 */
int SET_MOD(std::set<int> & Set1){
    return Set1.size();
}

/**
 * @brief Calculates set defference Set1 - Set2
 * @param Set1 :- Input set 1
 * @param Set2 :- Input set 2
 * @param Set3 :- Output set. Set3 = Set1 - Set2
 */
void Set_Difference(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3){
	std::set<int>::iterator it1, it2;
	Set3.clear();
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 == Set2.end())
            Set3.insert(*it1);
    }
}
