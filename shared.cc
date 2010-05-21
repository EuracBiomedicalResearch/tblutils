/*
 * shared: basic shared utilities and structures for tblutils - implementation
 *
 * Copyright(c) 2008-2010 Yuri D'Elia <yuri.delia@eurac.edu>
 * Copyright(c) 2008-2010 EURAC, Institute of Genetic Medicine
 */

/*
 * Headers
 */

// interface
#include "shared.hh"

// base headers
#include <memory>
using std::auto_ptr;

// c headers
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


/*
 * Generics
 */

Progress::Progress(long max, const char* type)
: max(max), type(type), clean(false)
{
  tty = isatty(STDERR_FILENO);
}


string
sprintf2(const char* fmt, ...)
{
  int n;
  vector<char> buf(64);

  va_list vl;
  for(;;)
  {
    va_start(vl, fmt);
    n = vsnprintf(buf.data(), buf.size(), fmt, vl);
    va_end(vl);

    if(n > -1 && static_cast<size_t>(n) < buf.size()) break;
    buf.resize((n > -1? n + 1: buf.size() * 2));
  }

  return string(buf.data(), n);
}


vector<string>&
tokenize(vector<string>& dst, const string& buf,
    const char* sep, bool coalesce)
{
  string::size_type s = 0;
  string::size_type e = buf.find_first_of(sep, s);

  while(s != string::npos || e != string::npos)
  {
    if(e != string::npos)
      dst.push_back(buf.substr(s, e - s));
    else
    {
      dst.push_back(buf.substr(s));
      break;
    }

    if(coalesce)
      s = buf.find_first_not_of(sep, e);
    else
      s = e + 1;

    e = buf.find_first_of(sep, s);
  }

  return dst;
}



/*
 * I/O
 */

char_matrix*
mapCharMatrix(int& fd, char** addr, const char* file, const char sep)
{
  // open the file
  *addr = NULL;
  fd = open(file, O_RDONLY);
  if(fd < 0)
    throw runtime_error(sprintf2("%s: error: cannot open file!", file));

  // map the file
  struct stat stBuf;
  fstat(fd, &stBuf);
  size_t addrLen = stBuf.st_size;
  *addr = (char*)mmap(NULL, addrLen + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(!*addr)
    throw runtime_error(sprintf2("%s: error: cannot map file!", file));

  // start reading
  auto_ptr<char_matrix> m(new char_matrix);
  vector<const char*> row;
  char* s = *addr;
  for(char* p = s; p != (*addr + addrLen); ++p)
  {
    if(*p == '\n')
    {
      *p = 0;
      row.push_back(s);
      s = p + 1;
      m->push_back(row);
      row.clear();

      if(m->front().size() != m->back().size())
	throw runtime_error(sprintf2("%s: error: variable number of columns", file));
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
  {
    // missing final newline
    cerr << file << ": warning: missing final newline!\n";
    (*addr)[addrLen] = 0;
    row.push_back(s);
    m->push_back(row);
  }

  return m.release();
}
