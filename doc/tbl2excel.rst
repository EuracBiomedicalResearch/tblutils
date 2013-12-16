tbl2excel converts CSV/text files to Excel sheets automatically very quickly
and without producing conversion errors.

Input format convention
-----------------------

See `Table/CSV utilities <Table/CSV utilities>`__. tbl2excel does not support
opening the standard input.

tbl2excel, as an exception, tries to autodetect column separators (including
repeated separators counting as one) and supports files with and without column
labels. If the autodetection process fails, ``TBLSEP`` is used as a default
column separator.

Command line flags
------------------

tbl2excel can be launched on the calculation servers as follows:

`` $ tbl2excel [options] file [file ...] > output-file.xls``

*[options]* can contain any of the following command line switches:

-  *-t type*: Use *type* for all columns, do not autodetect. Type can be
   *integer*, *double* or *string*.

-  *-T col:type*: Use *type* for the specified column. *col* is the column
   label, or the column number for files without column labels (numbers start
   at 1).

-  *-e*: Ensure exactness of all types/floating point conversions: write
   numbers as strings if they are not exactly convertible to integers or floats
   or when mixed contents for a column are detected.

-  *-l*: Do not read column labels from the first row

-  *-c*: Coalesce separators (consecutive separators count as one)

-  *-d sep*: Use *sep* as the column separator, do not autodetect.

-  *-u str*: Specify a custom comma-separated list of undefined values. Those
   values are converted to NA() in the Excel file.

-  *-m thr*: Column type autodetection minimum threshold (default: 99)

-  *-n str*: Assign sheet names for each input file (separate each sheet name
   with a comma).

-  *-r*: relax reader (continue reading on formatting errors)

-  *-x*: write XLSX (Excel 2012+) files instead of XLS (Excel 97-2003)

-  *-h*: Show an help summary.

Each file is put in a separate sheet inside the file. The sheet name, unless
overridden, is the name of the original text file.

You can repeat the '-T' flag more than once to specify types for different
columns. You can also combine '-t' and '-T' flags to skip the autodetection
mechanism but still override types for specific columns.

tbl2excel-helper
----------------

tbl2excel-helper is a small Perl script which is used internally by tbl2excel
to perform the actual format conversion. You can call tbl2excel-helper directly
to write Excel files programmatically from scripts or other utilities without
having to use custom libraries.

tbl2excel-helper reads list of simple commands from the standard input and
writes the results in Excel format.

tbl2excel-helper can be launched on the calculation servers as follows:

`` $ tbl2excel-helper [-x] [-h] [output file]``

The list of all available commands can be seen by using the *-h* flag. If no
output file is specified, the resulting binary is written on the standard
output.

By default, tbl2excel-helper writes files in Excel XLS format (97-2003) which
has a limit of 65535 rows and 255 columns. If *-x* is specified, the output is
written in XLSX format instead (Excel 2012 and onwards), which does not have
this limit.

Notes
-----

tbl2excel does not currently support dates or booleans as a column format.

The Perl ``Spreadsheet::WriteExcel`` module is required for tbl2excel-helper.
``Excel::Writer::XLSX`` is also required for XLSX writing.

Both modules are loaded dynamically depending on the output format, so only the
actually required modules need to be installed.
