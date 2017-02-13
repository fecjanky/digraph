#include <string>
#include <iostream>

#include "digraph.h"



struct MyType
{
    const std::string& begin()const { return from; };
    const std::string& end()const { return to; };
    std::string from;
    std::string to;
    
    struct hash {
        size_t operator()(const MyType& t)const noexcept{
            return std::hash<std::string>()(t.from) ^ std::hash<std::string>()(t.to);
        }
    };
    struct equal_to{
        bool operator()(const MyType& lhs, const MyType& rhs) const noexcept{
            return lhs.from == rhs.from && lhs.to == rhs.to;
        }
    };
};


int main( int, char** )
{
    std::string s2 = "kkk";
    digraph<MyType>::node_container c{ digraph<MyType>::node_type( s2 ) };
    std::string s = "kkk";
    auto ret = c.find( digraph<MyType>::node_type(s) );
    digraph<MyType> g{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    auto edge_a_b = g.find( "A", "B" );
    auto edge_a_d = g.find( "A", "D" );
    if (edge_a_b != g.edges().end()) {
        auto& s = *edge_a_b;
    }
    int a = 0;
}