============================================================
tblsubsplit: Split cells by a non-column separator character
============================================================

This tool splits subcells in a tabular text file using another inner delimiter,
producing a normalized file with multiple rows (one per each subcell). Expanded
cells are written as sorted rows. Optionally, it produces a unique index column
to merge the results back unambiguously, see the corresponding tool
`tblsubmerge - Merge previously split cells <tblsubmerge.html>`__. If needed,
white space is stripped from the resulting strings.

Input
=====

A column with text elements separated by a distinguished character.

Options applicable to more than a single tool are summarized in `common
command line options <common_cmd_line_options.html>`__.

Output example
==============

As an example we split multiple comma-separated Drosophila transformant IDs in
column two of the following file in ``/tmp/split.tsv``::

  CG number   Transformant ID
  CG32158     8490, 45641, 45643
  CG32155     5279
  CG32154     37275
  CG32152     34270, 34271
  CG32150     21668, 21669

with the following command::

  $ ./tblsubsplit.pl -H -n 2 -s , -w -u Index /tmp/split.tsv

into the lines below::

  CG number   Transformant ID   Index
  CG32158     45641             1
  CG32158     45643             1
  CG32158     8490              1
  CG32155     5279              2
  CG32154     37275             3
  CG32152     34270             4
  CG32152     34271             4
  CG32150     21668             5
  CG32150     21669             5
