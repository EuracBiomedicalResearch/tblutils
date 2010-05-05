/*
 * tbl2excel: convert a plain-text tabular file to Excel
 *
 * Copyright(c) 2008-2010 Yuri D'Elia <yuri.delia@eurac.edu>
 * Copyright(c) 2008-2010 EURAC, Institute of Genetic Medicine
 */

/*
 * Headers
 */

// base headers
#include <stdexcept>
using std::runtime_error;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::map;

#include <set>
using std::set;

#include <utility>
using std::min;
using std::max;

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;

#include <iostream>
using std::cerr;

#include <memory>
using std::auto_ptr;

// c headers
#include <locale.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


/*
 * Internal constants and types
 */

// detection constants
const char helperCmd[] = "tbl2excel-helper";
const size_t rowLimit = 65535;
const size_t colLimit = 255;
const int defaultDetectThr = 99;
const int detectLines = 3;
const char fallbackEnv[] = "TBLSEP";
const char fallbackSep = '\t';
const char detectSep[] = ",;: \t|";
const char defaultUndefStr[] = "NA,-, ,.,";

// types
enum datatype_t
{
  int_type,
  double_type,
  string_type,
  unknown_type
};

typedef vector<string> string_row;
typedef vector<string_row> string_matrix;
typedef map<string, datatype_t> type_map;

struct matrix_data
{
  bool labels;
  vector<datatype_t> colTypes;
  string_matrix* m;
};

struct detect_params
{
  const char* sep;
  type_map colTypes;
  datatype_t defType;
  unsigned detectThr;
  set<string> undefStr;
  bool exact;
  bool labels;
  bool coalesce;
};


/*
 * Generics
 */

#define ARRAY_LENGTH(X) (sizeof(X) / sizeof(*X))

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


class Progress
{
  long max;
  const char* type;
  bool clean;
  bool tty;

public:
  Progress(long max, const char* type)
  : max(max), type(type), clean(false)
  {
    tty = isatty(STDERR_FILENO);
  }


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



/*
 * Implementation
 */

size_t
scale100(size_t v, size_t m)
{
  return (!v? 0
      : v == m? 100
      : max(1UL, min(99UL, v * 100 / m)));
}


const char*
sepToString(char s)
{
  static char buf[4] = {'\'', 0, '\'', 0};

  switch(s)
  {
  case ' ': return "space";
  case '\t': return "tab";
  }

  buf[1] = s;
  return buf;
}


istream&
xgetline2(istream& fd, string& buf)
{
  istream& ret(getline(fd, buf));
  if(buf.size() && *(buf.end() - 1) == '\r')
    buf.resize(buf.size() - 1);
  return ret;
}


vector<string>&
tokenize(vector<string>& dst, const string& buf,
    const char* sep = "\t", bool coalesce = false)
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


void
detectTxt(named_ifstream& fd, detect_params& dp)
{
  // character fequencies
  const int seps = ARRAY_LENGTH(detectSep);
  int sepFreq[seps][detectLines] = {};
  int sepSFreq[seps][detectLines] = {};

  for(int i = 0; i != detectLines; ++i)
  {
    string line;
    if(!xgetline2(fd, line)) break;
    if(!line.size()) continue;

    // start counting
    char lst = 0;
    for(const char* p = line.c_str(); *p; ++p)
    {
      const char* s = strchr(detectSep, *p);
      if(!s)
      {
	lst = *p;
	continue;
      }

      int sn = (s - detectSep);
      ++sepFreq[sn][i];
      if(lst != *p)
      {
	lst = *p;
	++sepSFreq[sn][i];
      }
    }
  }

  // remove inconsistent separators
  set<int> okSeps;
  set<int> okSSeps;
  for(int i = 1; i < detectLines; ++i)
  {
    for(int j = 0; j != seps; ++j)
    {
      if(sepFreq[j][i]) okSeps.insert(j);
      if(sepSFreq[j][i]) okSSeps.insert(j);
    }
  }

  for(int i = 1; i < detectLines; ++i)
  {
    for(int j = 0; j != seps; ++j)
    {
      if(sepFreq[j][i] != sepFreq[j][i - 1]) okSeps.erase(j);
      if(sepSFreq[j][i] != sepSFreq[j][i - 1]) okSSeps.erase(j);
    }
  }

  // result of detection
  static char retBuf[2] = {};
  dp.sep = retBuf;

  if(okSeps.size() != 1 && okSSeps.size() != 1)
  {
    const char* fs = getenv(fallbackEnv);
    if(!fs) fs = &fallbackSep;

    cerr << fd.file() << ": warning: cannot detect column separator, using "
	 << sepToString(*fs) << "\n";
    retBuf[0] = *fs;
    return;
  }

  if(okSeps.size() == 1)
  {
    dp.coalesce = 0;
    retBuf[0] = detectSep[*okSeps.begin()];
    cerr << fd.file() << ": detected separator: "
	 << sepToString(retBuf[0]) << "\n";
  }
  else
  {
    dp.coalesce = 1;
    retBuf[0] = detectSep[*okSSeps.begin()];
    cerr << fd.file() << ": detected coalesced separator: "
	 << sepToString(retBuf[0]) << "\n";
  }
}


string_matrix*
loadTxt(named_ifstream& fd, const detect_params& dp)
{
  string line;
  string_row row;
  size_t cols = 0;
  auto_ptr<string_matrix> m(new string_matrix);

  // some stats
  fd.seekg(0, std::ios_base::end);
  ssize_t fdSize = fd.tellg();
  fd.seekg(0);

  // initial provisioning
  size_t fdLineSize;
  size_t fdAvg = 1;
  size_t fdSteps = 1;

  // start to read data
  for(long l = 1;; ++l)
  {
    if(!xgetline2(fd, line)) break;

    // check line
    if(!line.size())
    {
      cerr << fd.file() << ":" << l << ": warning: empty line\n";
      continue;
    }

    for(string::iterator it = line.begin();
	it != line.end(); ++it)
    {
      if(!strchr(dp.sep, *it) && !isprint(*it))
      {
	cerr << fd.file() << ":" << l << ": warning: odd characters\n";
	break;
      }
    }

    // incremental growing
    if(!(l % fdSteps))
    {
      fdLineSize = fd.tellg() / l;
      size_t nFdAvg = fdSize / fdLineSize;
      if(nFdAvg > fdAvg)
      {
	fdAvg = nFdAvg;
	fdSteps = max<size_t>(1, fdAvg / 100);
	m->reserve(fdAvg);
      }
    }

    // tokenize
    row.reserve(cols);
    tokenize(row, line, dp.sep, dp.coalesce);

    if(!cols) cols = row.size();
    else if(cols != row.size())
      throw namedio_error(fd, l, "error: variable number of columns");

    // save
    m->push_back(row);
    row.clear();
  }

  return m.release();
}


datatype_t
classifyColumn(const detect_params& dp, size_t c,
    string_matrix::const_iterator begin, string_matrix::const_iterator end)
{
  size_t asInteger = 0;
  size_t asDouble = 0;
  size_t asString = 0;
  size_t asTotal = (end - begin);
  const lconv* lc = localeconv();

  for(string_matrix::const_iterator it = begin; it != end; ++it)
  {
    string buf = (*it)[c];
    datatype_t t = unknown_type;

    if(buf.size())
    {
      // NaNs
      if(dp.undefStr.find(buf) != dp.undefStr.end())
	t = unknown_type;
      else
      {
	// run simple character classification
	t = int_type;
	for(size_t i = 0; i != buf.size(); ++i)
	{
	  if(!isdigit(buf[i]) && (buf[i] != *lc->thousands_sep))
	  {
	    if(buf[i] == *lc->decimal_point)
	      t = double_type;
	    else
	    {
	      t = string_type;
	      break;
	    }
	  }
	}
      }
    }

    // try complex double representations
    if(t == string_type)
    {
      char* tmp;

      // remove thousand separators
      if(*lc->thousands_sep)
      {
	string::iterator it = buf.begin();
	while(it != buf.end())
	{
	  if(*it == *lc->thousands_sep)
	    buf.erase(it);
	  else
	    ++it;
	}
      }

      strtod(buf.c_str(), &tmp);
      if(!*tmp) t = double_type;
    }

    switch(t)
    {
    case int_type: ++asInteger; break;
    case double_type: ++asDouble; break;
    case string_type: ++asString; break;
    case unknown_type: --asTotal; break;
    }
  }

  // scale to percentage with exact 0,100
  asDouble = scale100(asDouble, asTotal);
  asInteger = scale100(asInteger, asTotal);
  asString = scale100(asString, asTotal);

  size_t asMax = max(max(asInteger, asDouble), asString);
  bool exact = asMax < (dp.exact? 100: dp.detectThr);
  if(exact)
  {
    cerr << "warning: mixed contents in next column ("
	 << asInteger << "% integers, "
	 << asDouble << "% doubles, "
	 << asString << "% strings)\n";
  }

  datatype_t t;

  // widen
  if(exact? !asDouble && !asString && asTotal: asMax == asInteger)
    t = int_type;
  else if(exact? !asInteger && !asString && asTotal: asMax == asDouble)
    t = double_type;
  else
    t = string_type;

  return t;
}


datatype_t
parseType(const char* p)
{
  if(!strcasecmp(p, "str") || !strcasecmp(p, "string")
  || !strcasecmp(p, "char") || !strcasecmp(p, "character"))
    return string_type;
  else if(!strcasecmp(p, "int") || !strcasecmp(p, "integer"))
    return int_type;
  else if(!strcasecmp(p, "double") || !strcasecmp(p, "real"))
    return double_type;

  throw runtime_error(
      (string("unknown type specifier: ") + p).c_str());
}


void
parseColType(type_map& colTypes, const char* str)
{
  const char* p = strchr(str, ':');
  if(!p || p == str || !*(p + 1))
    throw runtime_error(
	(string("bad column type specifier: ") + str).c_str());

  string col(str, p - str);
  datatype_t type(parseType(++p));
  colTypes.insert(std::pair<string, datatype_t>(col, type));
}


string
columnLabel(const matrix_data& md, size_t c)
{
  return (md.labels?
      md.m->front()[c]:
      sprintf2("%d", c + 1));
}


void
classify(matrix_data& md, const detect_params& dp)
{
  cerr << "columns:\n";

  for(size_t c = 0; c != md.colTypes.size(); ++c)
  {
    string label(columnLabel(md, c));
    type_map::const_iterator type(dp.colTypes.find(label));

    // column type
    datatype_t t;
    if(type != dp.colTypes.end())
      t = type->second;
    else if(dp.defType == unknown_type)
      t = classifyColumn(dp, c, md.m->begin() + md.labels, md.m->end());
    else
      t = dp.defType;

    // save the result
    md.colTypes[c] = t;

    // show results
    cerr << "  " << label << ": type ";
    switch(t)
    {
    case int_type: cerr << "integer"; break;
    case double_type: cerr << "double"; break;
    case string_type: cerr << "string"; break;
    }
    cerr << std::endl;
  }
}


void
uniqueTokens(set<string>& dst, const char* str)
{
  vector<string> buf;
  tokenize(buf, str, ",");
  for(vector<string>::iterator it = buf.begin(); it != buf.end(); ++it)
    dst.insert(*it);
}


void
output(FILE* fd, const string& sheetName, const matrix_data& md, const detect_params& dp)
{
  // start a new sheet
  fprintf(fd, "ns %s\n", sheetName.c_str());

  // write a warning on the sheet if the conversion will overflow the Excel limits
  if(md.colTypes.size() > colLimit || md.m->size() > rowLimit)
  {
    fprintf(fd, "b a s WARNING: output truncated due to Excel row/column limits!\nnr\n");
    cerr << sheetName << ": warning: output truncated due to Excel row/column limits!\n";
  }

  // write the labels table, if any
  if(md.labels)
  {
    for(string_row::const_iterator it = md.m->front().begin();
	it != md.m->front().end(); ++it)
      fprintf(fd, "b a s %s\n", it->c_str());
    fprintf(fd, "nr\n");
  }

  // start writing the sheet
  size_t rows = min<size_t>(md.m->end() - md.m->begin() - md.labels, rowLimit);
  size_t cols = min<size_t>(md.colTypes.size(), colLimit);
  size_t steps = max<size_t>(1, rows / 100);
  Progress progress(rows, "rows");

  string_matrix::const_iterator it = md.m->begin() + md.labels;
  for(size_t x = 0; x != rows; ++x, ++it)
  {
    if(!(x % steps)) progress(x);

    for(size_t c = 0; c != cols; ++c)
    {
      datatype_t t = md.colTypes[c];
      const string& buf = (*it)[c];

      // NaNs
      if(dp.undefStr.find(buf) != dp.undefStr.end())
      {
	fprintf(fd, "a f NA()\n");
	continue;
      }

      // normal types
      switch(t)
      {
      case int_type:
	fprintf(fd, "a i %s\n", buf.c_str());
	break;

      case double_type:
	fprintf(fd, "a d %s\n", buf.c_str());
	break;

      case string_type:
	fprintf(fd, "a s %s\n", buf.c_str());
	break;
      }
    }

    fprintf(fd, "nr\n");
  }
}


// entry point
int
main(int argc, char* argv[]) try
{
  detect_params dp;
  dp.sep = NULL;
  dp.defType = unknown_type;
  dp.detectThr = defaultDetectThr;
  dp.labels = true;
  dp.exact = false;
  vector<string> names;

  int arg;
  while((arg = getopt(argc, argv, "t:T:elcd:u:m:n:")) != -1)
    switch(arg)
    {
    case 't':
      dp.defType = parseType(optarg);
      break;

    case 'e':
      dp.exact = !dp.exact;
      break;

    case 'l':
      dp.labels = !dp.labels;
      break;

    case 'c':
      dp.coalesce = !dp.coalesce;
      break;

    case 'd':
      dp.sep = optarg;
      break;

    case 'T':
      parseColType(dp.colTypes, optarg);
      break;

    case 'u':
      uniqueTokens(dp.undefStr, optarg);
      break;

    case 'm':
      dp.detectThr = strtoul(optarg, NULL, 10);
      break;

    case 'n':
      tokenize(names, optarg, ",");
      break;

    default:
      return EXIT_FAILURE;
    }

  // check args
  argc -= optind;
  if(argc < 1)
  {
    cerr << argv[0] << ": bad parameters:\n"
	 << "Usage: " << argv[0] << " [-tTelcdumn] input [input ...]\n\n"
	 << "  -t type:\tuse TYPE type for all columns, do not autodetect\n"
	 << "  -T col:type\tuse TYPE for the specified COLumn\n"
	 << "  -e:\t\tensure EXACTness of all types/floating point conversions\n"
	 << "  -l:\t\tdo not read column labels from the first row\n"
	 << "  -c:\t\tcoalesce separators (consecutive separators count as one)\n"
	 << "  -d sep:\tuse SEP as the column separator, do not autodetect\n"
	 << "  -u str:\tspecify a custom comma-separated list of undefined values\n"
	 << "  -m thr:\tcolumn type autodetection minimum THReshold (default: " << defaultDetectThr << ")\n"
	 << "  -n str:\tassign sheet names for each input file\n"
	 << "\n"
	 << "TYPE can be integer, double or string\n"
	 << "default undefined values: \"" << defaultUndefStr << "\"\n";
    return EXIT_FAILURE;
  }

  // defaults
  setlocale(LC_ALL, "");
  if(!dp.undefStr.size()) uniqueTokens(dp.undefStr, defaultUndefStr);

  // open comm with helper
  FILE* comm = popen(helperCmd, "w");
  if(!comm) throw("popen failed");

  const char* inFile;
  for(size_t argn = 0; (inFile = argv[optind + argn]); ++argn)
  {
    // open the files
    named_ifstream inFd(inFile);
    if(!inFd)
    {
      cerr << argv[0] << ": error: cannot open " << inFile << std::endl;
      return EXIT_FAILURE;
    }

    // loading stage
    detect_params inDp = dp;

    cerr << "loading ...\n";
    if(!inDp.sep)
    {
      detectTxt(inFd, inDp);
      inFd.seekg(0);
    }

    matrix_data md;
    md.labels = inDp.labels;
    md.m = loadTxt(inFd, inDp);

    md.colTypes.resize(md.m->size()? md.m->front().size(): 0);
    cerr << "loaded " << md.m->size() << " rows x " << md.colTypes.size()
	 << " columns from " << inFile << std::endl;
    if(!md.colTypes.size() || md.m->size() <= md.labels)
    {
      cerr << "warning: nothing to write\n";
      continue;
    }

    // classify columns
    classify(md, inDp);

    // output
    string sheetName = (argn < names.size()? names[argn]: inFile);
    cerr << "writing sheet \"" << sheetName << "\"...\n";
    output(comm, sheetName.c_str(), md, inDp);
  }

  if(pclose(comm))
  {
    cerr << argv[0] << ": error: " << helperCmd << " failed\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
catch(const runtime_error& e)
{
  cerr << argv[0] << ": " << e.what() << "\n";
  return EXIT_FAILURE;
}
