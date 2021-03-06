/*
 * shared: basic shared utilities and structures for tblutils
 *
 * Copyright(c) 2008-2010 Yuri D'Elia <yuri.delia@eurac.edu>
 * Copyright(c) 2008-2010 EURAC, Institute of Genetic Medicine
 */

#pragma once

/*
 * Headers
 */

// base headers
#include <iostream>
using std::cerr;
using std::cout;

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <stdexcept>
using std::runtime_error;

// c system headers
#include <string.h>
using std::string;


/*
 * Generics
 */

#define ARRAY_LENGTH(X) (sizeof(X) / sizeof(*X))
#define foreach(T, I, C) for(T::iterator (I) = (C).begin(); I != (C).end(); ++(I))
#define foreach_ro(T, I, C) for(T::const_iterator (I) = (C).begin(); I != (C).end(); ++(I))

string
sprintf2(const char* fmt, ...);

vector<string>&
tokenize(vector<string>& dst, const string& buf,
    const string& sep = "\t", bool coalesce = false);

class Progress
{
  long max;
  const char* type;
  bool clean;
  bool tty;

public:
  Progress(long max, const char* type);


  ~Progress() throw()
  {
    cleanup();
  }


  void
  remax(long newMax)
  {
    max = newMax;
  }


  void
  operator()(long v)
  {
    if(!tty) return;

    cerr << v << ' ';
    if(type) cerr << type;
    cerr << ' ' << (max? (v * 100 / max): 100) << "%\r";
  }


  void
  cleanup()
  {
    if(clean || !tty) return;

    clean = true;
    operator()(max);
    cerr << '\n';
  }
};



/*
 * I/O helpers
 */

template<class T>
class named_stream: public T
{
  const char* const file_;

public:
  named_stream(const char* file)
  : T(file), file_(file)
  {}

  const char*
  file() const
  { return file_; }
};


typedef named_stream<istream> named_istream;
typedef named_stream<ostream> named_ostream;
typedef named_stream<ifstream> named_ifstream;
typedef named_stream<ofstream> named_ofstream;


class namedio_error: public runtime_error
{
public:
  namedio_error(const char* file, const char* what)
  : runtime_error(sprintf2("%s:%lu: %s", file, what))
  {}

  namedio_error(const char* file, int line, const char* what)
  : runtime_error(sprintf2("%s:%lu: %s", file, line, what))
  {}

  template<class T>
  namedio_error(const named_stream<T>& fd, const char* what)
  : runtime_error(sprintf2("%s: %s", fd.file(), what))
  {}

  template<class T>
  namedio_error(const named_stream<T>& fd, int line, const char* what)
  : runtime_error(sprintf2("%s:%lu: %s", fd.file(), line, what))
  {}
};


class fix_string
{
  const char* addr;
  size_t len;

public:
  explicit
  fix_string(const char* addr, const size_t len)
  : addr(addr), len(len)
  {}

  operator string() const
  { return string(addr, len); }

  const char*
  data() const
  { return addr; }

  size_t
  size() const
  { return len; }

  bool
  operator !=(const fix_string& r) const
  {
    return ((len != r.len) || memcmp(addr, r.addr, len));
  }
};


inline ostream&
operator <<(ostream& buf, const fix_string& r)
{
  buf.write(r.data(), r.size());
  return buf;
}


typedef vector<vector<fix_string> > fix_string_matrix;

fix_string_matrix*
mapFixStringMatrix(const char** addr, const char* file, const char sep, int* fd = NULL);
