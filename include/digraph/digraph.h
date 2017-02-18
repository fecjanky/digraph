#ifndef DIGRAPH_DIGRAPH_H_INCLUDED__
#define DIGRAPH_DIGRAPH_H_INCLUDED__

#include <type_traits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include <functional>
#include <iterator>
#include <cassert>
#include <initializer_list>

namespace digraph
{

template<typename T>
struct TD;

namespace detail
{
template<typename vertex_t>
struct vertex_type_impl
{
    using type = std::remove_cv_t<vertex_t>;
    using underlying_type = type;
    using is_value_type_t = std::true_type;
};

template<typename vertex_t>
struct vertex_type_impl<std::reference_wrapper<vertex_t>>
{
    using type = std::reference_wrapper<vertex_t>;
    using underlying_type = vertex_t;
    using is_value_type_t = std::false_type;
};

template<typename T>
struct get_vertex_type;

template<typename T>
struct get_vertex_type < std::pair<T, T> >
{
    using type = T;
};

template<typename T>
using get_vertex_type_t = typename get_vertex_type<T>::type;

template<typename edge_t, class vertex_getter >
struct vertex_type
{
    using ret_type = decltype(std::declval<vertex_getter>()(std::declval<edge_t>()));
    using vertex_t = get_vertex_type_t<std::remove_cv_t<std::remove_reference_t<ret_type>>>;
    using type = typename vertex_type_impl<vertex_t>::type;
    using underlying_type = typename vertex_type_impl<vertex_t>::underlying_type;
};


template<typename edge_t, class vertex_getter >
using vertex_type_t = typename vertex_type<edge_t, vertex_getter>::type;

template<typename edge_t, class vertex_getter >
using underlying_type_t = typename vertex_type<edge_t, vertex_getter>::underlying_type;


template<typename T, class Hash, class EqualTo, class It>
std::unordered_set<T, Hash, EqualTo> to_unique_set( It begin, It end )
{
    return std::unordered_set<T, Hash, EqualTo>( begin, end );
}


template<typename T>
struct deref
{
    using type = T;
};
template<typename T>
struct deref<std::reference_wrapper<T>>
{
    using type = typename deref<T>::type;
};
template<typename T>
using deref_t = typename deref<T>::type;

template<typename T>
T& do_derefence( T& t )
{
    return t;
}

template<typename T>
std::add_lvalue_reference_t<deref_t<T>> do_derefence( std::reference_wrapper<T> t )
{
    return t.get();
}

template<typename T>
struct has_hash_member_type
{
    template<typename TT>
    static std::true_type check( const TT&, typename TT::hash* );
    static std::false_type check( ... );
    static constexpr bool value = std::is_same<decltype(check( std::declval<T>(), nullptr )), std::true_type>::value;
};

template<typename T>
struct has_equal_to_member_type
{
    template<typename TT>
    static std::true_type check( const TT&, typename TT::equal_to* );
    static std::false_type check( ... );
    static constexpr bool value = std::is_same<decltype(check( std::declval<T>(), nullptr )), std::true_type>::value;
};

template<typename T, bool B>
struct select_hash
{
    using type = std::hash<T>;
};

template<typename T>
struct select_hash<T, true>
{
    using type = typename T::hash;
};

template<typename T, bool B>
struct select_equal_to
{
    using type = std::equal_to<T>;
};

template<typename T>
struct select_equal_to<T, true>
{
    using type = typename T::equal_to;
};


template<typename T>
using select_hash_t = typename select_hash<T, has_hash_member_type<T>::value>::type;

template<typename T>
using select_equal_to_t = typename select_equal_to<T, has_equal_to_member_type<T>::value>::type;

template<typename T>
struct unordered_traits
{
    using hash = select_hash_t<std::remove_cv_t<T>>;
    using equal_to = select_equal_to_t<std::remove_cv_t<T>>;
};

template<typename T>
struct unordered_traits<std::reference_wrapper<T>>
{
    struct hash
    {
        using hash_t = select_hash_t<std::remove_cv_t<T>>;
        using type = decltype(std::declval<hash_t>()(std::declval<T>()));

        type operator()( const T& t ) const
        {
            return hash_t()(t);
        }
    };
    struct equal_to
    {
        bool operator()( T& lhs, T& rhs ) const
        {
            return select_equal_to_t<T>()(lhs, rhs);
        }
    };
};

template<typename T>
struct remove_reference_wrapper
{
    using type = T;
};

template<typename T>
struct remove_reference_wrapper<std::reference_wrapper<T>>
{
    using type = T;
};

template<typename T>
using remove_reference_wrapper_t = typename remove_reference_wrapper<T>::type;

template<typename T>
using hash_t = typename unordered_traits<T>::hash;

template<typename T>
using equal_to_t = typename unordered_traits<T>::equal_to;

}


template<
    typename edge_t,
    class vertex_getter,
    class edge_hash = detail::hash_t<edge_t>,
    class edge_equal_to = detail::equal_to_t<edge_t>,
    class vertex_hash_ = detail::hash_t<detail::vertex_type_t<edge_t, vertex_getter>>,
    class vertex_equal_to_ = detail::equal_to_t<detail::vertex_type_t<edge_t, vertex_getter>>
>
class digraph
{
public:
    using vertex_hash = vertex_hash_;
    using vertex_equal_to = vertex_equal_to_;
    using vertex_type = detail::vertex_type_t<edge_t, vertex_getter>;
    using underlying_vertex_type = detail::underlying_type_t<edge_t, vertex_getter>;
    using edge_container = std::unordered_set<edge_t, edge_hash, edge_equal_to>;
    using vertex_container = std::unordered_set<vertex_type, vertex_hash, vertex_equal_to>;
    using vertex_iterator = typename vertex_container::iterator;
    using vertex_const_iterator = typename vertex_container::const_iterator;
    using edge_iterator = typename edge_container::iterator;
    using edge_const_iterator = typename edge_container::const_iterator;


    digraph() = default;
    ~digraph() = default;

    template<typename InputIterator>
    digraph( InputIterator begin, InputIterator end );

    digraph( const std::initializer_list<edge_t>& );

    digraph( const digraph& other );
    digraph( digraph&& other )  noexcept;
    digraph& operator =( const digraph& other );
    digraph& operator =( digraph&& other )  noexcept;

    void swap( digraph& other ) noexcept;

    edge_container& edges() noexcept;
    vertex_container& vertices() noexcept;
    const edge_container& edges() const noexcept;
    const vertex_container& vertices() const noexcept;

    vertex_const_iterator find( const underlying_vertex_type& _vertex ) const;
    edge_const_iterator find( const underlying_vertex_type& _from, const underlying_vertex_type& _to ) const;

    bool operator==( const digraph& ) const noexcept;
    bool operator!=( const digraph& ) const noexcept;

private:
    struct hash_vertex_iterator
    {
        using has_ret_type = decltype(std::declval<vertex_hash>()(std::declval<underlying_vertex_type>()));

        has_ret_type operator()( vertex_const_iterator i ) const
        {
            return vertex_hash()(detail::do_derefence( *i ));
        }
    };

    struct equal_to_vertex_iterator
    {
        bool operator()( vertex_const_iterator lhs, vertex_const_iterator rhs ) const
        {
            return vertex_equal_to()(detail::do_derefence( *lhs ), detail::do_derefence( *rhs ));
        }
    };

    using graph_val_t = std::unordered_map<vertex_const_iterator, edge_const_iterator, hash_vertex_iterator, equal_to_vertex_iterator>;
    using graph_repr_t = std::unordered_map<vertex_const_iterator, graph_val_t, hash_vertex_iterator, equal_to_vertex_iterator>;
    using graph_val_iterator = typename graph_val_t::iterator;
    using graph_val_const_iterator = typename graph_val_t::const_iterator;
    using graph_repr_iterator = typename graph_repr_t::iterator;
    using graph_repr_const_iterator = typename graph_repr_t::const_iterator;

    static graph_repr_t build_graph( const edge_container&, const vertex_container& );
    static vertex_const_iterator map_vertex( vertex_const_iterator from, const vertex_container& to );
    static graph_repr_const_iterator map_vertex( vertex_const_iterator from, const digraph& to );

    void rebuild_graph();
    graph_repr_t& graph() noexcept;
    const graph_repr_t& graph() const noexcept;

    edge_container      _edges;
    vertex_container    _vertices;
    graph_repr_t        _g;
};

template<typename DiGraphT>
class digraph_iterator
{
private:
    template<typename T>
    struct get_digraph_type;

    template<typename... T>
    struct get_digraph_type<digraph<T...>>
    {
        using type = digraph<T...>;
        using vertex_ptr = typename type::vertex_iterator;
    };
    template<typename... T>
    struct get_digraph_type<const digraph<T...>>
    {
        using type = const digraph<T...>;
        using vertex_ptr = typename type::vertex_const_iterator;
    };
    using vertex_ptr = typename get_digraph_type<DiGraphT>::vertex_ptr;
    using graph_ptr = std::add_pointer_t<typename get_digraph_type<DiGraphT>::type>;
public:
    explicit digraph_iterator( vertex_ptr n = nullptr, graph_ptr g = nullptr ) : _vertex( n ), _g( g ) {}
    digraph_iterator( const digraph_iterator & ) = default;
    digraph_iterator& operator= ( const digraph_iterator & ) = default;

private:
    vertex_ptr _vertex;
    graph_ptr _g;
};

///////////////////////
///////////////////////
///////////////////////

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::digraph( const std::initializer_list<edge_t>& list ) : digraph( list.begin(), list.end() )
{}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::digraph( const digraph & other ) :
    _vertices( other._vertices ), _edges( other._edges ), _g( build_graph( _edges , _vertices ) )
{
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::digraph( digraph && other ) noexcept :
    _vertices( std::move( other._vertices ) ), _edges( std::move( other._edges ) ), _g( std::move( other._g ) )
{}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::operator=( const digraph & other ) -> digraph &
{
    using std::swap;
    auto vertices = other._vertices;
    auto edges = other._edges;
    auto g = build_graph( edges, vertices );
    swap( _edges, edges );
    swap( _vertices, vertices );
    swap( _g, g );
    return *this;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::operator=( digraph && other ) noexcept -> digraph &
{
    using std::swap;
    if (this != &other) {
        _edges = std::move( other._edges );
        _vertices = std::move( other._vertices );
        _g = std::move( other._g );
    }
    return *this;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline void digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::swap( digraph & other ) noexcept
{
    swap( _edges, other._edges );
    swap( _vertices, other._vertices );
    swap( _g, other._g );
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::edges() const noexcept -> const edge_container &
{
    return _edges;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::vertices() const noexcept -> const vertex_container &
{
    return _vertices;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::edges() noexcept -> edge_container &
{
    return _edges;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::vertices() noexcept -> vertex_container &
{
    return _vertices;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::graph() noexcept -> graph_repr_t &
{
    return _g;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::graph() const noexcept -> const graph_repr_t &
{
    return _g;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash_, class vertex_equal_to_>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash_, vertex_equal_to_>::
find( const underlying_vertex_type & _from, const underlying_vertex_type & _to ) const -> edge_const_iterator
{
    const auto from = _vertices.find( vertex_type( _from ) );
    const auto to = _vertices.find( vertex_type( _to ) );
    if (from == _vertices.end() || to == _vertices.end())return _edges.end();
    const auto g_from = _g.find( from );
    if (g_from == _g.end())return _edges.end();
    const auto g_to = g_from->second.find( to );
    if (g_to == g_from->second.end())return _edges.end();
    return g_to->second;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::find( const underlying_vertex_type& _vertex ) const -> vertex_const_iterator
{
    return _vertices.find( vertex_type( _vertex ) );
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::
build_graph( const edge_container& edges, const vertex_container& vertices ) -> graph_repr_t
{
    graph_repr_t g;
    for (auto edge_it = edges.begin(); edge_it != edges.end(); ++edge_it) {
        const auto curr_vertices = vertex_getter()(*edge_it);
        auto begin = std::find_if( vertices.begin(), vertices.end(),
            [&]( const auto& n ) { return vertex_equal_to()(n, curr_vertices.first); } );
        assert( begin != vertices.end() );
        auto i = g.emplace( begin, graph_val_t{} );
        auto end = std::find_if( vertices.begin(), vertices.end(),
            [&]( const auto& n ) { return vertex_equal_to()(n, curr_vertices.second); } );
        assert( end != vertices.end() );
        i.first->second.emplace( end, edge_it );
    }
    return g;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline void digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::rebuild_graph()
{
    _g = build_graph( _edges, _vertices );
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
template<typename InputIterator>
inline digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::digraph( InputIterator begin, InputIterator end ) :
    _edges( detail::to_unique_set<edge_t, edge_hash, edge_equal_to>( begin, end ) )
{
    std::for_each( _edges.begin(), _edges.end(), [this]( const auto& e ) {
        const auto vertices = vertex_getter()(e);
        _vertices.emplace( vertices.first );
        _vertices.emplace( vertices.second );
    } );
    _vertices = detail::to_unique_set<vertex_type, vertex_hash, vertex_equal_to>( _vertices.begin(), _vertices.end() );
    rebuild_graph();
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::map_vertex( vertex_const_iterator from, const vertex_container& to ) -> vertex_const_iterator
{
    return to.find( *from );
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline auto digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::map_vertex( vertex_const_iterator from, const digraph& to ) -> graph_repr_const_iterator
{
    auto to_it = map_vertex( from, to.vertices() );
    if (to_it == to.vertices().end())return to.graph().end();
    return to.graph().find( to_it );
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline bool digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::operator==( const digraph& rhs) const noexcept
{
    const digraph& lhs = *this;
    if (lhs.edges() != rhs.edges() || lhs.vertices() != rhs.vertices())return false;
    for(auto from_it = lhs.graph().begin(); from_it != lhs.graph().end();++from_it){
        auto rhs_from_it = map_vertex( from_it->first, rhs );
        if(rhs_from_it == rhs.graph().end() || from_it->second.size() != rhs_from_it->second.size())return false;
        for (auto to_it = from_it->second.begin(); to_it != from_it->second.end(); ++to_it) {
            auto rhs_to_it = rhs_from_it->second.find( map_vertex( to_it->first, rhs.vertices() ) );
            if (rhs_to_it == rhs_from_it->second.end())return false;
            if(!edge_equal_to()(detail::do_derefence( *to_it->second ), detail::do_derefence( *rhs_to_it->second )))return false;
        }
    }
    return true;
}

template<typename edge_t, class vertex_getter, class edge_hash, class edge_equal_to, class vertex_hash, class vertex_equal_to>
inline bool digraph<edge_t, vertex_getter, edge_hash, edge_equal_to, vertex_hash, vertex_equal_to>::operator!=( const digraph& rhs ) const noexcept
{
    return !(*this == rhs);
}


}  //namespace digraph

#endif  //DIGRAPH_DIGRAPH_H_INCLUDED__
