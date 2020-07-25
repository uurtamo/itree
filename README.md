# itree

This implements an interval tree which sits on top of an AVL tree, a la Cormen, et al, via augmenting the original tree with some additional information
at each node.  The original AVL tree code was written by Ian Piumarta in 2005.

Include the .h and go to town.

I'll drop in an example.c to demonstrate; it's efficient enough to use anywhere you need to do intersection testing and listing and where a single thread is enough.


