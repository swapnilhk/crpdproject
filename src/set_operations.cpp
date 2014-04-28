#include<set>
#include<stdio.h>

void Set_Union(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2, it3;
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
        Set3.insert(*it1);
    for(it2=Set2.begin(); it2!=Set2.end(); ++it2)
        Set3.insert(*it2);
}

void Set_Intersect(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3){
    std::set<int>::iterator it1, it2;
    for(it1=Set1.begin(); it1!=Set1.end(); ++it1){
        it2 = Set2.find(*it1);
        if(it2 != Set2.end())
            Set3.insert(*it1);
    }
}

void MultiSet_Intersect(std::multiset<int> & Set1, std::multiset<int> & Set2, std::multiset<int> & Set3){
    std::multiset<int>::iterator it1, it2;
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

void print_SET(std::set<int> & Set1, FILE *fp){
    std::set<int>::iterator it;
    for(it=Set1.begin(); it!= Set1.end(); ++it)       
        fprintf(fp, "%d ", *it);
    fprintf(fp, "\n");
}

int TestMultiSet(){
    int i;
    std::multiset<int> mymultiset;
    std::multiset<int>::iterator it;
    std::set<int> mySet, yourSet, ourSet;
    std::set<int>::iterator itSet;
    // set some initial values:
    for (int i=1; i<=5; i++) mymultiset.insert(i*10);  // 10 20 30 40 50
    it=mymultiset.insert(25);
    it=mymultiset.insert (it,27);    // max efficiency inserting
    it=mymultiset.insert (it,29);    // max efficiency inserting
    it=mymultiset.insert (it,24);    // no max efficiency inserting (24<29)
    int myints[]= {5,10,15};
    mymultiset.insert (myints,myints+3);
    for (int i=1; i<=5; i++) mymultiset.insert(i*10);
    for (it=mymultiset.begin(); it!=mymultiset.end(); ++it)
	    mySet.insert(*it);
    Set_Union(mySet, yourSet, ourSet);
    ourSet.clear();
    Set_Intersect(mySet, yourSet, ourSet);
    return 0;
}
