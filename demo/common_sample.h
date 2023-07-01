// Copyright Sebastien Kramm, 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file
\brief This file is used only to provide some additional code to build the provided samples.
Holds some helper functions to deal with loading, saving, string handling, Dot file printing, etc.

Is not really part of the library.

Home page: https://github.com/skramm/udgcd
*/

#ifndef HG_COMMON_SAMPLE_H
#define HG_COMMON_SAMPLE_H

#include <boost/version.hpp>
#include "boost/graph/graphviz.hpp"
#include "boost/graph/graph_utility.hpp"
#include <boost/graph/connected_components.hpp>


#define UDGCD_DO_CYCLE_CHECKING
//#define UDGCD_LOG_FUNC
//#define UDGCD_PRINT_STEPS
//#define UDGCD_DEV_MODE

#include "../udgcd.hpp"

#include <string>
#include <numeric>

#define SHOW_INFO \
	std::cout << "-START: " << __FILE__ \
		<< "\n-built with Boost " << BOOST_LIB_VERSION << '\n'

extern std::string prog_id; // allocated in samples files
int g_idx = 0;

//%%%%%%%%%%%%%%%%%%%%%%%%
/// Holds some helper types and functions used in sample code
namespace sample {
//%%%%%%%%%%%%%%%%%%%%%%%%

//-------------------------------------------------------------------
/// Used to store a vertex position, if the input dot file specifies it
/// This is used in detail::printVertices() and in loadGraph_dot()
struct NodePos
{
	bool  hasLocation = false;
	float x = 0.f;
	float y = 0.f;
};

std::ostream&
operator << ( std::ostream& f, NodePos np )
{
	f << "NodePos: ";
	if( np.hasLocation )
		f << np.x << "," << np.y;
	else
		f << "(undef)";
	f << '\n';
	return f;
}

/// This is the type that will be embedded in the graph's typedef, so
/// we only have one member to fetch (dynamic properties code).
struct NodeData
{
	NodePos pos;
};

//-------------------------------------------------------------------
/// Prints some details on graph and returns nb of expected cycles
/**
(assumes that no two vertices have two edges that join them !)

See https://en.wikipedia.org/wiki/Dense_graph
*/
template<typename graph_t>
size_t
printGraphInfo( const graph_t& g )
{
	auto v = boost::num_vertices(g);
	auto e = boost::num_edges(g);

	std::cout << "Graph info:"
		<< "\n -nb of vertices=" << v
		<< "\n -nb of edges=" << e
		<< "\n -density=" << 1.0 * e / v / (v-1);

	std::vector<size_t> component( v );
	auto nb_cc = boost::connected_components( g, &component[0] );
	auto nb_cycles = e -  v + nb_cc;
	std::cout  << "\n -nb graphs=" << nb_cc
		<< "\n  => nb cycles expected=" << nb_cycles << '\n';
	return nb_cycles;
}
//-------------------------------------------------------------------
#if 0
void
CallDot( std::string id_str )
{
	std::system(
		std::string(
//			"dot -Tsvg -Grankdir=LR -Nfontsize=24 "
			"dot -Tsvg -Nfontsize=24 out/"
			+ id_str
			+ ".dot > out/"
			+ id_str
			+ ".svg"
		).c_str()
	);
}
//-------------------------------------------------------------------
std::string
BuildDotFileName()
{
	return "out/plot_" + prog_id + "_" + std::to_string(g_idx);
}
#endif

#define THROW_ERROR( msg ) \
	{ \
		std::cerr << "udgcd: " << __FILE__ << '-' << __FUNCTION__ << ": " << msg << std::endl; \
		throw; \
	}


//-------------------------------------------------------------------
/// Define a type alias \c VBundle
template <
    typename Graph,
    typename Bundle =
        typename boost::property_map<Graph, boost::vertex_bundle_t>::type
	>
using VBundle = typename boost::property_traits<Bundle>::value_type;

//-------------------------------------------------------------------
/// A type that checks if the graph uses the type \c NodePos as vertex property
template <typename Graph>
using HasVertexProp = std::is_same<NodeData, VBundle<Graph> >;


//-------------------------------------------------------------------
/// Functor class to write attributes of vertices in dot file. Related to \ref NodeData
template <typename T>
class NodeWriter
{
	public:
		NodeWriter( T data ) : _data(data)
		{}

		template <class vertex_t>
		void operator()( std::ostream& out, const vertex_t& vert ) const
		{
			if( _data[vert].hasLocation )
				out << " [pos=\"" << _data[vert].x << "," << _data[vert].y  << "!\"]";
		}
	private:
		T _data;
};

//-------------------------------------------------------------------
/// Helper function
template <typename T>
NodeWriter<T>
make_node_writer( T data )
{
	return NodeWriter<T>( data );
}

//%%%%%%%%%%%%%%%%%%%%%%%%
/// Holds some "detail" code
namespace detail {
//%%%%%%%%%%%%%%%%%%%%%%%%

//-------------------------------------------------------------------
/// Print vertices in file \c f, for graphs having the \c NodePos as vertex property.
/// See printVertices( std::ofstream& f, const Graph_t& gr )
template <typename Graph_t>
void printVertices( std::ofstream& f, const Graph_t& gr, std::true_type )
{
//	print_graph(g, std::cout << "Graph with VertexProp bundle: ");
	for( auto p_vert = boost::vertices( gr ); p_vert.first != p_vert.second; p_vert.first++ )
	{
		f << *p_vert.first;
		if( gr[*p_vert.first].pos.hasLocation )
			f << " [pos=\""
				<< gr[*p_vert.first].pos.x
				<< ','
				<< gr[*p_vert.first].pos.y
				<< "!\"]";

		f << ";\n";
	}
	f << "\n";
}

/// Print vertices in file \c f, for general type graphs (i.e. NOT having the \c NodePos as vertex property)
/// See printVertices( std::ofstream& f, const Graph_t& gr )
template <typename Graph_t>
void
printVertices( std::ofstream& f, const Graph_t& gr, std::false_type )
{
//	print_graph(g, std::cout << "Graph with other/missing properties: ");
	for( auto p_vert = boost::vertices( gr ); p_vert.first != p_vert.second; p_vert.first++ )
		f << *p_vert.first << ";\n";
	f << "\n";
}

//-------------------------------------------------------------------
/// Call boost::write_graphviz for graphs having the \c NodePos vertex type
template<typename Graph_t>
void
callGraphiz( std::ostream& f, const Graph_t& gr, std::true_type )
{
	boost::write_graphviz( f, gr, make_node_writer( boost::get( &NodeData::pos, gr ) ) );
}

//-------------------------------------------------------------------
/// Call boost::write_graphviz for graphs NOT having the \c NodePos vertex type
template<typename Graph_t>
void
callGraphiz( std::ostream& f, const Graph_t& gr, std::false_type )
{
	boost::write_graphviz( f, gr );
}

//%%%%%%%%%%%%%%%%%%%%%%%%
} // namespace detail
//%%%%%%%%%%%%%%%%%%%%%%%%


//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph
template<typename Graph_t>
void
renderGraph( const Graph_t& gr, const std::string id_str )
{
/*	std::string id_str;
	if( !name )
		id_str = BuildDotFileName();
	else
		id_str = *name;*/
//	std::cout << "id_str=" << id_str << '\n';
	{
		std::string fname( "out/" + id_str + "_" + std::to_string(g_idx) + ".dot" );
		std::ofstream f ( fname );
		if( !f.is_open() )
			THROW_ERROR( "unable to open file" + fname );

// Calls detail::callGraphiz(), using tag dispatching on \ref HasVertexProp
		detail::callGraphiz( f, gr, HasVertexProp<Graph_t>{} );
	}
//	CallDot( id_str );
	g_idx++;
}

/// Renders graph in a .dot file but with edges part of cycles with some random color
/**
\note This could probably be done using the boost::write_graphviz() overload that takes as argument
a dynamic property map.
Will require diving into that.

See https://graphviz.gitlab.io/_pages/doc/info/attrs.html for Dot details
*/
template<typename Graph_t,typename Vertex_t>
void
renderGraph2( const Graph_t& gr, const std::vector<std::vector<Vertex_t>>& cycles, const std::string id_str )
{
	int nbColors = std::min( 32, (int)cycles.size() );

	int bi = (int)std::ceil( std::log(nbColors) / std::log(2) );
	bi = std::max(2,bi);

// build color set
	std::vector<std::string> color_set( nbColors );
	for( size_t i=0; i<cycles.size(); i++ )
	{
		int r = 255 * (i%bi)    / (bi-1);
		int g = 255 * ((i/bi)%bi) / (bi-1);
		int b = std::max( 0, std::min(384-r-g,255) );
		std::ostringstream oss;
//		oss << " [penwidth=\"2.0\";color=\"#" << std::setfill('0');
		oss << "\"#" << std::setfill('0');
		oss << std::hex
			<< std::setw(2) << r
			<< std::setw(2) << g
			<< std::setw(2) << b
//			<< "\"];\n";
			<< '"';
		color_set[i%nbColors] = oss.str();
//		std::cout << "col "  << i << ": " << color_set[i%nbColors] << std::endl;
	}

	auto v_VPV = udgcd::priv::convertCycles2VVPV<Vertex_t>( cycles );

	std::string fname( "out/" + id_str + "_" + std::to_string(g_idx) + ".dot" );

	std::ofstream f ( fname );
	if( !f.is_open() )
		THROW_ERROR( "unable to open file" + fname );
	f << "graph G {\n";

// Print vertices in file, dispatch to one of the two concrete functions in namespace \ref detail, using tag dispatch with \ref HasVertexProp
	detail::printVertices( f, gr, HasVertexProp<Graph_t>{} );

// First, output all the edges part of a cycle, with a given color
//  and store them in a set, so that we can know they have been drawned.
	std::set<udgcd::priv::VertexPair<Vertex_t>> pairSet;
	for( size_t i=0; i<v_VPV.size(); i++ )     // for each cycle
	{
		for( const auto& pair: v_VPV[i] )    // for each pair
		{
			auto v1 = pair.v1;
			auto v2 = pair.v2;
			f << v1 << "--" << v2
				<< " [penwidth=\"2.0\";color="
				<< color_set[i%nbColors]
				<< ";label="
				<< i
//				<< ";labelfontcolor="               // THIS DOES NOT WORK ?!?!?!!
//				<< color_set[i%nbColors]
				<< "]\n";

			pairSet.insert( pair );
		}
	}

// Second, add all the remaining edges that were not part of a cycle
	for( auto p_it=boost::edges(gr); p_it.first != p_it.second; p_it.first++ )
	{
		auto idx1 = boost::source( *p_it.first, gr );
		auto idx2 = boost::target( *p_it.first, gr );
		udgcd::priv::VertexPair<Vertex_t> p( idx1, idx2 );
		if( pairSet.find(p) == pairSet.end() )             // if not found in previous set
			f << p.v1 << "--" << p.v2 << ";\n";            // of pairs, then add it, using default style.
	}

	f << "}\n";
	g_idx++;
}


//-------------------------------------------------------------------
#if 0
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph,
/// with names of vertices in a provided external vector \c v_names ("external properties")
template<typename graph_t>
void
renderGraph2( const graph_t& g, std::vector<std::string>& v_names )
{
	std::string id_str = BuildDotFileName();
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz( f, g, boost::make_label_writer( &v_names[0] ) );
	}
	CallDot( id_str );
	g_idx++;
}
//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph, with printing of color attributes
/**
Note: involving dynamic properties implies that the graph is modified at present, thus the argument cannot be const !

See http://stackoverflow.com/questions/34160290/
*/
template<typename graph_t, typename vertex_t>
void
renderGraph3( graph_t& g )
{
	boost::dynamic_properties dp;
	dp.property( "color",   boost::get( &vertex_t::color_str, g ) );

//	dp.property( "node_id", boost::get( boost::vertex_index, g ) );
	dp.property( "node_id", boost::get( &vertex_t::node_name, g ) );

	std::string id_str = BuildDotFileName();
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz_dp( f, g, dp );
	}
	CallDot( id_str );
	g_idx++;
}
#endif

//-------------------------------------------------------------------
/// Tokenize with string as separator
/**
https://stackoverflow.com/questions/14265581/
*/
inline
std::vector<std::string>
splitString( const std::string& str, const std::string& delim )
{
	if( str.find( delim ) == std::string::npos )
		return std::vector<std::string>();   // if none, then return empty vector

    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if( pos == std::string::npos )
            pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if( !token.empty() )
            tokens.push_back(token);
        prev = pos + delim.length();
    }
    while( pos < str.length() && prev < str.length() );
    return tokens;
}

//-------------------------------------------------------------------
/// General string tokenizer, taken from http://stackoverflow.com/a/236803/193789
/**
- see also this one: http://stackoverflow.com/a/53878/193789
*/
inline
std::vector<std::string>
splitString( const std::string& s, char delim )
{
	std::vector<std::string> velems;
	std::stringstream ss( s );
    std::string item;
    while( std::getline( ss, item, delim ) )
        velems.push_back(item);
    return velems;
}
//-------------------------------------------------------------------
/// Remove spurious spaces (or other character) at beginning and end
std::string
trimString( std::string in, char c = ' ' )
{
	auto lamb = [c] ( std::string a, size_t& idx )    // lambda
	{
		while( a[idx] == c && idx < a.size() )
			idx++;
	};

	size_t idx1 = 0;
	lamb( in, idx1 );
	std::string s1 = in.substr( idx1 );
	std::reverse( s1.begin(), s1.end() );

	size_t idx2 = 0;
	lamb( s1, idx2 );
	std::string out = s1.substr( idx2 );

	std::reverse( out.begin(), out.end() );
	return out;
}
//-------------------------------------------------------------------
/// Read a graph in a DOT file
/**
\note Boost::graph provides a Graphviz/dot reader, but its usage requires to:
[quote]
build and link against the "boost_graph" and "boost_regex" libraries.
[/quote]
(See https://www.boost.org/doc/libs/1_72_0/libs/graph/doc/read_graphviz.html )
<br>
Thus, as we wan't to keep this "link-free", we do not use it.<br>
The counterpart is that we do not read the vertices/edges properties that can be given in a dot file,
\b except the "pos" property.<br>

\warning This is a minimal reader, don't expect any fancy features.

Limitation/features:  as we do not handle labels, the index in input file will be the BGL vertex index.<br>
Drawback: if a node is missing in input file, it will be in the generated graph, because BGL indexes are always
consecutives.

For example, this input DOT file:
\verbatim
graph G {
0--3;
}
\endverbatim
This will generate a graph of 4 vertices, with 0 and 3 connected and 1 and 2 unconnected.

Another example. This input DOT file:
\verbatim
graph G {
2;
5;
0--1;
}
\endverbatim
will generate a graph of 6 vertices (0 to 5), with only 0 and 1 connected.
*/
template<typename graph_t>
graph_t
loadGraph_dot( const char* fname )
{
	std::cout<< " - Reading file:" << fname << '\n';
	std::ifstream f( fname );
	if( !f.is_open() )
	{
		std::cerr << "Unable to open file '" << fname << "'\n";
		throw "Unable to open file";
	}

	size_t nb_lines     = 0;
	size_t nb_empty     = 0;

	std::map<size_t,NodePos> mapPos; // holds the position of node, if given

	size_t max_vert_idx = 0;
	std::vector<std::pair<size_t,size_t>> v_edges;
	do
	{
		std::string temp;
		std::getline( f, temp );
		nb_lines++;

		if( temp.empty() || temp.front() == '#' )          // if empty or comment
			nb_empty++;
		else                        // if NOT empty
		{
            auto trimmed = trimString( temp );         // remove unwanted spaces
            auto vs_spc = splitString( trimmed, ' ' );
            if( vs_spc.size() > 2 )
            {
                if( vs_spc[0] == "graph" )
                    std::cout << "graph name=" << vs_spc[1] << "\n";
            }

			if( trimmed.back() == ';' )   // if EOL, then remove that and proceed
			{
				auto s2 = trimmed.substr( 0, trimmed.size()-1 );
//				std::cout << "line=" << trimmed <<  ", s2=" << s2 << "\n";
				auto v_tok = splitString( s2, "--" );

				if( v_tok.size() == 0 )                      // vertex
				{
					NodePos nPos;
					bool pos_is_given = false;
					auto p1 = s2.find( "[" );                  // check for attributes
					if( p1 != std::string::npos )
					{
						auto p2 = s2.find( "]" );
//						std::cout << "p1=" << p1 << " p2=" << p2 << "\n";
						if( p2 == std::string::npos || p2 == p1+1 )
							throw "invalid line: " + s2;
						auto sv = trimString( s2.substr( 0, p1-1 ) );
						auto sa = trimString( s2.substr( p1+1, p2-p1-1 ) );
//						std::cout << "sv='" << sv << "' sa='" << sa << "'\n";
						auto v_att = splitString( sa, '=' );
						if( v_att.size() != 2 )
							throw "invalid attribute string, line=" + s2;
						if( v_att[0] != "pos" )
							throw "unknown attribute string, line=" + s2;
						auto val = trimString( v_att[1], '"' );
//						std::cout << "val1='" << val << std::endl;

						if( val.back() == '!' )                    // if pos has an exclamation
							val = val.substr( 0, val.size()-1 );   // mark, remove it
//						std::cout << "val2='" << val << "'\n";
						auto v_values = splitString( val, ',' );
						if( v_values.size() != 2 )
							throw "invalid attribute string, line=" + s2;

						pos_is_given = true;

						nPos.x = std::stof( v_values[0] );
						nPos.y = std::stof( v_values[1] );
						nPos.hasLocation = true;
//std::cout << np;
						s2 = sv;
					}
					auto idx = std::stoul(s2);
					if( max_vert_idx < idx )
						max_vert_idx = idx;

					if( pos_is_given )
						mapPos[ idx ] = nPos;

//					std::cout << "-max_vert_idx=" << max_vert_idx << "\n";
				}
				else                          // found a "--" in the line !
				{
					if( v_tok.size() == 2 )
					{
						auto v0 = std::stoul( v_tok[0] );
						auto v1 = std::stoul( v_tok[1] );
						if( v1<v0 )
							std::swap( v0, v1 );
						v_edges.push_back( std::make_pair( v0, v1 ) );
//						std::cout << "adding edge " << v0 << "-" << v1 << "\n";
					}
					else   // invalid line
						throw std::string(__FUNCTION__) + "(): error, invalid line: " + trimmed;
				}
			}
        }
	}
	while( !f.eof() );

// parse through set of pairs, and add the required vertices
	for( const auto& p: v_edges )
	{
		if( p.first > max_vert_idx )
			max_vert_idx = p.first;
		if( p.second > max_vert_idx )
			max_vert_idx = p.second;
	}
	std::cout << "FINAL: max_vert_idx=" << max_vert_idx << "\n";

// Final step: generate the graph and return it
   	graph_t gr;
	for( size_t i=0; i<=max_vert_idx; i++ )
	{
		auto idx = boost::add_vertex( gr );
		if( mapPos.count(idx) )                   // if found in map,
			gr[idx] = NodeData{ mapPos[idx] };    //  we store its position,
		else                                      // else, not.
			gr[idx] = NodeData{ NodePos() };
	}

	using vertex_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
	for( const auto& p: v_edges )
		boost::add_edge( (vertex_t)p.first, (vertex_t)p.second, gr );

	std::cout<< " - file info:"
		<< "\n  - nb lines=" << nb_lines
		<< "\n  - nb empty=" << nb_empty
		<< '\n';

	boost::print_graph( gr, std::cout );
	return gr;
}

//-------------------------------------------------------------------
/// Load graph from custom simple text format
template<typename graph_t>
graph_t
loadGraph_txt( const char* fname )
{
	graph_t g;

	std::cout<< " - Reading file:" << fname << '\n';
	std::ifstream f( fname );
	if( !f.is_open() )
	{
		std::cerr << "Unable to open file\n";
		throw "Unable to open file";
	}

	size_t nb_lines     = 0;
	size_t nb_empty     = 0;
	size_t nb_comment   = 0;

	{                            // read nb vertices
		std::string temp;
		std::getline( f, temp );
		auto v_tok = splitString( temp, ':' );
		if( v_tok.size() < 2 )
			throw "error 1st line";
		size_t nb = std::atoi( v_tok[1].c_str() );
//		std::cout << "nb vertices=" << nb << '\n';
		for( size_t i=0; i<nb; i++ )
			boost::add_vertex(g);
	}

	do
	{
		std::string temp;
		std::getline( f, temp );
		nb_lines++;

		if( temp.empty() )          // if empty
			nb_empty++;
		else                        // if NOT empty
		{
			if( temp.at(0) == '#' )  // if comment
				nb_comment++;
			else                     // if NOT comment
			{
				auto v_tok = splitString( temp, '-' );
				if( v_tok.size() < 2 )
				{
					std::cerr << "not enough items on line " << nb_lines << ": -" << temp << "-\n";
					throw ( "Invalid data on line: " + std::to_string( nb_lines ) );
				}
				int v1 = std::atoi( v_tok[0].c_str() );
				int v2 = std::atoi( v_tok[1].c_str() );
				boost::add_edge( v1, v2, g );
			}
		}
	}
	while( !f.eof() );

	std::cout<< " - file info:"
		<< "\n  - nb lines=" << nb_lines
		<< "\n  - nb empty=" << nb_empty
		<< "\n  - nb comment=" << nb_comment << '\n';

	return g;
}
//-------------------------------------------------------------------
template<typename vertex_t>
std::vector<size_t>
buildSizeHistogram( const std::vector<std::vector<vertex_t>>& cycles )
{
	std::vector<size_t> histo;
	for( const auto& c: cycles )
	{
		auto siz = c.size();
		if( histo.size() < siz-2 )
			histo.resize( siz-2 );
		histo.at( siz-3 )++;
	}

	return histo;
}

//-------------------------------------------------------------------
/// Process the graph \c g to find cycles
/// This function is called by the two apps: random_test.cpp and read_graph.cpp
/**
This function makes sure :
- that the correct number of cycles are found
- that the computed cycles are correct.

It will return a pair <int,cycles>
The first value is 0 in case of success, -1 if incorrect cycles were found.
If other value, it is the absolute number of differences between:
- the \b computed number of cycles
- and the \b expected number of cycles

*/
template<typename graph_t,typename vertex_t>
std::pair<int,std::vector<std::vector<vertex_t>>>
processGraph( graph_t& g )
{
//	using vertex_t = boost::graph_traits<graph_t>::vertex_descriptor;

	auto expected = printGraphInfo( g );

	udgcd::UdgcdInfo info;
	auto cycles = udgcd::findCycles<graph_t,vertex_t>( g, info );
//	udgcd::printPaths( std::cout, cycles, "final" );
	if( expected != cycles.size() )
		std::cout << "ERROR: computed nb of cycles is not what expected (expected=" << expected << ")\n";

//	std::cout << "diff=" << (int)expected - (int)cycles.size() << "\n";

	udgcd::priv::printStatus( std::cout, cycles, __LINE__ );

	auto check = udgcd::priv::checkCycles( cycles, g );
	if( check.first != 0 )
	{
		std::cout << "ERROR: " << check.first << " incorrect cycles found\n";
		return std::make_pair(-1, cycles );
	}
	if( check.second != 0 )
	{
		std::cout << "Found: " << check.second << " non chordless cycles\n";
	}

	info.print( std::cout );
//	info.printCSV( std::cerr );

	std::cout << "Histogram of cycle sizes:\n";
	auto histog = buildSizeHistogram( cycles );
	for( size_t i=0; i<histog.size(); i++ )
		std::cout << i+3 << ":" << histog[i] << "\n";

	auto diff = (int)cycles.size() - (int)expected;
	return std::make_pair(diff, cycles );
}
//-------------------------------------------------------------------

//%%%%%%%%%%%%%%%%%%%%%%%%
} // namespace sample
//%%%%%%%%%%%%%%%%%%%%%%%%

#endif // HG_COMMON_SAMPLE_H
