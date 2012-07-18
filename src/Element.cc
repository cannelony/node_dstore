#include "Element.h"

/////////////////////////////////////////////////////////////////////////////////////////

Element::Element() : dict_(dict_)
{
  subjectID_ = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

Element::Element(Dictionary& dict) : dict_(dict)
{
  subjectID_ = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

Element::~Element()
{

}

/////////////////////////////////////////////////////////////////////////////////////////

void Element::setSubject(const std::string& subjectName)
{
  //Dictionary dict("dict.bin");
  subjectID_ = dict_.Lookup(subjectName);
//  std::cout << "Element Subject: " << subjectName << " SubjectID: " << subjectID_ << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Element::setSubject(const DStore::DictID& subjectID)
{
  subjectID_ = subjectID;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Element::addProperty(const std::string& uri, const std::string& type, const std::string& value, const std::string& language, const std::string& datatype)
{
  //Dictionary dict("dict.bin");
  // lookup property
  DStore::DictID propertyID = dict_.Lookup(uri);
  // set type
  DStore::Type tID;
  if (type == "literal") {
    tID = DStore::LITERAL;
  } else if (type == "uri") {
    tID = DStore::URI;
  } else if (type == "bnode") {
    tID = DStore::BNODE;
  }
  // lookup value, language, datatype
  DStore::DictID vID = dict_.Lookup(value);
  DStore::DictID lID = dict_.Lookup(language);
  DStore::DictID dID = dict_.Lookup(datatype);
  // create objectstruct
  DStore::ObjectStruct o(tID, vID, lID, dID);
  // push object to PropertyMap
  bool isDistinct = true;
  for (auto pair : properties_) {
    if (pair.first == propertyID) {
      isDistinct = false;
      break;
    }
  }
  if (isDistinct) {
    // create entry
    ObjectVector objectVector;
    objectVector.push_back(o);
    std::pair<DStore::DictID, ObjectVector> pair;
    pair.first = propertyID;
    pair.second = objectVector;
    properties_.insert(pair);
  } else {
    // use existing objectvector to push objectstruct
    properties_.at(propertyID).push_back(o);
  }

//  std::cout << "Element o: " << o.t << " value " << dict_.Find(vID) << std::endl;
//  std::cout << "Element size objectvector of " << dict_.Find(propertyID) << ": " << properties_.at(propertyID).size() << std::endl;
//  std::cout << "Element size of properties_ " << properties_.size() << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Element::addProperty(const DStore::DictID& propertyID, const ObjectVector& objectVector)
{
  PropertyPair propertyPair(propertyID, objectVector);
  properties_.insert(propertyPair);
}

/////////////////////////////////////////////////////////////////////////////////////////

std::size_t Element::getSize()
{
  std::size_t size = sizeof(DStore::PropertyCount);
  for (auto pair : properties_) {
    size += sizeof(DStore::PropertyHeader) + pair.second.size() * sizeof(DStore::ObjectStruct);
  }
  return size;
}

/////////////////////////////////////////////////////////////////////////////////////////

Element::PropertyMap Element::getProperties()
{
  return properties_;
}

/////////////////////////////////////////////////////////////////////////////////////////

DStore::SubjectID Element::getSubjectID()
{
  return subjectID_;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Element::reset() {
  properties_.clear();
  subjectID_ = 0;
}
