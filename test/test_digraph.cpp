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

struct getvertices_MyType
{
    using str_const_ref = std::reference_wrapper<const std::string>;
    std::pair<str_const_ref, str_const_ref> operator()( const MyType& t ) const
    {
        return std::make_pair( std::ref( t.from ), std::ref( t.to ));
    }

};

struct getvertices_by_value_MyType
{
    std::pair<std::string, std::string> operator()( const MyType& t ) const
    {
        return std::make_pair( t.from,t.to );
    }

};



using digraph_t = digraph::digraph<MyType, getvertices_MyType, hash_MyType, equal_to_MyType>;
using digraph_by_val_t = digraph::digraph<MyType, getvertices_by_value_MyType, hash_MyType, equal_to_MyType>;

TEST_CASE( "DiGraph created from elems contains the edges and vertices that it was created from", "[digraph]" )
{
    digraph_t g{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    auto edge_a_b = g.find( "A", "B" );
    auto edge_b_c = g.find( "B", "C" );
    auto edge_b_a = g.find( "B", "A" );
    auto edge_c_a = g.find( "C", "A" );
    auto edge_d_a = g.find( "D", "A" );

    REQUIRE( g.vertices().size() == 3 );

    REQUIRE( g.find( "A" ) != g.vertices().end() );
    REQUIRE( g.find( "B" ) != g.vertices().end() );
    REQUIRE( g.find( "C" ) != g.vertices().end() );
    REQUIRE( g.find( "D" ) == g.vertices().end() );

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

TEST_CASE( "DiGraph (vertices getter by value) created from elems contains the edges and vertices that it was created from", "[digraph]" )
{
    digraph_by_val_t g{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    auto edge_a_b = g.find( "A", "B" );
    auto edge_b_c = g.find( "B", "C" );
    auto edge_b_a = g.find( "B", "A" );
    auto edge_c_a = g.find( "C", "A" );
    auto edge_d_a = g.find( "D", "A" );

    REQUIRE( g.vertices().size() == 3 );

    REQUIRE( g.find( "A" ) != g.vertices().end() );
    REQUIRE( g.find( "B" ) != g.vertices().end() );
    REQUIRE( g.find( "C" ) != g.vertices().end() );
    REQUIRE( g.find( "D" ) == g.vertices().end() );

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


TEST_CASE( "DiGraph can be copied and the contents are going to be equal", "[digraph]" )
{
    digraph_t g_a{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    digraph_t g_b = g_a ;
    REQUIRE( g_a == g_b );
    REQUIRE_FALSE( g_a != g_b );
}

TEST_CASE( "DiGraph (getter_by_value) can be copied and the contents are going to be equal", "[digraph]" )
{
    digraph_by_val_t g_a{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    digraph_by_val_t g_b = g_a;
    REQUIRE( g_a == g_b );
    REQUIRE( !(g_a != g_b) );
}

TEST_CASE( "DiGraph equality comparison returns false if graphs are not made from the same edges", "[digraph]" )
{
    digraph_t g_a{ { "A", "B" },{ "B","C" } ,{ "C","A" },{ "B","A" } };
    digraph_t g_b{ { "A", "B" },{ "B","C" } ,{ "C","A" } };
    
    REQUIRE_FALSE( g_a == g_b );
    REQUIRE( g_a != g_b );
}


