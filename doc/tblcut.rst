tblcut allows to extract columns from a table by their name.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__.

Command line flags
------------------

tblcut can be launched on the calculation servers as follows:

`` $ tblcut [options] -f column file``

*[options]* can contain any of the following command line switches:

-  *-f col,col,...*: Mandatory: sets the column names (comma separated) to be
   extracted. The order and repetitions are preserved.
-  *-d sep*: Set a different column separator directly on the command line
   (default is tab).
-  *-c*: Complement the selected fields.
-  *-h*: Show the help summary.

