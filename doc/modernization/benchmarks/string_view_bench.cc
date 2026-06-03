//
//  Phase 1.4 micro-benchmark: const std::string & vs std::string_view
//
//  Models a parser hot path: repeatedly scanning a buffer into whitespace
//  separated tokens and classifying each token against a small keyword set.
//  The std::string variant materializes an owning std::string per token (the
//  status quo when helpers take "const std::string &"); the string_view variant
//  works on non-owning views into the original buffer (no allocation).
//
//  Build/run via doc/modernization/benchmarks/run_string_view_bench.py.
//
//  Usage: string_view_bench <mode:string|view> <iterations>
//

#include <string>
#include <string_view>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

const char *keywords[] = { "if", "then", "else", "while", "var", "return", "true", "false" };
const int n_keywords = int (sizeof (keywords) / sizeof (keywords[0]));

std::string make_input ()
{
  //  a representative expression-like blob
  std::string s;
  const char *words[] = { "var", "x", "if", "x", "then", "return", "true", "else",
                          "while", "y", "foo", "bar", "false", "baz", "return", "x" };
  const int n = int (sizeof (words) / sizeof (words[0]));
  for (int i = 0; i < 4000; ++i) {
    s += words[i % n];
    s += ' ';
  }
  return s;
}

//  Classify by owning std::string (allocates one string per token).
int classify_string (const std::string &tok)
{
  for (int i = 0; i < n_keywords; ++i) {
    if (tok == keywords[i]) {
      return i;
    }
  }
  return -1;
}

//  Classify by non-owning view (no allocation).
int classify_view (std::string_view tok)
{
  for (int i = 0; i < n_keywords; ++i) {
    if (tok == keywords[i]) {
      return i;
    }
  }
  return -1;
}

long run_string (const std::string &buf, long iters)
{
  long acc = 0;
  for (long it = 0; it < iters; ++it) {
    size_t i = 0;
    while (i < buf.size ()) {
      while (i < buf.size () && buf[i] == ' ') {
        ++i;
      }
      size_t j = i;
      while (j < buf.size () && buf[j] != ' ') {
        ++j;
      }
      if (j > i) {
        std::string tok = buf.substr (i, j - i);   //  owning copy + allocation
        acc += classify_string (tok);
      }
      i = j;
    }
  }
  return acc;
}

long run_view (const std::string &buf, long iters)
{
  std::string_view all (buf);
  long acc = 0;
  for (long it = 0; it < iters; ++it) {
    size_t i = 0;
    while (i < all.size ()) {
      while (i < all.size () && all[i] == ' ') {
        ++i;
      }
      size_t j = i;
      while (j < all.size () && all[j] != ' ') {
        ++j;
      }
      if (j > i) {
        std::string_view tok = all.substr (i, j - i);  //  no allocation
        acc += classify_view (tok);
      }
      i = j;
    }
  }
  return acc;
}

}

int main (int argc, char **argv)
{
  if (argc < 3) {
    std::fprintf (stderr, "usage: %s <string|view> <iterations>\n", argv[0]);
    return 2;
  }
  std::string mode = argv[1];
  long iters = std::atol (argv[2]);
  std::string buf = make_input ();

  long acc = (mode == "view") ? run_view (buf, iters) : run_string (buf, iters);

  //  print so the work cannot be optimized away
  std::printf ("%s acc=%ld\n", mode.c_str (), acc);
  return 0;
}
