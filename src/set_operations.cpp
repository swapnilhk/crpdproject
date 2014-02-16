#include<iostream>
#include<set>
#include"stdio.h"

void Set_Union(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3)
{

  std::set<int>::iterator it1, it2, it3;

  for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
  {
       Set3.insert(*it1);
  }

  for(it2=Set2.begin(); it2!=Set2.end(); ++it2)
  {
       Set3.insert(*it2);
  }

}

void Set_Intersect(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3)
{
  std::set<int>::iterator it1, it2;

  for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
  {
       it2 = Set2.find(*it1);

       if(it2 != Set2.end())
              Set3.insert(*it1);

  }

}

void MultiSet_Intersect(std::multiset<int> & Set1, std::multiset<int> & Set2, std::multiset<int> & Set3)
{
     std::multiset<int>::iterator it1, it2;

     for(it1=Set1.begin(); it1!=Set1.end(); ++it1)
     {
       it2 = Set2.find(*it1);

       if(it2 != Set2.end())
       {       Set3.insert(*it1);
               Set2.erase(it2);
       }

     }

}

int MultiSet_MOD(std::multiset<int> & Set1)
{
    return Set1.size();
}


int SET_MOD(std::set<int> & Set1)
{
    return Set1.size();
}



void print_SET(std::set<int> & Set1, FILE *fp)
{
     std::set<int>::iterator it;

     for(it=Set1.begin(); it!= Set1.end(); ++it)
     {
         std::cout << ' ' << *it;
         fprintf(fp, "%d ", *it);
     }

     std::cout << '\n';
     fprintf(fp, "\n");

}


int TestMultiSet()
{
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

  std::cout << "mymultiset contains:";
  for (it=mymultiset.begin(); it!=mymultiset.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';

  std::cout << "Size of multiset is" << mymultiset.size() << std::endl ;

  for (it=mymultiset.begin(); it!=mymultiset.end(); ++it)
  {
      mySet.insert(*it);
  }

  std::cout << "MySet is " << std::endl;

  for(itSet=mySet.begin(); itSet!=mySet.end(); ++itSet)
  {
      std::cout << ' ' << *itSet;
  }

  std::cout << '\n' << '\n';
  std::cout << "Size of MySet is" << mySet.size() << std::endl ;

  for(i=1; i<10; i++)
  {
     yourSet.insert(i*20);
  }

  std::cout << "YourSet is " << std::endl;

  for(itSet=yourSet.begin(); itSet!=yourSet.end(); ++itSet)
  {
      std::cout << ' ' << *itSet;
  }

  std::cout << '\n' << '\n';
  std::cout << "Size of YourSet is" << yourSet.size() << std::endl ;

  Set_Union(mySet, yourSet, ourSet);

  std::cout << "Union OurSet is " << std::endl;
  for(itSet=ourSet.begin(); itSet!=ourSet.end(); ++itSet)
  {
      std::cout << ' ' << *itSet;
  }
  std::cout << '\n' << '\n';
  std::cout << "Size of OurSet is" << ourSet.size() << std::endl ;

  ourSet.clear();
  Set_Intersect(mySet, yourSet, ourSet);

  std::cout << "Intersection OurSet is " << std::endl;
  for(itSet=ourSet.begin(); itSet!=ourSet.end(); ++itSet)
  {
      std::cout << ' ' << *itSet;
  }
  std::cout << '\n' << '\n';
  std::cout << "Size of OurSet is" << SET_MOD(ourSet) << std::endl ;

  return 0;

}
