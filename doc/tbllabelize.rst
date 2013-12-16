tblabelize assigns numerical or custom labels to an unlabeled CSV file
automatically without manually editing the file. tblabelize is especially
useful when using other table utilities that require column labels. Also see
tblunlabelize.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__.

Command line flags
------------------

tblabelize can be launched on the calculation servers as follows:

`` $ tblabelize [options] file [column names] > output``

If all (or some) column names are specified, the names are used for the
leftmost columns of the table. Other columns are assigned numerical labels
incrementally.

*[options]* can contain any of the following command line switches:

-  *-z*: Column numbers start from 0.
-  *-n N*: Column numbers start from the specified number *N*.

