tblmerge performs either a full or inner join of two CSV tables, comparing
common columns and/or guess possible relationships between any column.

Theory of operation
-------------------

tblmerge can be used to merge (the correct term is 'join') two tables which
share a common ID (called 'key'), in a way similar to the common use of
*VLOOKUP* in Excel.

The 'key' in the first table is looked up in the second table independently of
the row. All additional columns present in the second table are appended to the
same row in the resulting table. All columns which share the same name (if any)
are also compared for equality, ensuring the data to be merged is the same.

The result is a single table, with exactly a single ID for each row, containing
columns from both files. Command-line switches are provided to control whether
only common IDs should be included in the output or not. The output is always
exact: no numerical alterations are ever performed.

There are many other options to perform common tasks during a merge operation
in order to minimize user errors.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__. tblmerge does not support
opening the standard input.

Command line flags
------------------

tblmerge can be launched on the calculation servers as follows:

`` $ tblmerge [options] 'key column name' file1 file2 > output-file``

*[options]* can contain any of the following command line switches:

-  *-c*: Only include common records in the output file (IDs with corresponding
   pair in the second table). Without this option, all records are instead
   included.

-  *-t*: Remove leading and trailing spaces from *all* cells. This improves the
   comparison and the resulting table if some cells only differ by useless
   whitespace.

-  *-n*: Normalize all numbers: remove excess zeroes, trailing dots and padding
   from cells containing *only* a single number. The number itself is **not**
   manipulated (no rounding is involved). This improves the comparison and the
   resulting table if some columns are simply formatted differently.

-  *-1* or *-2*: When a difference between the two tables is found, *-1*
   instructs tblmerge to copy the result from the first table (the first file
   specified on the command line) while *-2* copies from the second. If neither
   *-1*, *-2* or *-i* is used (and a difference is found), tblmerge stops with
   an error. Also see *-i*.

-  *-g N*: Guess for possible column pairs: columns having the same data across
   the two files but different labels. *-g N* compares the first *N* rows of
   the file, shows the list of probable pairs and then outputs a sequence of
   commands that can be used to merge them by using *-L* subsequently. If *-g0*
   is specified, all the rows are used. See `#5: guess
   pairs <#5:_guess_pairs>`__ for a detailed description.

-  *-v*: Be more verbose. If two duplicated IDs are found in the same table,
   but the rows are identical, no output is usually shown. This switch
   instructs tblmerge to moan about duplicated entries.

-  *-k*: If an error condition is encountered, try to keep going as far as
   possible in order to show all errors in one run.

-  *-i*: Ignore empty cells for comparisons; always take the non-empty cell as
   the reference.

-  *-L expr*: When loading the tables, process each **label** through the
   specified Perl expression. Most 'sed' expressions work unchanged as long as
   you use '$N' for back-references instead of '\\N'. You must know how to
   program in Perl or 'sed' to use this feature. Substitution is performed
   after the *-E expr* expression.

-  *-C expr*: When loading the tables, process each **cell** through the
   specified Perl expression. Substitution is performed after the *-E expr*
   expression.

-  *-E expr*: When loading the tables, process **all cells and labels** through
   the specified Perl expression.

-  *-h*: Show an help summary.

The flags cannot be repeated more than once. You can combine more *-L* or *-E*
expressions by using ';' in the expression itself (it's a Perl one-liner
after-all).

Main passes
-----------

`right <image:Tblmerge.png>`__

1: all cells expressions
~~~~~~~~~~~~~~~~~~~~~~~~

Each cell content is processed immediately after loading the file.

If the *-t* option is used, all leading and trailing spaces are removed.

If the *-n* option is used, and the cell contains only a single number, the
number is reformatted in *normalized* form: formatting elements such as spaces
or padding, trailing zeros and dots are removed. The number is not altered: the
resulting output is always exact.

The contents can be furthermore altered by the use of the *-E expr* expression.

By reducing each cell in common form, comparison of the two tables can improve
significantly without the need of user intervention.

2: label and cells expressions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since the join and comparison operations happen as a result of columns having
the same name on the two files, it may become useful (if not vital) to process
the labels in order to eliminate unneeded differences such as capitalization,
or simply different aliases.

The *-L expr* expression can be used to alter the names of the columns in an
automated way without actually modifying the source files.

The *-C expr* expression can be used instead to alter the contents of the
columns prior to processing.

3: column collapse
~~~~~~~~~~~~~~~~~~

If a file contains more than single column with the same name (by either
mistake, duplication or as a result of a labels expression), the duplicated
column is compared against the left-most column and removed.

If any difference is found, tblmerge stops with an inconsistency error.

Empty columns can be ignored by using *-i* (so that an empty column does not
result in a difference by taking the value from the non-empty cell).

The *-k* switch can be used to ignore differences altogether and keep the
left-most column of each file only.

4: row collapse
~~~~~~~~~~~~~~~

All duplicated rows with the same ID are also automatically removed if all
other columns are otherwise identical.

If any difference is found, tblmerge stops with an inconsistency error.

The *-k* switch can be used to ignore differences and keep the first row
containing the duplicated ID.

5: guess pairs
~~~~~~~~~~~~~~

Shared columns across the two tables can be guessed regardless of the labels.

This may come in handy if the tables have a number of columns with translated
labels, possibly different typing and/or you just need to check if there is any
possible correlation in a fully automated way.

Guessing happens pairwise, only if the *-g N* option is used, and only for the
first *N* records.

First, the 'key' in the first table is looked up, and all relating columns in
the second table are brought together. All column combinations are then
compared pairwise to each other. A score is calculated by the ratio of
perfectly equal values against compared records. Columns already having the
same name are not considered.

A list of column pairs having at least 50% of equal values in common is then
shown, along with the score and the 'labels expression' eventually required to
perform the rename automatically.

Execution stops after guessing: inspect the results and run tblmerge again if
needed.

As before, numerical values are compared exactly: if the -n option is not used,
differences in formatting could hamper the guesswork accuracy. There is
currently no support for *fuzzy matching* (comparing against slightly different
values), however it can be implemented quite easily if requested.

6: table lookup
~~~~~~~~~~~~~~~

For each row in the first table, the 'key' column is extracted and the ID is
cross-referenced on the second table, much in the same way as in Excel's
*VLOOKUP* function.

The *-c* option controls whether unique IDs in each table do result in the
final output.

By default, **all** IDs are included. Missing columns simply result as empty.
If *-c* is specified, only IDs present in both tables are included.

7: comparison
~~~~~~~~~~~~~

All columns having the same name are compared for equality. If a difference is
found, tblmerge simply stops with an error showing the difference.

The *-1* and *-2* switches instruct tblmerge to continue the copy by using the
first (or second) table as the reference value.

8: merge
~~~~~~~~

After all comparisons are performed, the final table is then assembled with
columns from both files. The order of the rows and the columns is *not*
preserved (rows and columns may be rearranged by tblmerge).

Usage examples
--------------

Simple merge of two files
~~~~~~~~~~~~~~~~~~~~~~~~~

Consider the following tables:

-  Table 1:

| `` Barcode Sex Trait1``
| `` B1      m   1.234``
| `` B2      f   4.321``
| `` B3      m   1.234``

-  Table 2:

| `` Barcode Sex Trait2``
| `` B1      m   4.321``
| `` B2      f   1.234``
| `` B3      m   4.321``

The tables have both the Barcode and sex in common. You obviously want to merge
the two tables together, check that the Sex is the same, and have both Trait1
and Trait2 on the same line. Run tblmerge as follows:

`` $ tblmerge Barcode table1.txt table2.txt > result.txt``

The resulting table is now:

| `` Barcode Sex Trait1 Trait2``
| `` B1      m   1.234  4.321``
| `` B2      f   4.321  1.123``
| `` B3      m   1.234  4.321``

Limits
------

tblmerge is memory hungry. Provide for 40 times the size of files being merged
of free memory. On shell.gm.eurac.edu (with 32GB or memory), you can
approximatively merge for a total of 800MB.

tblmerge2 is a faster alternative to tblmerge if you just need to perform full
join operations. tblmerge2 has no inherent memory limit.
