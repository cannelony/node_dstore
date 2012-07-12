#ifndef DSTORE_H
#define DSTORE_H

#include "Dictionary.h"
#include <string>
#include <cstddef> // size_t
#include <cstring> // memcpy
#include <unordered_map>
#include <functional>
#include <vector>

#define OFFSET_STORE "offset.dstore"
#define SUBJECT_MAP "dstore.bin"
#define DICTIONARY_MAP "dict.bin"

class Element;

class DStore
{
public:
  // DictID
  typedef std::size_t DictID;
  // SubjectID
  typedef std::size_t SubjectID;
  // Offset
  typedef std::size_t Offset;
  // PropertyCount
  typedef std::size_t PropertyCount;
  // ObjectCount
  typedef std::size_t ObjectCount;
  // enum for different types
  enum Type { LITERAL, TYPED_LITERAL, URI, BNODE };
  // ObjectStruct
  struct ObjectStruct {
    // type
    // 0 - literal
    // 1 - typded-literal
    // 2 - resource
    // 3 - bnode
    Type t;
    // value
    DictID v;
    // language
    DictID l;
    // datatype
    DictID d;
    ObjectStruct() : t(LITERAL), v(0), l(0), d(0) {};
    ObjectStruct(Type t_, DictID v_, DictID l_, DictID d_)
      : t(t_), v(v_), l(l_), d(d_) {};
  };
  // PropertyHeader
  struct PropertyHeader {
    // propertyid
    DictID id;
    // object count
    ObjectCount oc;
    PropertyHeader() : id(0), oc(0) {};
    PropertyHeader(DictID id_, DictID oc_)
      : id(id_), oc(oc_) {};
  };
  // default constructor
  DStore(Dictionary& dict);
  // constructor
  DStore(const std::string& fileName, Dictionary& dict);
  // desctructor
  ~DStore();
  // return all existing data of a subject
  Element find(const SubjectID& id);
  bool hasSubject(const SubjectID& id);
  // write subject
  void insert(Element& resource);
  // create Element and return it
  Element createElement();
private:
  // dictionary
  Dictionary& dict_;
  // objectVector
  typedef std::vector<ObjectStruct> ObjectVector;
  // save subject ids and offset in mapped file
  typedef std::unordered_multimap<SubjectID, Offset> OffsetMap;
  // pair of subject id and offset
  typedef std::pair<SubjectID,Offset> SubjectPropertyPair;
  // pair of propertyID and a vector of objectstructs
  typedef std::pair<DictID,ObjectVector> PropertyPair;
  // declare an OffsetMap
  OffsetMap subjects_;
  std::size_t size_;
  // size of the mapping in pages
  std::size_t numPages_;
  // file descriptor of backing file
  int fd_;
  // root pointer of the mapping
  char* map_ = nullptr;
  // pointer to the beginning of the next entry
  char* pos_ = nullptr;
  // creates a mapping of size pages
  void map(std::size_t size);
  // destroys the current mapping
  void unmap();
  // restore states on startup
  void restoreStates();
  // write actual state to file on destroy
  void writeStates();
  // write property
  void writeProperty(const std::pair<DictID, ObjectVector>& property);
  // write object struct
  void writeObject(const ObjectStruct& objectStruct);
};

#endif
