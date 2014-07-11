#ifndef SET_OPERATIONS_H
#define SET_OPERATIONS_H
#include<set>
void Set_Union(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3);
void Set_Intersect(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3);
int SET_MOD(std::set<int> & Set1);
void Set_Difference(std::set<int> Set1, std::set<int> Set2, std::set<int> & Set3);
#endif
