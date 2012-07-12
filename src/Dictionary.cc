#include "Dictionary.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdexcept>
#include <utility>

#define PAGE_SIZE 4096
#define LOG2_PAGE_SIZE 12
#define INIT_NUM_PAGES 32
#define GOLDEN_RATIO 1.618f
#define MAGIC_NUMBER 0x00000000d1c70000ULL

#define MAX(x, y) (x > y ? x : y)

////////////////////////////////////////////////////////////////////////////////

struct Dictionary::EntryHeader {
  KeyType id;
  KeyType next;
  std::size_t lsize;
  EntryHeader() : id(0), next(0), lsize(0) {};
  EntryHeader(KeyType id_, KeyType next_, std::size_t lsize_)
    : id(id_), next(next_), lsize(lsize_) {};
};

////////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary()
{
  int openResult = ::fileno(::tmpfile());
  if (openResult < 0) {
    throw std::invalid_argument("Could not create backing file.");
  }
  fd_ = openResult;
  map(INIT_NUM_PAGES);
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary(const std::string& fileName)
{
  struct stat st;
  bool createdFlag(false);
  if (stat(fileName.c_str(), &st) < 0) {
    // file does not exists
    createdFlag = true;
  }

  int openResult = ::open(fileName.c_str(), O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (openResult < 0) {
    throw std::invalid_argument("File could not be opened or created.");
  }

  fd_ = openResult;
  fstat(fd_, &st);
  // TODO: if the file has not been created by us, it does not
  // necessarily have the size of a PAGE_SIZE multiple
  map(MAX(st.st_size / PAGE_SIZE, INIT_NUM_PAGES));

  if (createdFlag) {
    // create magic number
    writeValue(pos_ - map_, MAGIC_NUMBER);
    pos_ += sizeof(MAGIC_NUMBER);
    writeValue(pos_ - map_, 0x123);
    pos_ += sizeof(std::size_t);
  } else {
    // check magic number
    uint64_t magic(readValue<unsigned long long>(0));
    if (magic != MAGIC_NUMBER) {
      throw std::invalid_argument("File cannot be used as dictionary backing.");
    }
    restoreState();
  }
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::~Dictionary()
{
  saveState();
  unmap();
  close(fd_);
}

////////////////////////////////////////////////////////////////////////////////

std::size_t Dictionary::writeHeader(const EntryHeader& entry, std::size_t offset)
{
  // prevent string from crossing page boundary,
  // moving it entirely to the next page
  if ((offset >> LOG2_PAGE_SIZE) < (offset + sizeof(EntryHeader) + entry.lsize) >> LOG2_PAGE_SIZE) {
    offset = PAGE_SIZE * ((offset >> LOG2_PAGE_SIZE) + 1);
    pos_ = map_ + offset;
  }

  // remap if necessary
  if ((offset + sizeof(EntryHeader) + entry.lsize) > size_) {
    unmap();
    map(floorf(GOLDEN_RATIO * numPages_));
    offset = pos_ - map_;
  }

  // write entry
  writeValue(offset, entry);

  // advance pointer
  pos_ += sizeof(EntryHeader);

  return offset;
}

////////////////////////////////////////////////////////////////////////////////

std::size_t Dictionary::writeLiteral(const std::string& lit, std::size_t offset)
{
  // write literal
  lit.copy(map_ + offset, lit.size());

  // advance pointer
  // TODO: ofsetting to dword boundary seems to be slower!
  // pos_ += (((offset + lit.size() - 1) >> 3) + 1) * 8 - offset;
  pos_ += lit.size();

  return offset;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::EntryHeader Dictionary::readEntry(const KeyType offset)
{
  return readValue<EntryHeader>(offset);
}

////////////////////////////////////////////////////////////////////////////////

std::string Dictionary::readLiteral(const KeyType offset, std::size_t length)
{
  return std::string(map_ + offset, length);
}

////////////////////////////////////////////////////////////////////////////////

std::size_t Dictionary::hash(const std::string& str)
{
  std::size_t hash(0);
  for (std::size_t i(0); i != str.size(); ++i) {
    hash = hash * 101 + str[i];
  }

  return hash;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::KeyType Dictionary::Lookup(const std::string& lit)
{
  KeyModifier emptyModifier;
  return Lookup(lit, emptyModifier);
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::KeyType Dictionary::Lookup(const std::string& lit, const KeyModifier& keyModifier)
{
  // hash the string an use the hash as an index into the id map
  std::size_t hash = hasher_(lit);
  auto it(ids_.find(hash));
  KeyType literalID;

  if (it != std::end(ids_)) {
    // we found an id for hash but it could be a collision
    // check the catual string values
    literalID = it->second;
    std::ptrdiff_t parentOffset = literals_[literalID - 1];
    EntryHeader entry(0, parentOffset, 0);

    // check an entry and all overflow buckets
    // until there are no more entries or the strings match
    do {
      parentOffset = entry.next;
      entry = readEntry(parentOffset);
      if (entry.lsize == lit.size()) {
        if (::memcmp(map_ + parentOffset + sizeof(EntryHeader), lit.data(), entry.lsize) == 0) {
          // strings matched, return the actual id
          return entry.id;
        }
      }
    } while (entry.next);

    // no string did match, so we create a new overflow entry
    std::ptrdiff_t offset = pos_ - map_;
    writeValue(parentOffset + sizeof(KeyType), offset);
    return writeEntry(lit, offset, keyModifier);
  }

  // no id was found, so we just create a new entry
  std::ptrdiff_t offset = pos_ - map_;
  literalID = writeEntry(lit, offset, keyModifier);
  ids_[hash] = literalID;
  return literalID;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::KeyType Dictionary::HasKey(const std::string& lit)
{
  // hash the string an use the hash as an index into the id map
  std::size_t hash = hasher_(lit);
  auto it(ids_.find(hash));
  KeyType literalID;

  if (it != std::end(ids_)) {
    // we found an id for hash but it could be a collision
    // check the catual string values
    literalID = it->second;
    std::ptrdiff_t parentOffset = literals_[literalID - 1];
    EntryHeader entry(0, parentOffset, 0);

    // check an entry and all overflow buckets
    // until there are no more entries or the strings match
    do {
      parentOffset = entry.next;
      entry = readEntry(parentOffset);
      if (entry.lsize == lit.size()) {
        if (::memcmp(map_ + parentOffset + sizeof(EntryHeader), lit.data(), entry.lsize) == 0) {
          // strings matched, return the actual id
          return entry.id;
        }
      }
    } while (entry.next);

    return 0;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::KeyType Dictionary::writeEntry(const std::string& lit, std::ptrdiff_t offset,
    const KeyModifier& keyModifier)
{
  KeyType literalID = nextKey_++;
  if (static_cast<bool>(keyModifier)) {
    keyModifier(literalID);
  }
  EntryHeader newEntry(literalID, 0, lit.size());
  // write the header
  offset = writeHeader(newEntry, offset);
  // write the character data
  writeLiteral(lit, offset + sizeof(EntryHeader));
  // save the new offset
  literals_.push_back(offset);
  return literalID;
}

////////////////////////////////////////////////////////////////////////////////

std::string Dictionary::Find(KeyType key) const
{
  // we should read the complete entry header here,
  // but we only need the size and the string data
  std::ptrdiff_t offset = literals_[key - 1] + 2 * sizeof(KeyType);
  char* pos = map_ + offset;

  // read size
  std::size_t size;
  ::memcpy(&size, pos, sizeof(size));
  pos += sizeof(size);

  // instantiate string from byte pointer and size
  return std::string(pos, size);
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::unmap()
{
  ::munmap(map_, size_);
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::map(std::size_t numPages)
{
  // keep the current offset
  std::ptrdiff_t offset = pos_ - map_;

  // resize the backing file to the new size
  if (::ftruncate(fd_, numPages * PAGE_SIZE) != 0) {
    throw std::runtime_error("Could not change file size.");
  }
  // map the file into memory
  void* map = ::mmap(0, numPages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (map == MAP_FAILED) {
    throw std::runtime_error("Could not map file.");
  }
  // the root pointer of the mapping
  map_ = static_cast<char*>(map);
  // restore psoition pointer
  pos_ = map_ + offset;
  // update size
  size_ = numPages * PAGE_SIZE;
  // update number of mapped pages
  numPages_ = numPages;
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::restoreState()
{
  // get the map offset
  std::ptrdiff_t mapOffset(readValue<std::ptrdiff_t>(8));
  std::size_t writeOffset = mapOffset;
  std::size_t indexSize(readValue<std::size_t>(writeOffset));
  writeOffset += sizeof(std::size_t);
  nextKey_ = indexSize + 1;
  for (std::size_t i(0); i != indexSize; ++i) {
    KeyType key(readValue<KeyType>(writeOffset));
    writeOffset += sizeof(KeyType);
    KeyType val(readValue<KeyType>(writeOffset));
    writeOffset += sizeof(KeyType);
    ids_.insert(std::make_pair(key, val));
  }
  for (std::size_t i(0); i != indexSize; ++i) {
    KeyType val(readValue<KeyType>(writeOffset));
    writeOffset += sizeof(KeyType);
    literals_.push_back(val);
  }
  // zero-fill to end
  pos_ = map_ + mapOffset;
  memset(pos_, 0, size_ - mapOffset);
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::saveState()
{
  std::size_t indexSize(ids_.size());
  std::size_t writeSize(3 * indexSize * sizeof(KeyType) + sizeof(KeyType));
  std::ptrdiff_t mapOffset = pos_ - map_;

  // remap if necessary
  if ((mapOffset + writeSize) > size_) {
    unmap();
    map(numPages_ + ::ceilf(writeSize / PAGE_SIZE));
    mapOffset = pos_ - map_;
  }

  std::ptrdiff_t writeOffset = mapOffset;

  // write size
  writeValue(mapOffset, indexSize);
  writeOffset += sizeof(indexSize);
  // write map
  for (auto& pair : ids_) {
    writeValue(writeOffset, pair);
    writeOffset += sizeof(pair);
  }
  // write literals
  for (auto& value : literals_) {
    writeValue(writeOffset, value);
    writeOffset += sizeof(value);
  }
  // write map begin/next offset
  writeValue(0x8, mapOffset);
}
