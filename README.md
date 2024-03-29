# udgcd (UnDirected Graph Cycle Detection)


C++ wrapper over Boost Graph Library (aka BGL), provides a mean to detect cycles in an undirected graph.

For example, with the following graph generated by the included sample program:

![alt](
https://github.com/skramm/udgcd/blob/master/sample1_2.png "sample graph")

This code will give you the three cycles as three sets of vertices.
These are sorted with the smallest vertex in first position, and such as the second vertices is always smaller than the last one (see [notes](#s_notes)):
```
1-6-5-4-3-2-
1-6-5-4-13-14-7-3-2-
3-7-14-13-4-
```

### Status
<a name="s_stat"></a>

- Home page: https://github.com/skramm/udgcd
- Author: Sebastien Kramm, firstname.lastname@univ-rouen.fr
- Latest news:
  - 2023-07-17: (**major update**) Algorithm improvment, now much more efficient, see [details here](misc/tree_stripping.md)
  - 2023-07-03: was reported to [build on Windows using VisualStudio](https://github.com/skramm/udgcd/issues/4#issuecomment-1611426339)
  - 2020-06-09: experimental code and preliminar release, source is pretty messy, but it works fine, give it a try (instructions below).
- Released under the Boost licence.

### Features

- Single-file header only library, OS agnostic
- Works for graphs holding unconnected sub-graphs.
- Works for non-planar graphs
- Modern C++ design (RAII), basic C++11.
- Fairly generic, should be suited for pretty much all types of undirected graphs, as long as you can [order the vertices](#s_notes).
- Intended audience: Any C++ app having a graph cycle detection issue and whose licence is compatible with the Boost licence.

### Usage in your own code:
<a name="s_usage"></a>

 1. Add `#include "udgcd.hpp"` in your application (all-in-one file).
 1. Create your graph ([check this](#s_notes)).
 1. Build your graph (add vertices and edges, see BGL manual or the provided samples).
 1. Call `udgcd::findCycles()`. It will return a set of paths that are cycles (1):

 `auto cycles = udgcd::findCycles<my_graph_type,my_vertex_type>( graph );`
 - Just to check, you may call `udgcd::printPaths()` to print out the cycles:

   `udgcd::printPaths( std::cout, cycles, "cycles" );`
 - Done !

(1) The type of the returned value is actually `std::vector<std::vector<my_vertex_type>>`

See included sample programs.


To check without writing any code, you can also try the program `read_graph.cpp`


### Building & installing & content:
<a name="s_build"></a>

- header only, no build. Provided as a single file (the other files are useless for basic user).
- To build & run the provided sample code, just use `make run`, no other dependency than BGL.
(tested with boost 1.70, let me know if you discover any inconsistency with later releases.)

#### Installing
Just fetch the file `udgcd.hpp` above and store it where you want. Or use the provided target of makefile (if you clone the whole repo):
`make install` (might require `sudo`).
This will copy the file in `/usr/local/include/`

#### What's in there beside that single header file ?

Some additional apps are included, that are build by the makefile:
 - `read_graph.cpp`: reads graph from a text file given as argument, computes its cycles, prints them and generate the corresponding dot file.
 - `random_test.cpp`: generates a random graph computes its cycles, prints them and generate the corresponding dot file.
 - `sample_?.cpp`: C++ apps that build a graph and computes its cycles.

#### Build options:
 - The provided makefile is not requested to use the library, as it is "header-only".
 It can be used as a demo. It has the following targets (for a full list, please enter `make help`):
  - `make` (no targets) : builds the included demos apps
  - `make run` : builds and runs all the included demo programs
  - `make runsam` : builds and runs the `read_graph.cpp` program and runs it on all provided data samples, in folder `samples/`
  - `make doc` : builds the doxygen reference file (needs doxygen installed). Useful if you want to dive into the code...


** Demos **

 - To run a single demo, run `build/bin/sample_X`.
 - To run more significant stuff, you can try:
 ```
 $ build/bin/random_test 15 25
 ```
This will generate a random graph with 15 nodes and 25 vertices, and will check for cycles. Print a lot of additional info.


The program `read_graph.cpp` (build an run by `make runsam`) will generate a dot file that can be rendered as an image with Graphviz.
So if Graphviz/Dot is installed, you can try `make svg`: this will call Graphivz on all the dot files in the `out` folder.

Some datafiles are included in the [samples folder](https://github.com/skramm/udgcd/tree/master/samples).
For example, this:
```
$ build/bin/read_graph samples/graph_0.txt
```
will produce these two dot files in the `out` folder:
```
graph_0_0.dot
graph_0_color_1.dot
```
The first one is the raw graph, the second holds the cycles expressed as additional edges.
They can be rendered graphically with `$ make svg`, that will produce theses two files:

![alt](misc/img/graph_0_0_neato.svg)
![alt](misc/img/graph_0_color_1_neato.svg)



### Issues:
 <a name="s_issues"></a>

- At present, this code requires a static allocated variable (done automatically by compiler, as it is templated).
Thus it is **not** thread safe, neither can it handle multiple graphs simultaneously.
- Complexity has not been quantified, but should be for sure more than the one of some most recent papers (see References).
- Some inputs will generate a high memory request, that may exceed the OS possibility and lead to a failure.

### How does it work ?
 <a name="s_inside"></a>

The algorithm involved here is pretty simple, but probably not very efficient, thus slow for large graphs.
Three steps are involved: first we need to check if there **is** at least one cycle.
Is this is true, we explore the graph to find it/them.
It can be considered as a variant of the [Horton Algorithm](https://epubs.siam.org/doi/10.1137/0216026).

- The first step is done by a Depth First Search (DFS), with  [boost::undirected_dfs()](http://www.boost.org/doc/libs/1_70_0/libs/graph/doc/undirected_dfs.html)
with passing a visitor of class `CycleDetector`, inherited from
[boost::dfs_visitor](http://www.boost.org/doc/libs/1_70_0/libs/graph/doc/dfs_visitor.html).
This object holds a set of vertices that are part of an edge on which `back_edge()` is called (called here "source vertices").
If this happens, it means that a cycle *has* been encountered.

- The second step is done by exploring recursively the graph, by starting from each of the sourece vertices.
This step is the most time-consuming.

- The third steps does some post-processing:
sort cycles by decreasing length, and do Gaussian Elimination to retain a Minimal Cycle Basis (MCB).

### Dependencies

- boost::graph - https://www.boost.org/doc/libs/1_82_0/libs/graph/doc/
(warning: boost graph itself has a lot of other boost dependencies)
- boost::dynamic_bitset https://www.boost.org/doc/libs/1_82_0/libs/dynamic_bitset/dynamic_bitset.html


### References
<a name="s_ref"></a>

- https://en.wikipedia.org/wiki/Cycle_basis
- J. D. Horton, <i>A polynomial-time algorithm to find a shortest cycle basis of a graph</i>, SIAM Journal of Computing 16, 1987, pp. 359–366
[link](https://epubs.siam.org/doi/10.1137/0216026).
- K. Mehlhorn, D. Michail, <i>Implementing Minimum Cycle Basis Algorithms</i>, ACM Journal of Experimental Algorithmics, Vol. 11, 2006, pp. 1–14.
- E. Amaldi, C. Iuliano, R. Rizzi, <i>Efficient Deterministic Algorithms for Finding a Minimum Cycle Basis in Undirected Graphs</i>, IPCO 2010, LNCS 6080, pp. 397–410.


### Notes
<a name="s_notes"></a>

#### Output normalizing
In the output vector, the paths can be sorted if the symbol `UDGCD_NORMALIZE_CYCLES` is defined.
This requires the `<` operator defined for the vertices.
Sorting is done such as:
- The smallest element is in first position;
- The element in second position is smaller than the last one.

As an example, say you have a raw cycle expressed as
   `6-2-1-4`, it is released as `1-2-6-4` (and not `1-4-6-2`).

<a name="note_bp"></a>
#### User properties & graph type

You can use whatever edge and vertices types, the coloring needed by the algorithm is handled by providing color maps as external properties.
So if you have no special needs on vertices and edge properties, you can use something as trivial as this:
```C++
    using graph_t = boost::adjacency_list<
	   boost::vecS,
	   boost::vecS,
	   boost::undirectedS
	>;
```
But you can also have some user properties, defines as bundled properties. For example:
```C++
struct my_Vertex
{
	int         v1;
	std::string v2;
	float       v3;
};
struct my_Edge
{
	int         e1;
	std::string e2;
};
```
Then your graph type definition will become:
```C++
	using graph_t = boost::adjacency_list<
		boost::vecS,                 // edge container
		boost::vecS,                 // vertex container
		boost::undirectedS,          // type of graph
		my_Vertex,                   // vertex type
		my_Edge
		>;
```
