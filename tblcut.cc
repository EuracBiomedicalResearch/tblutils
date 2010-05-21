/*
 * tblcut: fast 'cut' alternative - implementation
 * Copyright(c) 2010 EURAC, Institute of Genetic Medicine
 */

/*
 * Headers
 */

// local headers
#include "shared.hh"

// system headers
#include <map>
using std::map;
using std::make_pair;

// c headers
#include <stdlib.h>


/*
 * Types and constants
 */

typedef map<string, size_t> col_map;


/*
 * Implementation
 */

void
help(char* argv[])
{
  cerr << argv[0] << ": bad parameters:\n"
       << "Usage: " << argv[0] << " [-h] -f col,col,... file\n"
       << "tblcut allows to extract single columns by name from the selected CSV file.\n"
       << "CSV files are TAB separated, containing column labels on the first row. You\n"
       << "can change the column separator by setting the TBLSEP environment variable.\n"
       << "\n"
       << "  -f col,col,...:	extract the selected columns (mandatory)\n"
       << "  -h:			help summary\n";
}


int
main(int argc, char* argv[]) try
{
  vector<string> fields;

  int arg;
  while((arg = getopt(argc, argv, "hf:")) != -1)
    switch(arg)
    {
    case 'h':
      help(argv);
      return EXIT_SUCCESS;

    case 'f':
      tokenize(fields, optarg, ",", true);
      break;

    default:
      return EXIT_FAILURE;
    }

  // check args
  argc -= optind;
  const char* file(argv[optind++]);
  if(argc != 1 || !fields.size())
  {
    help(argv);
    return EXIT_FAILURE;
  }

  // get default separator
  char sep = '\t';
  const char *envSep = getenv("TBLSEP");
  if(envSep && *envSep)
    sep = *envSep;

  // open the file
  int fd;
  char* addr;
  char_matrix& m = *mapCharMatrix(fd, &addr, file, sep);

  // build the column map
  col_map cmap;
  for(size_t i = 0; i != m.front().size(); ++i)
  {
    const char* cname = m.front()[i];
    if(!cmap.insert(make_pair(string(cname), i)).second)
    {
      cerr << file << ": duplicated column \"" << cname << "\"\n";
      return EXIT_FAILURE;
    }
  }

  // build the resulting column list
  vector<size_t> cols;
  for(vector<string>::const_iterator it = fields.begin();
      it != fields.end(); ++it)
  {
    col_map::const_iterator cIt = cmap.find(*it);
    if(cIt == cmap.end())
    {
      cerr << file << ": unknown column \"" << *it << "\"\n";
      return EXIT_FAILURE;
    }
    cols.push_back(cIt->second);
  }

  // output
  for(char_matrix::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    vector<size_t>::const_iterator it2 = cols.begin();
    cout << (*it)[*it2];
    for(++it2; it2 != cols.end(); ++it2)
      cout << sep << (*it)[*it2];
    cout << '\n';
  }
}
catch(runtime_error& e)
{
  cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
