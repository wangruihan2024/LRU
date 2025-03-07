#include "src.hpp"
#include <iostream>
#include <cassert>
#include <string>
int main() {
    sjtu::double_list<int> a;
    a.insert_tail(3);
    a.insert_tail(4);
    a.insert_head(5);
    a.insert_head(6);
    //6534
    a.print();
    a.delete_head();
    a.delete_tail();
    //53
    a.print();
    a.insert_tail(4);
    a.insert_tail(6);
    //5346
    a.print();
    
    auto it = a.begin();
    ++it;
    std::cout << *it << std::endl;
    auto it2 = a.erase(it); 
    std::cout << *it2 << std::endl;
    a.print();  
    //546
    it2++;
    std::cout << *a.erase(it2) << std::endl;
    a.print();
    //5 4
    // std::cout << *it << std::endl;
    //应抛出错误
}