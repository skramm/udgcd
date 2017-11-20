// Copyright Sebastien Kramm, 2016-2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file
\brief This file is used only to build the provided samples

Home page: https://github.com/skramm/udgcd

*/


#include <boost/version.hpp>
#include "boost/graph/graphviz.hpp"
//#include <boost/graph/connected_components.hpp>

#include <string>

#define SHOW_INFO \
	std::cout << "-START: " << __FILE__ \
		<< "\n-built with Boost " << BOOST_LIB_VERSION << '\n'

extern std::string prog_id; // allocated in samples files
int g_idx = 0;

//-------------------------------------------------------------------
/// Prints some details on graph and returns nb of expected cycles
/**
(assumes that no two vertices have two edges that join them !)
*/
template<typename graph_t>
size_t
PrintGraphInfo( const graph_t& g )
{
	std::cout << "Graph info:"
		<< "\n -nb of vertices=" << boost::num_vertices(g)
		<< "\n -nb of edges=" << boost::num_edges(g);

	std::vector<size_t> component( boost::num_vertices( g ) );
	auto nb_cc = boost::connected_components( g, &component[0] );
	auto nb_cycles = boost::num_edges(g) -  boost::num_vertices(g) + nb_cc;
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
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph
template<typename graph_t>
void
RenderGraph( const graph_t& g, const std::string id_str )
{
/*	std::string id_str;
	if( !name )
		id_str = BuildDotFileName();
	else
		id_str = *name;*/
	std::cout << "id_str=" << id_str << '\n';
	{
		std::string fname( "out/" + id_str + "_" + std::to_string(g_idx) + ".dot" );
		std::ofstream f ( fname );
		if( !f.is_open() )
			THROW_ERROR( "unable to open file" + fname );
		boost::write_graphviz( f, g );
	}
//	CallDot( id_str );
	g_idx++;
}
//-------------------------------------------------------------------
#if 0
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph,
/// with names of vertices in a provided external vector \c v_names ("external properties")
template<typename graph_t>
void
RenderGraph2( const graph_t& g, std::vector<std::string>& v_names )
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
RenderGraph3( graph_t& g )
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
/// General string tokenizer, taken from http://stackoverflow.com/a/236803/193789
/**
- see also this one: http://stackoverflow.com/a/53878/193789
*/
inline
std::vector<std::string>
split_string( const std::string &s, char delim )
{
	std::vector<std::string> velems;
	std::stringstream ss( s );
    std::string item;
    while( std::getline( ss, item, delim ) )
        velems.push_back(item);
    return velems;
}
//-------------------------------------------------------------------
template<typename graph_t>
graph_t
LoadGraph( const char* fname )
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
		auto v_tok = split_string( temp, ':' );
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
				auto v_tok = split_string( temp, '-' );
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
/// Process the graph to find cycles
/// This function is called by the two apps.
template<typename graph_t>
int
Process( graph_t& g )
{
	typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

	auto expected = PrintGraphInfo( g );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final" );
	if( expected != cycles.size() )
	{
		std::cout << "ERROR: computed nb of cycles is not what expected (expected=" << expected << ")\n";
		return 1;
	}
	return 0;
}
//-------------------------------------------------------------------
