#include "DStore.h"
#include "Dictionary.h"
#include "Element.h"
#include <iostream>

int main()
{
  Dictionary dictionary("dict.bin");
  DStore dstore(dictionary);

  Element r = dstore.createElement();
  r.setSubject("http://example.org/about");
  r.addProperty("http://purl.org/dc/elements/1.1/creator", "literal", "Anna Wilder");
  r.addProperty("http://purl.org/dc/elements/1.1/creator", "literal", "Wilder");
  r.addProperty("http://purl.org/dc/elements/1.1/creator", "bnode", "Anna");
  r.addProperty("http://purl.org/dc/elements/1.1/title", "literal", "Anna's Homepage", "en");
  r.addProperty("http://purl.org/dc/elements/1.1/title", "literal", "Homepage Anna", "en");
  r.addProperty("http://purl.org/dc/elements/1.1/title", "literal", "Homepage Anna2", "en");
  r.addProperty("http://xmlns.com/foaf/0.1/maker", "bnode", "_:person");
  r.addProperty("http://purl.org/dc/elements/1.1/title2", "literal", "3. Property", "en");

  std::cout << "sizeof Element r " << r.getSize() << std::endl;

  Element r2 = dstore.createElement();
  r2.setSubject("http://example.org/about2");
  r2.addProperty("http://purl.org/dc/elements/1.1/title", "literal", "about2", "en");
  r2.addProperty("http://purl.org/dc/elements/1.1/title", "literal", "about2 - second object to proeprty title", "en");

  dstore.insert(r);
  dstore.insert(r2);

  std::cout << "about hasSubject " << dstore.hasSubject(dictionary.HasKey("http://example.org/about")) << std::endl;
  std::cout << "foobar hasSubject " << dstore.hasSubject(dictionary.HasKey("foobar")) << std::endl;

  // multimap test
  Element r_again = dstore.createElement();
  r_again.setSubject("http://example.org/about");
  r_again.addProperty("http://purl.org/dc/elements/1.1/title3", "literal", "3. Property", "en");

  dstore.insert(r_again);

  // read element
  DStore::SubjectID id = dictionary.HasKey("http://example.org/about");
  Element read = dstore.find(id);
  std::cout << "read " << dictionary.Find(read.getSubjectID()) << " Size " << read.getSize() << std::endl;
  for (auto prop : read.getProperties()) {
    std::cout << dictionary.Find(prop.first) << " - " << prop.second.size() << std::endl;
  }

  DStore::SubjectID id2 = dictionary.HasKey("foobar");
  Element r4 = dstore.find(id2);
  std::cout << "Should be zero because of not in dstore " << r4.getSubjectID() << std::endl;

  return 0;
}
