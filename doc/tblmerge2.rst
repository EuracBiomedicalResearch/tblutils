tblmerge2 performs a full join of two CSV tables, comparing common row/columns
for differences as it operates. tblmerge2 is a faster/simplified alternative to
tblmerge that only performs a simple full join of two (or more) files.

Theory of operation
-------------------

tblmerge2 can be used to merge (the correct term is 'join') two tables which
share a common ID (called 'key'), in a way similar to the common use of
*VLOOKUP* in Excel.

The 'key' in the first table is looked up in the second table independently of
the row. All additional columns present in the second table are appended to the
same row in the resulting table. All columns which share the same name (if any)
are also compared for equality, ensuring the data to be merged is the same.

The 'key' (as opposed to tblmerge) can be comprised of several columns to form
complex, unique indexes between files. Each column name should be separated by
a comma (eg: *``column1,column2,...``*; use backslashes to escape commas in
column names).

The result is a single table, with exactly a single ID for each row, containing
columns from both files.

Additionally, tblmerge2 allows to merge more than two files in a single
operation.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__. tblmerge2 does not support
opening the standard input.

Command line flags
------------------

tblmerge2 can be launched on the calculation servers as follows:

`` $ tblmerge2 [options] 'key column name' file1 file2 [file3...] > output-file``

*[options]* can contain any of the following command line switches:

-  *-h*: Show an help summary.

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
and Trait2 on the same line. Run tblmerge2 as follows:

`` $ tblmerge2 Barcode table1.txt table2.txt > result.txt``

The resulting table is now:

| `` Barcode Sex Trait1 Trait2``
| `` B1      m   1.234  4.321``
| `` B2      f   4.321  1.123``
| `` B3      m   1.234  4.321``

Complex merge of two files
~~~~~~~~~~~~~~~~~~~~~~~~~~

Consider the following tables:

-  Table 1:

| `` Study Barcode Trait1``
| `` A1    B1      1.234``
| `` A1    B2      4.321``
| `` A2    B1      1.234``
| `` A2    B2      4.321``

-  Table 2:

| `` Study Barcode Trait2``
| `` A1    B1      4.321``
| `` A1    B2      1.234``
| `` A2    B1      4.321``
| `` A2    B2      1.234``

The tables should be merged on *both* ``Study`` and ``Barcode``, since
``Barcode`` is not an unique index anymore. You can use a complex index for
this, as follows:

`` $ tblmerge2 Study,Barcode table1.txt table2.txt > result.txt``

The resulting table is now:

| `` Study Barcode Trait1 Trait2``
| `` A1    B1      1.234  4.321``
| `` A1    B2      4.321  1.123``
| `` A2    B1      1.234  4.321``
| `` A2    B2      4.321  1.123``
