/*
 * tblmerge2: fast table merge - implementation
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
#include <unistd.h>


/*
 * Types and constants
 */

typedef map<string, size_t> col_map;
typedef map<string, size_t> key_map;
typedef vector<size_t> key_col;


/*
 * Implementation
 */

void
help(char* argv[])
{
  cerr << argv[0] << ": bad parameters:\n"
       << "Usage: " << argv[0] << " [-h] key file1 file2 [file3 ...]\n"
       << "Perform a full join on 'key' of two or more CSV files, performing a\n"
       << "comparison of common columns. 'key' can be a comma-separated list of\n"
       << "column names to form unique indexes. CSV files are TAB separated,\n"
       << "containing column labels on the first row. You can change the column\n"
       << "separator by setting the TBLSEP environment variable.\n"
       << "\n"
       << "  -v:	increase verbosity\n"
       << "  -k:	keep going on duplicate rows\n"
       << "  -h:	help summary\n";
}


string
buildKey(const key_col& kc, const vector<fix_string>& row)
{
  key_col::const_iterator it = kc.begin();
  string buf(row[*it]);
  for(++it; it != kc.end(); ++it)
  {
    buf += '\0';
    buf += row[*it];
  }
  return buf;
}


string
unescape(const string& str, const char c = ',')
{
  string buf;
  bool esc = false;
  foreach_ro(string, p, str)
  {
    if(*p == '\\' && !esc)
    {
      esc = true;
      continue;
    }

    if(*p == c && !esc)
      buf += '\0';
    else
      buf += *p;

    esc = false;
  }
  return buf;
}


string
escape(const string& str, const char c = ',')
{
  string buf;
  foreach_ro(string, it, str)
  {
    if(*it == '\0')
      buf += c;
    else if(*it != c)
      buf += *it;
    else
    {
      buf += '\\';
      buf += c;
    }
  }
  return buf;
}


void
mergeCharMatrix(fix_string_matrix& dst, col_map& dstCm, key_map& dstKm, const key_col& dstKc,
		const fix_string_matrix& add, const col_map& addCm, key_map& addKm, const key_col& addKc,
		bool keep_going=false)
{
  // preallocate all columns on dst
  vector<size_t> addDstCm;

  foreach_ro(vector<fix_string>, it, add.front())
  {
    string cname = *it;
    col_map::iterator dIt = dstCm.find(cname);
    if(dIt != dstCm.end())
      addDstCm.push_back(dIt->second);
    else
    {
      // allocate a new (empty) column
      size_t ki = dst.front().size();
      addDstCm.push_back(ki);
      dstCm.insert(make_pair(cname, ki));
      dst.front().push_back(*it);
      for(fix_string_matrix::iterator dIt = dst.begin() + 1; dIt != dst.end(); ++dIt)
	dIt->push_back(fix_string(NULL, 0));
    }
  }

  // iterate on add rows
  dst.reserve(dst.size() + add.size());

  for(fix_string_matrix::const_iterator it = add.begin() + 1; it != add.end(); ++it)
  {
    // key lookup
    string key = buildKey(addKc, *it);
    key_map::iterator dstKIt = dstKm.find(key);
    if(dstKIt == dstKm.end())
    {
      // new row
      dstKIt = dstKm.insert(make_pair(key, dst.size())).first;
      dst.push_back(vector<fix_string>(dst.front().size(), fix_string(NULL, 0)));
    }

    // merge row
    vector<fix_string>& dstRow = dst[dstKIt->second];
    for(size_t addCol = 0; addCol != it->size(); ++addCol)
    {
      if(!(*it)[addCol].size()) continue;

      size_t dstCol = addDstCm[addCol];
      if(!dstRow[dstCol].size())
	dstRow[dstCol] = (*it)[addCol];
      else
      {
	if((*it)[addCol] != dstRow[dstCol])
	{
	  string cname = add.front()[addCol];
	  string error = sprintf2("conflicting contents for column \"%s\", key \"%s\"",
				  cname.c_str(), escape(key).c_str());
	  if(keep_going)
	    cerr << error << std::endl;
	  else
	    throw runtime_error(error.c_str());
	}
      }
    }
  }
}


int
main(int argc, char* argv[]) try
{
  int arg;
  int verb = 0;
  bool keep_going = false;
  while((arg = getopt(argc, argv, "vhk")) != -1)
    switch(arg)
    {
    case 'v':
      ++verb;
      break;

    case 'k':
      keep_going = true;
      break;

    case 'h':
      help(argv);
      return EXIT_SUCCESS;

    default:
      return EXIT_FAILURE;
    }

  // check args
  argc -= optind;
  if(argc < 3)
  {
    help(argv);
    return EXIT_FAILURE;
  }

  // get default separator
  char sep = '\t';
  const char *envSep = getenv("TBLSEP");
  if(envSep && *envSep)
    sep = *envSep;

  // key
  const char* keyArg(argv[optind++]);
  vector<string> keys;
  tokenize(keys, unescape(keyArg), string("\0", 1), true);
  if(!keys.size())
  {
    cerr << argv[0] << ": no key specified!\n";
    return EXIT_FAILURE;
  }

  // loading stage
  // NOTE: the (mapped) memory is never freed after the merge, due to pointers
  //       to the mmap-ed region being used for the actual storage. This
  //       results in a very compact memory layout.
  fix_string_matrix* m = NULL;
  col_map cm;
  key_map km;
  key_col kc;
  do
  {
    const char* addr;
    const char* file(argv[optind++]);

    if(verb > 0) cerr << "loading " << file << "...\n";
    fix_string_matrix* tmp(mapFixStringMatrix(&addr, file, sep));
    if(!tmp->size() || !tmp->front().size())
    {
      cerr << file << ": file is empty\n";
      return EXIT_FAILURE;
    }

    // build the column map
    col_map ctmp;
    for(size_t i = 0; i != tmp->front().size(); ++i)
    {
      const string cname = tmp->front()[i];
      if(!ctmp.insert(make_pair(cname, i)).second)
      {
	cerr << file << ": duplicated column \"" << cname << "\"\n";
	return EXIT_FAILURE;
      }
    }

    key_col kcTmp;
    foreach_ro(vector<string>, keyIt, keys)
    {
      col_map::const_iterator ci = ctmp.find(*keyIt);
      if(ci == ctmp.end())
      {
	cerr << file << ": cannot find key column \"" << *keyIt << "\"\n";
	if(!keep_going) return EXIT_FAILURE;
      }
      kcTmp.push_back(ci->second);
    }

    // build the key map
    key_map ktmp;
    for(size_t i = 1; i != tmp->size(); ++i)
    {
      const string key = buildKey(kcTmp, (*tmp)[i]);
      if(!ktmp.insert(make_pair(key, i)).second)
      {
	cerr << file << ": duplicated key \"" << escape(key) << "\"\n";
	if(!keep_going) return EXIT_FAILURE;
      }
    }

    if(m)
    {
      // table merge on the working copy
      if(verb > 0) cerr << "merging " << file << "...\n";
      try { mergeCharMatrix(*m, cm, km, kc, *tmp, ctmp, ktmp, kcTmp, keep_going); }
      catch(const runtime_error& e)
      {	throw runtime_error(sprintf2("%s: %s", file, e.what())); }
    }
    else
    {
      // setup the destination table
      m = tmp;
      cm.swap(ctmp);
      km.swap(ktmp);
      kc.swap(kcTmp);
    }
  }
  while(argv[optind]);
  if(!m)
  {
    cerr << argv[0] << ": not enough files loaded\n";
    return EXIT_FAILURE;
  }

  // output
  foreach_ro(fix_string_matrix, it, *m)
  {
    vector<fix_string>::const_iterator it2 = it->begin();
    if(it2->size()) cout << *it2;
    for(++it2; it2 != it->end(); ++it2)
    {
      cout << sep;
      if(it2->size()) cout << *it2;
    }
    cout << '\n';
  }
}
catch(runtime_error& e)
{
  cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
