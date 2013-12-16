tblfilter allows to filter (remove) the rows of the specified CSV file through
an arbitrary Perl expression.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__.

Command line flags
------------------

tblfilter can be launched on the calculation servers as follows:

`` $ tblfilter [options] -e expression file``

*[options]* can contain any of the following command line switches:

-  *-e*: Perl expression (mandatory)

Just the lines for which the expression returns non-zero are returned. Any Perl
expression or statement can be used, and will be executed once for each row.
For instance '1' will return all rows and '0' will return nothing.

You can refer to $N[x] for column positions or $L{name} for column labels
relative to the current row. For example:

`` tblfilter -e '$N[1] > 1'``

returns all the lines for which the first column is > 1, and:

`` tblfilter -e '$L{test} > 1'``

returns all the lines for which the column 'test' is > 1
