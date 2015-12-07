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

extern int prog_id;

//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a svg image of the graph
template<typename Graphtype>
void RenderGraph( const Graphtype& g )
{
	static int idx=0;

	std::string id_str( "obj/sample" + std::to_string(prog_id) + "_" + std::to_string(idx) );
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz( f, g );
	}
	std::system(
		std::string(
			"dot -Tsvg -Grankdir=LR -Nfontsize=24 "
			+ id_str
			+ ".dot > "
			+ id_str
			+ ".svg"
		).c_str()
	);

	idx++;
}
//-------------------------------------------------------------------
