# Stripping cycles using a tree technique

The DFS algorithm used as first step will generate multiple duplicates of each cycle.
Thus the need to strip these before searching the minimum cycle basis.

For example

From 2023-07-16, a new technique has been implemented, that greatly improves computation time.
Before that, searching for dupes was done in a rather "brute force" way:
The cycles were stored one by one in an array, and before adding the next one, a search was conducted through the whole array to check if the cycle
was not already stored.

This is inneficient because whatever a given cycle, it will parse each time the whole array previously created, even thought the current cycle can be discarded.

Instead, the new approach






