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
#include <string.h>


/*
 * Types and constants
 */

typedef map<string, size_t> col_map;
typedef map<string, size_t> key_map;


/*
 * Implementation
 */

void
help(char* argv[])
{
  cerr << argv[0] << ": bad parameters:\n"
       << "Usage: " << argv[0] << " [-h] key file1 file2 [file3 ...]\n"
       << "Perform a full join on 'key' of two or more CSV files, performing a\n"
       << "comparison of common columns. CSV files are TAB separated, containing\n"
       << "column labels on the first row. You can change the column separator\n"
       << "by setting the TBLSEP environment variable.\n"
       << "\n"
       << "  -v:	increase verbosity\n"
       << "  -h:	help summary\n";
}


void
mergeCharMatrix(fix_string_matrix& dst, col_map& dstCm, key_map& dstKm, size_t dstKi,
    const fix_string_matrix& add, const col_map& addCm, key_map& addKm, size_t addKi)
{
  // preallocate all columns on dst
  vector<size_t> addDstCm;

  for(vector<fix_string>::const_iterator it = add.front().begin();
      it != add.front().end(); ++it)
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
    string kname = (*it)[addKi];
    key_map::iterator dstKIt = dstKm.find(kname);
    if(dstKIt == dstKm.end())
    {
      // new row
      dstKIt = dstKm.insert(make_pair(kname, dst.size())).first;
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
	  throw runtime_error(
	      sprintf2("conflicting contents for column \"%s\", key \"%s\"",
		  cname.c_str(), kname.c_str()));
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
  while((arg = getopt(argc, argv, "vh")) != -1)
    switch(arg)
    {
    case 'v':
      ++verb;
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

  // loading stage
  // NOTE: the (mapped) memory is never freed after the merge, due to pointers
  //       to the mmap-ed region being used for the actual storage. This
  //       results in a very compact memory layout, except when cross-merging
  //       identical tables (as for comparisons).
  const char* key(argv[optind++]);
  fix_string_matrix* m = NULL;
  col_map cm;
  key_map km;
  size_t ki;
  do
  {
    int fd;
    const char* addr;
    const char* file(argv[optind++]);

    if(verb > 0) cerr << "loading " << file << "...\n";
    fix_string_matrix* tmp(mapFixStringMatrix(fd, &addr, file, sep));
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

    col_map::const_iterator keyIt = ctmp.find(key);
    if(keyIt == ctmp.end())
    {
      cerr << file << ": cannot find key column \"" << key << "\"\n";
      return EXIT_FAILURE;
    }

    // build the key map
    key_map ktmp;
    size_t itmp = keyIt->second;
    for(size_t i = 1; i != tmp->size(); ++i)
    {
      const string kname = (*tmp)[i][itmp];
      if(!ktmp.insert(make_pair(kname, i)).second)
      {
	cerr << file << ": duplicated key \"" << kname << "\"\n";
	return EXIT_FAILURE;
      }
    }

    if(m)
    {
      // table merge on the working copy
      if(verb > 0) cerr << "merging " << file << "...\n";
      try { mergeCharMatrix(*m, cm, km, ki, *tmp, ctmp, ktmp, itmp); }
      catch(const runtime_error& e)
      {	throw runtime_error(sprintf2("%s: %s", file, e.what())); }
    }
    else
    {
      // setup the destination table
      m = tmp;
      cm.swap(ctmp);
      km.swap(ktmp);
      ki = itmp;
    }
  }
  while(argv[optind]);
  if(!m)
  {
    cerr << argv[0] << ": not enough files loaded\n";
    return EXIT_FAILURE;
  }

  // output
  for(fix_string_matrix::const_iterator it = m->begin(); it != m->end(); ++it)
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
