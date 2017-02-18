#ifndef DIGRAPH_DIGRAPH_H_INCLUDED__
#define DIGRAPH_DIGRAPH_H_INCLUDED__

#include <type_traits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>
#include <initializer_list>

namespace digraph
{

template<typename T>
struct TD;

template<typename T>
class default_node_getter;

namespace detail
{
template<typename node_t>
struct node_type_impl
{
    using type = std::remove_cv_t<node_t>;
    using is_value_type_t = std::true_type;
    using underlying_type = type;
};

template<typename node_t>
struct node_type_impl<node_t&>
{
    using type = std::reference_wrapper<node_t>;
    using is_value_type_t = std::false_type;
    using underlying_type = std::remove_cv_t<node_t>;
};

template<typename edge_t, class node_getter >
struct node_type
{
    using ret_type = decltype(std::declval<node_getter>().begin( std::declval<edge_t>() ));
    using ret_type_control = decltype(std::declval<node_getter>().end( std::declval<edge_t>() ));
    static_assert(std::is_same < ret_type, ret_type_control>::value, "Edge begin end getter type mismatch");
    using type = typename node_type_impl<ret_type>::type;
    using underlying_type = typename node_type_impl<ret_type>::underlying_type;
    using is_value_type_t = std::false_type;
};


template<typename edge_t, class node_getter = default_node_getter<edge_t> >
using node_type_t = typename node_type<edge_t, node_getter>::type;

template<typename edge_t, class node_getter = default_node_getter<edge_t> >
using underlying_type_t = typename node_type<edge_t, node_getter>::underlying_type;


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
    return do_derefence( t.get() );
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

template<typename T>
class default_node_getter
{
public:
    static decltype(auto) begin( const T& t )
    {
        return t.begin();
    }
    static decltype(auto) end( const T& t )
    {
        return t.end();
    }
};


template<
    typename edge_t,
    class node_getter = default_node_getter<edge_t>,
    class edge_hash = detail::hash_t<edge_t>,
    class edge_equal_to = detail::equal_to_t<edge_t>,
    class node_hash_ = detail::hash_t<detail::node_type_t<edge_t, node_getter>>,
    class node_equal_to_ = detail::equal_to_t<detail::node_type_t<edge_t, node_getter>>
>
class digraph
{
public:
    using node_hash = node_hash_;
    using node_equal_to = node_equal_to_;
    using node_type = detail::node_type_t<edge_t, node_getter>;
    using underlying_node_type = detail::underlying_type_t<edge_t, node_getter>;
    using edge_container = std::unordered_set<edge_t, edge_hash, edge_equal_to>;
    using node_container = std::unordered_set<node_type, node_hash, node_equal_to>;
    using node_iterator = typename node_container::iterator;
    using node_const_iterator = typename node_container::const_iterator;
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
    node_container& nodes() noexcept;
    const edge_container& edges() const noexcept;
    const node_container& nodes() const noexcept;


    /// Found if ret != edges().end()
    node_const_iterator find( const underlying_node_type& _node ) const;
    edge_const_iterator find( const underlying_node_type& _from, const underlying_node_type& _to ) const;

    //TODO: add iterator
    //TODO: add shortest path

private:
    struct hash_node_iterator
    {
        template<typename T>
        using underlying_t = detail::deref_t< typename T::value_type >;

        //using has_ret_type = decltype(std::declval<node_hash>()(std::declval<underlying_t>()));
        using has_ret_type = size_t;

        has_ret_type operator()( node_const_iterator i ) const
        {
            return node_hash()(detail::do_derefence( *i ));
        }
    };

    struct equal_to_node_iterator
    {
        bool operator()( node_const_iterator lhs, node_const_iterator rhs ) const
        {
            return node_equal_to()(detail::do_derefence( *lhs ), detail::do_derefence( *rhs ));
        }
    };

    using graph_val_t = std::unordered_map<node_const_iterator, edge_const_iterator, hash_node_iterator, equal_to_node_iterator>;
    using graph_repr_t = std::unordered_map<node_const_iterator, graph_val_t, hash_node_iterator, equal_to_node_iterator>;

    static graph_repr_t build_graph( const edge_container&, const node_container& );
    void rebuild_graph();

    edge_container     _edges;
    node_container     _nodes;
    graph_repr_t    _g;
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
        using node_ptr = typename type::node_iterator;
    };
    template<typename... T>
    struct get_digraph_type<const digraph<T...>>
    {
        using type = const digraph<T...>;
        using node_ptr = typename type::node_const_iterator;
    };
    using node_ptr = typename get_digraph_type<DiGraphT>::node_ptr;
    using graph_ptr = std::add_pointer_t<typename get_digraph_type<DiGraphT>::type>;
public:
    explicit digraph_iterator( node_ptr n = nullptr, graph_ptr g = nullptr ) : _node( n ), _g( g ) {}
    digraph_iterator( const digraph_iterator & ) = default;
    digraph_iterator& operator= ( const digraph_iterator & ) = default;

private:
    node_ptr _node;
    graph_ptr _g;
};

///////////////////////
///////////////////////
///////////////////////

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::digraph( const std::initializer_list<edge_t>& list ) : digraph( list.begin(), list.end() )
{}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::digraph( const digraph & other ) :
    _nodes( other._nodes ), _edges( other._edges ), _g( build_graph( _nodes, _edges ) )
{}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::digraph( digraph && other ) noexcept :
    _nodes( std::move( other._nodes ) ), _edges( std::move( other._edges ) ), _g( std::move( other._g ) )
{}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::operator=( const digraph & other ) -> digraph &
{
    using std::swap;
    auto nodes = other._nodes;
    auto edges = other._edges;
    auto g = build_graph( edges, nodes );
    swap( _edges, edges );
    swap( _nodes, nodes );
    swap( _g, g );
    return *this;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::operator=( digraph && other ) noexcept -> digraph &
{
    using std::swap;
    if (this != &other) {
        _edges = std::move( other._edges );
        _nodes = std::move( other._nodes );
        _g = std::move( other._g );
    }
    return *this;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline void digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::swap( digraph & other ) noexcept
{
    swap( _edges, other._edges );
    swap( _nodes, other._nodes );
    swap( _g, other._g );
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash_, class node_equal_to_>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash_, node_equal_to_>::edges() const noexcept -> const edge_container &
{
    return _edges;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash_, class node_equal_to_>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash_, node_equal_to_>::nodes() const noexcept -> const node_container &
{
    return _nodes;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash_, class node_equal_to_>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash_, node_equal_to_>::edges() noexcept -> edge_container &
{
    return _edges;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash_, class node_equal_to_>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash_, node_equal_to_>::nodes() noexcept -> node_container &
{
    return _nodes;
}


template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash_, class node_equal_to_>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash_, node_equal_to_>::
find( const underlying_node_type & _from, const underlying_node_type & _to ) const -> edge_const_iterator
{
    const auto from = _nodes.find( node_type( _from ) );
    const auto to = _nodes.find( node_type( _to ) );
    if (from == _nodes.end() || to == _nodes.end())return _edges.end();
    const auto g_from = _g.find( from );
    if (g_from == _g.end())return _edges.end();
    const auto g_to = g_from->second.find( to );
    if (g_to == g_from->second.end())return _edges.end();
    return g_to->second;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::find( const underlying_node_type& _node ) const -> node_const_iterator
{
    return _nodes.find( node_type( _node ) );
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline auto digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::
build_graph( const edge_container& edges, const node_container& nodes ) -> graph_repr_t
{
    graph_repr_t g;
    for (auto edge_it = edges.begin(); edge_it != edges.end(); ++edge_it) {
        auto begin = std::find_if( nodes.begin(), nodes.end(),
            [&]( const auto& n ) { return node_equal_to()(n, node_getter::begin( *edge_it )); } );
        assert( begin != nodes.end() );
        auto i = g.emplace( begin, graph_val_t{} );
        auto end = std::find_if( nodes.begin(), nodes.end(),
            [&]( const auto& n ) { return node_equal_to()(n, node_getter::end( *edge_it )); } );
        assert( end != nodes.end() );
        i.first->second.emplace( end, edge_it );
    }
    return g;
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
inline void digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::rebuild_graph()
{
    _g = build_graph( _edges, _nodes );
}

template<typename edge_t, class node_getter, class edge_hash, class edge_equal_to, class node_hash, class node_equal_to>
template<typename InputIterator>
inline digraph<edge_t, node_getter, edge_hash, edge_equal_to, node_hash, node_equal_to>::digraph( InputIterator begin, InputIterator end ) :
    _edges( detail::to_unique_set<edge_t, edge_hash, edge_equal_to>( begin, end ) )
{
    std::for_each( _edges.begin(), _edges.end(), [this]( const auto& e ) {
        _nodes.emplace( node_getter::begin( e ) );
        _nodes.emplace( node_getter::end( e ) );
    } );
    _nodes = detail::to_unique_set<node_type, node_hash, node_equal_to>( _nodes.begin(), _nodes.end() );
    rebuild_graph();
}



}  //namespace digraph

#endif  //DIGRAPH_DIGRAPH_H_INCLUDED__
