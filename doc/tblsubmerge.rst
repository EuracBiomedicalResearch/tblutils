=========================================
tblsubmerge: Merge previously split cells
=========================================

For a certain column, condense rows with identical column content into a single
row, removing duplicates on a per column level. This tool is meant to perform
the inverse action as `tblsubsplit - Split cells by a non-column separator
character <tblsubsplit.html>`__.

Input
=====

A column which is used as an marker for which rows to condense. Rows belonging
to cells with identical strings will be joined: Each row's columns will be
folded into a single cell, removing duplicates and joining the entries with a
separator character. The order of the joined rows is undefined.

Options applicable to more than a single tool are summarized in `common command
line options <common_cmd_line_options.html>`__.

Output and example
==================

The output file from `tblsubsplit - Split cells by a non-column separator
character <tblsubsplit.html>`__, ``/tmp/split-index.tsv``, is reunited with the
command::

  $ ./tblsubmerge.pl -H -s , -u 3 /tmp/split-index.tsv

to yield the original file with the exception that white space characters have
been removed::

  CG number  Transformant ID
  CG32152    34270,34271
  CG32158    45641,45643,8490
  CG32154    37275
  CG32155    5279
  CG32150    21668,21669
