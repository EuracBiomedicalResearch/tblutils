/*
 * tbltransp2: fast table transposition - implementation
 * Copyright(c) 2008 EURAC, Institute of Genetic Medicine
 */

/*
 * Headers
 */

// local headers
#include "shared.hh"

// c headers
#include <stdlib.h>
#include <unistd.h>


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
main(int argc, char* argv[]) try
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
  int fd;
  const char* addr;
  fix_string_matrix& m = *mapFixStringMatrix(fd, &addr, file, sep);

  // start writing back
  for(size_t x = 0; x != m.front().size(); ++x)
  {
    cout << m[0][x];
    for(size_t y = 1; y != m.size(); ++y)
      cout << sep << m[y][x];
    cout << '\n';
  }
}
catch(runtime_error& e)
{
  cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
