#include <string>
#include <iostream>

#include "catch.hpp"

#include <digraph/digraph.h>



struct MyType
{
    std::string from;
    std::string to;
};

struct hash_MyType
{
    size_t operator()( const MyType& t )const noexcept
    {
        return std::hash<std::string>()(t.from) ^ std::hash<std::string>()(t.to);
    }
};

struct equal_to_MyType
{
    bool operator()( const MyType& lhs, const MyType& rhs ) const noexcept
    {
        return lhs.from == rhs.from && lhs.to == rhs.to;
    }
};

struct getnodes_MyType
{
    static const std::string& begin( const MyType& t )
    {
        return t.from;
    }
    static const std::string& end( const MyType& t )
    {
        return t.to;
    }
};


using digraph_t = digraph::digraph<MyType, getnodes_MyType, hash_MyType, equal_to_MyType>;

TEST_CASE( "DiGraph created from elems contains the edges and nodes that it was created from", "[digraph]" )
{
    digraph_t g{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    auto edge_a_b = g.find( "A", "B" );
    auto edge_b_c = g.find( "B", "C" );
    auto edge_b_a = g.find( "B", "A" );
    auto edge_c_a = g.find( "C", "A" );
    auto edge_d_a = g.find( "D", "A" );

    REQUIRE( g.nodes().size() == 3 );

    REQUIRE( g.find( "A" ) != g.nodes().end() );
    REQUIRE( g.find( "B" ) != g.nodes().end() );
    REQUIRE( g.find( "C" ) != g.nodes().end() );
    REQUIRE( g.find( "D" ) == g.nodes().end() );

    REQUIRE( edge_a_b != g.edges().end() );
    REQUIRE( edge_b_c != g.edges().end() );
    REQUIRE( edge_b_a != g.edges().end() );
    REQUIRE( edge_c_a != g.edges().end() );
    REQUIRE( edge_d_a == g.edges().end() );

    REQUIRE( edge_a_b->from == "A" );
    REQUIRE( edge_a_b->to == "B" );
    REQUIRE( edge_b_c->from == "B" );
    REQUIRE( edge_b_c->to == "C" );
    REQUIRE( edge_b_a->from == "B" );
    REQUIRE( edge_b_a->to == "A" );
    REQUIRE( edge_c_a->from == "C" );
    REQUIRE( edge_c_a->to == "A" );

}
