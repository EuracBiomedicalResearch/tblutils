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
       << "Usage: " << argv[0] << " [-hd] < -f col,col,... | -n col,col,... > file\n"
       << "tblcut allows to extract single columns by name from the selected CSV file.\n"
       << "CSV files are TAB separated, containing column labels on the first row. You\n"
       << "can change the column separator by setting the TBLSEP environment variable.\n"
       << "\n"
       << "  -d sep:		set a different column separator\n"
       << "  -f col,col,...:	extract the selected column names\n"
       << "  -n col,col,...:	extract the selected column numbers\n"
       << "  -h:			help summary\n";
}


int
main(int argc, char* argv[]) try
{
  vector<string> fieldNames;
  vector<size_t> fieldNums;
  char sep = 0;

  int arg;
  while((arg = getopt(argc, argv, "hf:n:d:")) != -1)
    switch(arg)
    {
    case 'h':
      help(argv);
      return EXIT_SUCCESS;

    case 'd':
      sep = *optarg;
      break;

    case 'f':
      tokenize(fieldNames, optarg, ",", true);
      break;

    case 'n':
      {
	vector<string> tmp;
	tokenize(tmp, optarg, ",", true);
	for(vector<string>::iterator it = tmp.begin(); it != tmp.end(); ++it)
	  fieldNums.push_back(strtoul(it->c_str(), NULL, 0));
      }
      break;

    default:
      return EXIT_FAILURE;
    }

  // check args
  argc -= optind;
  const char* file(argv[optind++]);
  if(argc != 1
  || (fieldNames.size() && fieldNums.size())
  || (!fieldNames.size() && !fieldNums.size()))
  {
    help(argv);
    return EXIT_FAILURE;
  }

  // get default separator
  if(!sep)
  {
    const char *envSep = getenv("TBLSEP");
    if(envSep && *envSep) sep = *envSep;
    else sep = '\t';
  }

  // open the file
  int fd;
  char* addr;
  char_matrix& m = *mapCharMatrix(fd, &addr, file, sep);

  // build the resulting column list
  vector<size_t> cols;
  if(fieldNums.size())
  {
    // from column numbers
    for(vector<size_t>::const_iterator it = fieldNums.begin();
	it != fieldNums.end(); ++it)
    {
      if(!*it || *it > m.front().size())
      {
	cerr << file << ": invalid column number " << *it << "\n";
	return EXIT_FAILURE;
      }

      cols.push_back(*it - 1);
    }
  }
  else
  {
    // from column names
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

    for(vector<string>::const_iterator it = fieldNames.begin();
	it != fieldNames.end(); ++it)
    {
      col_map::const_iterator cIt = cmap.find(*it);
      if(cIt == cmap.end())
      {
	cerr << file << ": unknown column \"" << *it << "\"\n";
	return EXIT_FAILURE;
      }
      cols.push_back(cIt->second);
    }
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
