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


/*
 * Generics
 */

#define ARRAY_LENGTH(X) (sizeof(X) / sizeof(*X))

string
sprintf2(const char* fmt, ...);

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


typedef vector<vector<const char*> > char_matrix;

char_matrix*
mapCharMatrix(int& fd, char** addr, const char* file, const char sep);
