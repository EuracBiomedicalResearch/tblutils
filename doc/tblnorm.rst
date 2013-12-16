tblnorm normalizes the contents of a table by reordering both columns and rows.
This is mostly useful for comparing tables with a text editor.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__.

Command line flags
------------------

tblnorm can be launched on the calculation servers as follows:

`` $ tblnorm key file > output``

*key* is the key column that will dictate the row ordering. The *key* column is
also placed first in the resulting file.
