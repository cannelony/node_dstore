#ifndef Element_H
#define Element_H

#include "DStore.h"
#include "Dictionary.h"
#include <string>
#include <cstddef> // size_t
#include <unordered_map>
#include <functional>
#include <vector>
#include <iostream>

class Element
{
public:
  typedef std::vector<DStore::ObjectStruct> ObjectVector;
  typedef std::unordered_map<DStore::DictID, ObjectVector> PropertyMap;
  Element();
  Element(Dictionary& dict);
  ~Element();
  void setSubject(const std::string& subjectName);
  void setSubject(const DStore::DictID& subjectID);
  void addProperty(const std::string& uri, const std::string& type, const std::string& value, const std::string& language = "", const std::string& datatype = "");
  void addProperty(const DStore::DictID& propertyID, const ObjectVector& objectVector);
  std::size_t getSize();
  DStore::SubjectID getSubjectID();
  PropertyMap getProperties();
private:
  typedef std::pair<DStore::DictID,ObjectVector> PropertyPair;
  Dictionary& dict_;
  PropertyMap properties_;
  DStore::SubjectID subjectID_;
};

#endif
