==============================================================================
tblutils: Text-based Table utilities
==============================================================================

`tblutils` contains several utilities to work with tabular text files: files
containing tabular data written in plain text, with one row per line and
columns separated by a common character (usually TAB or semicolon).

`tblutils` complements the usual unix tools like ``cut`` and ``paste`` by
providing enhanced versions that support column labels through-out, so that you
can extract columns by name (``tblcut``), filter data using a mathematical
expression (``tblfilter``), re-order columns without caring about the column
index (``tblcsort``), join multiple files on a common index without having to
pre-sort them (``tblmerge``), and much more.


Toolset
-------

:tbl2excel: Generate Excel files directly from tabular text files, without
            causing the typical conversion errors that happen when opening a
            text file directly from Excel.
:tblabelize: Assign automatic labels (header) to an unlabeled tabular text file.
:tblunlabelize: Remove labels from a tabular text file file.
:tblcsort: Sort/reorder the columns of a tabular text file by name.
:tblcut: Extract columns from tabular text files by name.
:tblfilter: Filters rows of a tabular text file using column names and
            regular/mathematical expressions.
:tblmerge: Merge/compare two tabular text files together using a common
           index/column name.
:tblmerge2: Merge two or more tabular text files together using a common
            index/column name (faster C implementation).
:tblnorm: Normalize a tabular text file (reorders both columns and rows).
          Generally used to make two files easier to compare.
:tblsize: Get a tabular text file width (column) and height (row count).
:tbltransp: Transpose a tabular text file (flip columns/rows).
:tbltransp2: Transpose a tabular text file (faster C implementation).
:tbl2tbl: Convert CSV files to simple tabular, tab-separated text files (TSV)
          and back, fixing quoting/escaping along the way.
:tblsubsplit: Expand the contents of a column which contains multiple values
              into multiple rows, making it uniform and easier to process.
:tblsubmerge: Reverses the effect of `tblsubsplit`, merging values spanning
	      across multiple rows into a single line/cell containing all the
	      values.

Full documentation is available for each tool inside the ``doc/`` directory.


Installation
------------

On Debian/Ubuntu, after unpacking the sources, type::

  sudo apt-get install libtext-csv-xs-perl libexcel-writer-xlsx-perl libspreadsheet-writeexcel-perl
  sudo make install

to install all the required Perl modules and utilities to ``/usr/local/bin``.

On other systems (Mac, other linux listributions), you can use ``cpan`` to
install the required Perl modules instead::

  sudo cpan -i Text::CSV_XS Spreadsheet::WriteExcel Excel::Writer::XLSX
  sudo make install


Installation as a normal user
-----------------------------

You can install all the utilities as a normal user by executing::

  cpan -i Text::CSV_XS Spreadsheet::WriteExcel Excel::Writer::XLSX
  make PREFIX=$HOME/.local install

This will install all the utilities and Perl modules into your home directory
(inside ``~/.local/bin``). Be sure to add this directory to your ``PATH`` (if
not already present as a default) by running::

  PATH="$PATH:$HOME/.local/bin"

You can make this change permanent by adding the above line to your
``~/.profile`` script.


Usage
-----

You can change the column separator by setting the *TBLSEP* environment
variable as follows::

  export TBLSEP=';'

The default separator, if not specified, is TAB. If you use Excel to export
your data, you should use the save as type *"Text (Tab delimited) (\*.txt)"*
when doing *"Save as ..."* to create a file compatible with the table
utilities.

The command line flag ``-d`` (when supported) takes precedence over the
environment variable.

Files are read and written with the same separator. If you need to change the
separator, use ``tbl2tbl``.

All the tools assume that the first line of any file contains column labels.
You always refer to columns by labels, not numbers. Some tools support the
``-H`` flag to work with column numbers instead, in which case the first column
is always 1. You can (should) use ``tblabelize`` to assign labels to an
existing file.

You can open the standard input by using '-' instead of a named file. This is
not universally supported by all tools.


Notes
-----

`tblutils` was developed out of necessity during my work in bioinformatics.
Perhaps surprisingly, the foundation of science is not some fancy HDF file or a
complex database; but just a lousy, flat, loosely defined text file. In
genetics these files are of gigabytes in size, and usually come by the
thousands. A lot of time is dedicated to convert one text format to a slightly
different one, munge IDs to make them acceptable by some analysis software,
slice, dice, read and aggregate the results.

`tblutils` is a collection of tools that aim to complement the conventional
unix text-processing tools, allowing the user to focus on the data while still
allowing powerful scripting capabilities directly from the command line. We
refer to columns by label, not by number. We want to compare numbers with
mathematical operators, not regular expressions. By processing the files
line-by-line as opposed to loading them in full, these tools work with
unbounded file sizes, with pipes, with any compressed file format and can be
run in parallel with zero effort.

We tried to set some conventions, such as mimicking the traditional unix tools
when possible and re-use the same flags for equivalent meaning, but it's not
always the case. We will fix/improve the tools and documentation with time.
Sometimes multiple implementations are provided, making different
memory/disk/complexity trade-offs.

In addition to `tblutils`, we also recommend `csvtool
<http://forge.ocamlcore.org/projects/csv/>`_, which includes several other
complementary utilities to work with tabular text files, such as the ability to
extract sub-matrices from larger tables, performing column replacements, etc.


Authors and Copyright
---------------------

`tblutils` can be found at http://www.thregr.org/~wavexx/software/tblutils/

| `tblutils` is distributed under GPL2 (see COPYING) WITHOUT ANY WARRANTY.
| Copyright(c) 2008-2013 by wave++ "Yuri D'Elia" <wavexx@thregr.org>.
| Copyright(c) 2008-2010 EURAC, Institute of Genetic Medicine.

`tblutils`'s GIT repository is publicly accessible at
``git://src.thregr.org/tblutils``.
