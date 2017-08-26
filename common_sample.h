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
#include <boost/graph/connected_components.hpp>

#include <string>

#define SHOW_INFO \
	std::cout << "\n-START: " << __FILE__ \
		<< "\n-built with Boost " << BOOST_LIB_VERSION << '\n'

extern int prog_id; // allocated in samples files
int g_idx = 0;

//-------------------------------------------------------------------
template<typename graph_t>
void PrintGraphInfo( const graph_t& g )
{
	std::cout << "Graph info:";
	std::cout << "\n -nb of vertices=" << boost::num_vertices(g);
	std::cout << "\n -nb of edges=" << boost::num_edges(g);
//	std::cout << "\n -nb of connected components=" << boost::con(g);

	std::vector<size_t> component( boost::num_vertices( g ) );
	auto nb_cc = boost::connected_components( g, &component[0] );
	std::cout  << "\n -nb graphs=" << nb_cc;
	std::cout  << "\n -nb cycles expected=" << boost::num_edges(g) -  boost::num_vertices(g) + nb_cc;
	std::cout << '\n';
}

//-------------------------------------------------------------------
void CallDot( std::string id_str )
{
	std::system(
		std::string(
			"dot -Tsvg -Grankdir=LR -Nfontsize=24 "
			+ id_str
			+ ".dot > "
			+ id_str
			+ ".svg"
		).c_str()
	);
}
std::string BuildIdString()
{
	return "obj/sample" + std::to_string(prog_id) + "_" + std::to_string(g_idx);
}

//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph
template<typename graph_t>
void RenderGraph( const graph_t& g )
{
	std::string id_str = BuildIdString();
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz( f, g );
	}
	CallDot( id_str );
	g_idx++;
}

//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph,
/// with names of vertices in a provided external vector \c v_names ("external properties")
template<typename graph_t>
void RenderGraph2( const graph_t& g, std::vector<std::string>& v_names )
{
	std::string id_str = BuildIdString();
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
void RenderGraph3( graph_t& g )
{

	boost::dynamic_properties dp;
	dp.property( "color",   boost::get( &vertex_t::color_str, g ) );

//	dp.property( "node_id", boost::get( boost::vertex_index, g ) );
	dp.property( "node_id", boost::get( &vertex_t::node_name, g ) );

	std::string id_str = BuildIdString();
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz_dp( f, g, dp );
	}
	CallDot( id_str );
	g_idx++;
}
//-------------------------------------------------------------------
