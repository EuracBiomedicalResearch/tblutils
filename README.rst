==============================================================================
tblutils: Text-based Table utilities
==============================================================================

:tbl2excel: Generate Excel files from CSV/text files
:tblabelize: Assign labels to an unlabeled CSV file
:tblcsort: Sort the columns of a CSV file
:tblcut: Extract columns from CSV files by name
:tblfilter: Filter rows of a CSV file easily
:tblmerge2: Merge two CSV tables together (faster/simplified alternative that
	    can work on larger files)
:tblmerge: Merge/compare two CSV tables together
:tblnorm: Normalize (reorder) a CSV table
:tblsize: Get table width and height of a CSV file.
:tbltransp/tbltransp2: Transpose a CSV table
:tblunlabelize: Remove labels from a CSV file


Input format convention
-----------------------

All the table tools were developed with the same input format conventions with
the aim to reduce human errors.

CSV files are TAB separated by default, despite the name. This is usually the
default tabular format for UNIX.

If you use Excel to export your data, you should use the save as type *"Text
(Tab delimited) (\*.txt)"* when doing *"Save as ..."* to create a file
compatible with the table utilities.

You can change the column separator by setting the *TBLSEP* environment
variable as follows::

  export TBLSEP=';'

to work with the default excel format for all the subsequent invocations. You
can make that change permanent by appending this line to your
``/home/username/.bash_profile``. Files are also written with the same
separator.

The first line of any table **must** contain column labels. You always refer to
columns with columns labels, not numbers. All the files are written back with
column labels. This is intentional to avoid errors.

You can open the standard input by using '-' instead of a named file.  This is
not supported for some tools.
