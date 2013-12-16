tblcsort allow to sort and/or reorder the *columns* of a CSV file.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__.

Command line flags
------------------

tblcsort can be launched on the calculation servers as follows:

`` $ tblcsort [options] file [column names]``

*[options]* can contain any of the following command line switches:

-  *-n*: Sort the columns numerically instead of alphabetically.
-  *-h*: Show the help summary.

By default tblcsort reorders the columns in alphabetical order. You can supply
any custom order by writing the labels after the file name.

If you specify a custom order, but omit some columns, the omitted columns
appear in the same order as the source file, but after the ones you manually
entered.
