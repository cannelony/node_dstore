#include "DStore.h"
//#include "ionary.h"
#include "Element.h"
#include <iostream>
#include <vector>
#include <cstddef> // size_t
#include <cstring> // memcpy
#include <cstdio>
#include <fstream> // read from files
#include <sstream>
#include <string>
#include <cmath>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdexcept>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm> //foreach

#define PAGE_SIZE 4096
#define LOG2_PAGE_SIZE 12
#define INIT_NUM_PAGES 1
#define GOLDEN_RATIO 1.618f

#define MAX(x, y) (x > y ? x : y)

////////////////////////////////////////////////////////////////////

DStore::DStore(Dictionary& dict) : dict_(dict)
{
  int openResult = ::open(SUBJECT_MAP, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (openResult < 0) {
    throw std::invalid_argument("File could not be opened or created.");
  }
  struct stat st;
  fd_ = openResult;
  fstat(fd_, &st);
  map(MAX(st.st_size / PAGE_SIZE, INIT_NUM_PAGES));

  // restore the offsetmap
  restoreStates();
}

////////////////////////////////////////////////////////////////////

DStore::DStore(const std::string& fileName, Dictionary& dict) : dict_(dict)
{
  int openResult = ::open(fileName.c_str(), O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (openResult < 0) {
    throw std::invalid_argument("File could not be opened or created.");
  }
  struct stat st;
  fd_ = openResult;
  fstat(fd_, &st);
  map(MAX(st.st_size / PAGE_SIZE, INIT_NUM_PAGES));

  // restore the offsetmap
  restoreStates();
}

////////////////////////////////////////////////////////////////////

DStore::~DStore()
{
  writeStates();
  unmap();
  close(fd_);
}

////////////////////////////////////////////////////////////////////

Element DStore::find (const SubjectID& id)
{
//  std::cout << "DSTORE size of subjects_ " << subjects_.size() << std::endl;

  Element result(dict_);

  // fill result element if it exists in the dstore otherwise return empty element
  if (subjects_.count(id) > 0) {
    // set subjectID
    result.setSubject(id);
    // get the range of subjectids from offsetmap
    auto range = subjects_.equal_range(id);
    // iterate
    for (auto it = range.first; it != range.second; ++it) {
//        std::cout << "offset " << it->second << std::endl;
        PropertyCount num;
        ::memcpy(&num, map_ + it->second, sizeof(PropertyCount));
//        std::cout << "DSTORE read property count from mapped file " << num << std::endl;
        // read properties
        char* propertyPos = map_ + it->second + sizeof(PropertyCount);
        for (unsigned int i=0; i < num; i++) {
          // create objectvector for each property, which contains the objects
          ObjectVector objectVector;
          PropertyHeader h;
          ::memcpy(&h, propertyPos, sizeof(PropertyHeader));
          propertyPos += sizeof(PropertyHeader);
          // read objectvalue
          for (unsigned int j=0; j < h.oc; j++) {
            ObjectStruct o;
            ::memcpy(&o, propertyPos, sizeof(ObjectStruct));
//            std::cout << "<" << dict_.Find(it->first) << "> <" << dict_.Find(h.id) << "> \"" << dict_.Find(o.v) << "\"" << std::endl;
            propertyPos += sizeof(ObjectStruct);
            // push object to the objectvector
            objectVector.push_back(o);
          }
          // at the end add the property to the result
          result.addProperty(h.id, objectVector);
        }
    }
    return result;
  }

  return result;
}

////////////////////////////////////////////////////////////////////

bool DStore::hasSubject(const SubjectID& id)
{
  //Dictionary dict("dict.bin");
  //SubjectID subjectID = dict_.HasKey(subject);
//  std::cout << "DSTORE hasSubject - parameter subjectid " << id << " offsetmap size " << subjects_.size() << std::endl;
  for (auto pair : subjects_) {
    if (pair.first == id) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////

//TODO restore pos_
void DStore::restoreStates()
{
  std::ifstream offset_dstore;
  std::string line;
  offset_dstore.open(OFFSET_STORE);
  if (offset_dstore.is_open()) {
    // restore
    while (offset_dstore.good()) {
      std::getline(offset_dstore, line);
      // validate string > 0 and contains * as decollator
      if (line.size() != 0 && line.find('*') != std::string::npos) {
        std::istringstream f(line);
        std::string firstelement;
        std::string secondelement;
        // explode string
        std::getline(f,firstelement,'*');
        std::getline(f,secondelement,'*');
        // set std::pair for unordered_map
        std::pair<SubjectID,Offset> pair;
        pair.first = std::atoi(firstelement.c_str());
        pair.second = std::atoi(secondelement.c_str());
        // output
//        std::cout << "first " << firstelement << std::endl;
//        std::cout << "second " << secondelement << std::endl;
//        std::cout << pair.first << std::endl;
//        std::cout << pair.second << std::endl;
        // insert into OffsetMap
        subjects_.insert(pair);
      } else if (line.size() != 0){
//        std::cout << "RESTORE pos_ " << line << std::endl;
        std::ptrdiff_t mapOffset = atoi(line.c_str());
        pos_ = map_ + mapOffset;
      }
    }
    offset_dstore.close();
//    std::cout << "size of OffsetMap " << subjects_.size() << std::endl;
  } else {
    std::cout << "unable to open file" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////

//TODO write pos_
void DStore::writeStates()
{
//  std::cout << "called writeStates" << std::endl;
  // add test entry
  //subjects_.insert(SubjectPropertyPair(1321,3123));
  std::ptrdiff_t mapOffset = pos_ - map_;
//  std::cout << "WRITE pos_ " << mapOffset << std::endl;
  // write unordered_multimap to offset.dstore
  std::fstream offset_dstore;
  offset_dstore.open(OFFSET_STORE,  std::fstream::out | std::fstream::trunc);
  if (offset_dstore.is_open()) {
    offset_dstore << mapOffset << std::endl;
    for (auto pair : subjects_) {
//      std::cout << "destructor first entry " << pair.first << " second " << pair.second <<std::endl;
      offset_dstore << pair.first << "*" << pair.second << std::endl;
    }
    offset_dstore.close();
  } else {
    std::cout << "unable to write actual state" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////

void DStore::map(std::size_t numPages)
{
  // keep the current offset
  std::ptrdiff_t offset = pos_ - map_;

  // resize the backing file to the new size
  if (::ftruncate(fd_, numPages * PAGE_SIZE - 1) != 0) {
    throw std::runtime_error("Could not change file size.");
  }
  // map the file into memory
  void* map = ::mmap(0, numPages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (map == MAP_FAILED) {
    throw std::runtime_error("Could not map file.");
  }
  // the root pointer of the mapping
  map_ = static_cast<char*>(map);
  // restore position pointer
  pos_ = map_ + offset;
  // update size
  size_ = numPages * PAGE_SIZE;
  // update number of mapped pages
  numPages_ = numPages;
}

////////////////////////////////////////////////////////////////////////////////

void DStore::unmap()
{
  ::munmap(map_, size_);
}

////////////////////////////////////////////////////////////////////////////////

//void DStore::writeProperty(const DictID& propertyID, const ObjectCount& objCount)
void DStore::writeProperty(const std::pair<DictID, ObjectVector>& property)
{
  std::ptrdiff_t propertyIdOffset = pos_ - map_;

  DictID propertyID = property.first;
  ObjectCount objectCount = property.second.size();

//  std::cout << "DSTORE by writeProperty propertyID " << propertyID << " objectCount " << objectCount << std::endl;

  // create PropertyHeader
  PropertyHeader h(propertyID, objectCount);

  // write PropertyHeader
  ::memcpy(map_ + propertyIdOffset, &h, sizeof(PropertyHeader));
  pos_ += sizeof(PropertyHeader);

  // insert objects
  for (unsigned int i=0; i < objectCount; i++) {
//    std::cout << "DSTORE by writeProperty objectType " << property.second[i].t << std::endl;
    writeObject(property.second[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DStore::writeObject(const ObjectStruct& objectStruct)
{
  std::ptrdiff_t objectOffset = pos_ - map_;

//  std::cout << "DSTORE by writeObject valueID " << objectStruct.v << std::endl;

  ::memcpy(map_ + objectOffset, &objectStruct, sizeof(ObjectStruct));
  pos_ += sizeof(ObjectStruct);
}

////////////////////////////////////////////////////////////////////////////////

void DStore::insert(Element& element)
{
//  std::cout << "---------------write Subject by writeSubject------------" << std::endl;
  std::ptrdiff_t subjectOffset = pos_ - map_;

  // prevent string from crossing page boundary,
  // moving it entirely to the next page
  if ((subjectOffset >> LOG2_PAGE_SIZE) < (subjectOffset + element.getSize()) >> LOG2_PAGE_SIZE) {
    subjectOffset = PAGE_SIZE * ((subjectOffset >> LOG2_PAGE_SIZE) + 1);
    pos_ = map_ + subjectOffset;
  }

  // remap if necessary
  if ((subjectOffset + element.getSize()) > size_) {
    unmap();
    map(floorf(GOLDEN_RATIO * numPages_));
    subjectOffset = pos_ - map_;
  }

  SubjectID subjectID = element.getSubjectID();
  PropertyCount propertyCount = element.getProperties().size();

//  std::cout << "DSTORE sizeof RESOURCE by writeSubject " << element.getSize() << std::endl;
//  std::cout << "DSTORE subjectID RESOURCE by writeSubject " << subjectID << std::endl;
//  std::cout << "DSTORE propertyCount RESOURCE by writeSubject " << propertyCount << std::endl;

  // insert offset to OffsetMap subjects_
  subjects_.insert(std::pair<SubjectID,Offset>(subjectID, subjectOffset));

  // insert PropertyCount
  ::memcpy(map_ + subjectOffset, &propertyCount, sizeof(PropertyCount));
  pos_ += sizeof(PropertyCount);

  // insert properties
  for (auto property : element.getProperties()) {
//    std::cout << "DSTORE propertyID " << property.first << " ObjectCount " << property.second.size() << std::endl;
    writeProperty(property);
  }
}

////////////////////////////////////////////////////////////////////////////////

Element DStore::createElement()
{
  Element element(dict_);
  return element;
}
