#ifndef DSTORE_H
#define DSTORE_H

#include <string>

class DStore
{
public:
  // default constructor
  DStore();
  // desctructor
  ~DStore();
  // insert
  void insert(const std::string& json);
  void insertFile(const std::string& filename);
  void insertMultiple(const std::string& json);
  // handle requests
  std::string find(const std::string& subject);
  bool hasSubject(const std::string& subject);
private:
  // private parts
};

#endif
