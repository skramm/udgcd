// Copyright Sebastien Kramm - 2015
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

#include <string>

#define SHOW_INFO \
	std::cout << "\n-START: " << __FILE__ \
		<< "\n-built with Boost " << BOOST_LIB_VERSION << '\n'

extern int prog_id; // allocated in samples files
int g_idx = 0;

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
/// with names of vertices in an external vector ( "external properties" )
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
