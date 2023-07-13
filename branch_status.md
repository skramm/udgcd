Current branch status:

branch tree_stripping

2023-07-13

- failure with:
```
$ build/bin/read_graph samples/graph_V60_E73_A.txt
```
(infinite loop?)

- failure with:
```
$ build/bin/read_graph samples/samples/graph_50.txt
```
=> incorrect number of cycles found (6 instead of 7)

- failure with:
```
$ build/bin/read_graph samples/graph_V8_E14_1.txt
```
=> incorrect number of cycles found (5 instead of 7)


