/*
 * tbltransp2: fast table transposition - implementation
 * Copyright(c) 2008 EURAC, Institute of Genetic Medicine
 */

/*
 * Headers
 */

// system headers
#include <iostream>
using std::cerr;
using std::cout;

#include <vector>
using std::vector;


// c headers
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>



/*
 * Implementation
 */

void
help(char* argv[])
{
    cerr << argv[0] << ": bad parameters:\n"
	 << "Usage: " << argv[0] << " [-h] file\n"
	 << "Transpose the contents of a CSV file. CSV files are TAB separated by default.\n"
	 << "You can change the column separator by setting the TBLSEP environment variable.\n"
	 << "\n"
	 << "  -h:	help summary\n";
}


int
main(int argc, char* argv[])
{
  int arg;
  while((arg = getopt(argc, argv, "h")) != -1)
    switch(arg)
    {
    case 'h':
      help(argv);
      return EXIT_SUCCESS;

    default:
      return EXIT_FAILURE;
    }

  // check args
  argc -= optind;
  const char* file(argv[optind++]);
  if(argc != 1)
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
  int fd = open(file, O_RDONLY);
  if(fd < 0)
  {
    cerr << argv[0] << ": cannot open " << file << "\n";
    return EXIT_FAILURE;
  }

  // map the file
  struct stat stBuf;
  fstat(fd, &stBuf);
  size_t addrLen = stBuf.st_size;
  char* addr = (char*)mmap(NULL, addrLen, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(!addr)
  {
    cerr << argv[0] << ": cannot map " << file << "\n";
    return EXIT_FAILURE;
  }

  // start reading
  vector<vector<const char*> > m;
  vector<const char*> row;
  char* s = addr;
  for(char* p = s; p != addr + addrLen; ++p)
  {
    if(*p == '\n')
    {
      *p = 0;
      row.push_back(s);
      s = p + 1;
      m.push_back(row);
      row.clear();
    }
    else if(*p == '\r')
    {
      *p = 0;
    }
    else if(*p == sep)
    {
      *p = 0;
      row.push_back(s);
      s = p + 1;
    }
  }
  if(row.size())
    cerr << argv[0] << ": missing final newline, discarding last row!\n";

  // start writing back
  for(size_t x = 0; x != m.front().size(); ++x)
  {
    cout << m[0][x];
    for(size_t y = 1; y != m.size(); ++y)
      cout << sep << m[y][x];
    cout << '\n';
  }
}
