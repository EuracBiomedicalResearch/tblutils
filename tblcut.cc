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
using std::multimap;
using std::make_pair;

#include <algorithm>
using std::find;

// c headers
#include <stdlib.h>
#include <unistd.h>


/*
 * Types and constants
 */

typedef multimap<string, size_t> col_map;


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
       << "  -c:			complement the selected fields\n"
       << "  -h:			help summary\n";
}


int
main(int argc, char* argv[]) try
{
  vector<string> fieldNames;
  vector<size_t> fieldNums;
  bool complement = false;
  char sep = 0;

  int arg;
  while((arg = getopt(argc, argv, "hf:n:d:c")) != -1)
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

    case 'c':
      complement = !complement;
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
  const char* addr;
  fix_string_matrix& m = *mapFixStringMatrix(fd, &addr, file, sep);

  // build the resulting column list
  vector<size_t> cols;
  if(fieldNums.size())
  {
    // check arguments
    for(vector<size_t>::const_iterator it = fieldNums.begin();
	it != fieldNums.end(); ++it)
    {
      if(!*it || *it > m.front().size())
      {
	cerr << file << ": invalid column number " << *it << "\n";
	return EXIT_FAILURE;
      }
    }

    // from column numbers
    if(!complement)
    {
      for(vector<size_t>::const_iterator it = fieldNums.begin();
	  it != fieldNums.end(); ++it)
	cols.push_back(*it - 1);
    }
    else
    {
      for(size_t i = 0; i != m.front().size(); ++i)
      {
	if(find(fieldNums.begin(), fieldNums.end(), i + 1) == fieldNums.end())
	  cols.push_back(i);
      }
    }
  }
  else
  {
    // from column names
    if(!complement)
    {
      col_map cmap;
      for(size_t i = 0; i != m.front().size(); ++i)
      {
	const string cname = m.front()[i];
	cmap.insert(make_pair(string(m.front()[i]), i));
      }

      for(vector<string>::const_iterator it = fieldNames.begin();
	  it != fieldNames.end(); ++it)
      {
	col_map::const_iterator cIt = cmap.lower_bound(*it);
	if(cIt == cmap.end())
	{
	  cerr << file << ": unknown column \"" << *it << "\"\n";
	  return EXIT_FAILURE;
	}

	for(col_map::const_iterator cEnd = cmap.upper_bound(*it);
	    cIt != cEnd; ++cIt)
	  cols.push_back(cIt->second);
      }
    }
    else
    {
      for(vector<fix_string>::const_iterator it = m.front().begin();
	  it != m.front().end(); ++it)
      {
	if(find(fieldNames.begin(), fieldNames.end(), string(*it)) == fieldNames.end())
	  cols.push_back(it - m.front().begin());
      }
    }
  }

  // output
  for(fix_string_matrix::const_iterator it = m.begin(); it != m.end(); ++it)
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
