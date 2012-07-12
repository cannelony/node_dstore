#include "Dictionary.h"
#include <cassert>
#include <iostream>

int main()
{
  // some strings
  std::string s1("string 1");
  std::string s2("string 2");
  std::string s3("string 3");
  std::string s4("string 4");
  std::string s5("string 5");

  Dictionary::KeyType k1, k2, k3, k4, k5, k6;

  // create first dictionary
  // for the desctructor to be called
  // we wrap it in a block
  {
    Dictionary dict("test.data");
    k1 = dict.Lookup(s1);
    k2 = dict.Lookup(s2);
    k3 = dict.Lookup(s3);
  }

  // create second dictionary on same file
  Dictionary dict2("test.data");
  // size should not increase
  assert(dict2.Size() == 3);
  // key should be the same
  k4 = dict2.Lookup(s1);
  assert(dict2.Size() == 3);
  assert(k4 == k1);
  k5 = dict2.Lookup(s2);
  assert(dict2.Size() == 3);
  assert(k5 == k2);
  k6 = dict2.Lookup(s3);
  assert(dict2.Size() == 3);
  assert(k6 == k3);
}
