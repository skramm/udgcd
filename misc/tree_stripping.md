# Stripping cycles using a tree technique

The DFS algorithm used as first step will generate multiple duplicates of each cycle.
Thus the need to strip these before searching the minimum cycle basis, to have only unique cycles to process.

From 2023-07-16, a new technique has been implemented, that greatly improves computation time.
Before that, searching for dupes was done in a rather "brute force" way:
The cycles were stored one by one in an array, and before adding the next one, a search was conducted through the whole array to check if the cycle
was not already stored.
This is inneficient because whatever a given cycle, it will parse each time the whole array previously created, even thought the current cycle can be discarded pretty quickly.

Instead, the new approach will build several trees, one for each (potential) starting node, and store the candidate cycle in these trees, *only if not found in tree*.
This is a form of indexing the cycles using the starting node as index.

The great advantage is that for non-stored cycles, we can stop as soon as we discover a discrepancy when parsing the tree.

For example, consider the [Petersen graph](https://en.wikipedia.org/wiki/Petersen_graph) ([included in samples](../samples/graph_Petersen.dot)).
Then we consider the candidate cycles one by one.

The first cycle we find is 0-1-2-3-4.
The tree #0 is empty, so we just store that cycle in it:

\image html tree_tmp_0_0_dot.svg


The second we consider is 0-1-2-7-5. We parse the tree-#0 up to node 2 and, discovering it has no child 7, we add the rest of the cycle to node 2:
\image html tree_tmp_0_2_dot.svg

Then, we consider the cycle 0-4-3-2-7-5. It is not present in the tree, so we store it:

\image html tree_tmp_0_3_dot.svg


This process iterates until all cycles are stored in the tree.
And any already present cycle in it will not be stored:

\image html tree_tmp_0_5_dot.svg






