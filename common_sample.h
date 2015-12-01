

#include <boost/version.hpp>
#include "boost/graph/graphviz.hpp"

#include <string>

#define SHOW_INFO \
	std::cout << "START: " << __FILE__ \
		<< "\n-built with Boost " << BOOST_LIB_VERSION << '\n'

//-------------------------------------------------------------------
/// Generates a dot file from graph \c g and calls the renderer (dot/Graphviz) to produce a png image of the graph
template<typename Graphtype>
void RenderGraph( const Graphtype& g, int app_idx, int idx )
{
	std::string id_str( "obj/sample" + std::to_string(app_idx) + "_" + std::to_string(idx) );
	{
		std::ofstream f ( id_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz( f, g );
	}
	// the cast to void is there to avoid a warning about "unused return value".
	(void)std::system(
		std::string(
			"dot -Tpng -Gsize=\"6,6\" -Grankdir=LR -Nfontsize=24 "
			+ id_str
			+ ".dot > "
			+ id_str
			+ ".png"
		).c_str()
	);
}
//-------------------------------------------------------------------
