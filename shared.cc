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
    const string& sep, bool coalesce)
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

fix_string_matrix*
mapFixStringMatrix(int& fd, const char** addr, const char* file, const char sep)
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
  *addr = (const char*)mmap(NULL, addrLen, PROT_READ, MAP_SHARED, fd, 0);
  if(!*addr)
    throw runtime_error(sprintf2("%s: error: cannot map file!", file));

  // start reading
  auto_ptr<fix_string_matrix> m(new fix_string_matrix);
  vector<fix_string> row;
  const char* s = *addr;
  const char* p;
  for(p = s; p != (*addr + addrLen); ++p)
  {
    if(*p == '\n')
    {
      size_t len = p - s;
      if(*(p - 1) == '\r') --len;
      row.push_back(fix_string(s, len));
      s = p + 1;
      m->push_back(row);
      row.clear();

      if(m->front().size() != m->back().size())
	throw runtime_error(sprintf2("%s: error: variable number of columns", file));
    }
    else if(*p == sep)
    {
      row.push_back(fix_string(s, p - s));
      s = p + 1;
    }
  }
  if(row.size())
  {
    // missing final newline
    cerr << file << ": warning: missing final newline!\n";
    row.push_back(fix_string(s, p - s));
    m->push_back(row);
  }

  return m.release();
}
