#ifndef CAFFE_UTIL_DB_HPP
#define CAFFE_UTIL_DB_HPP

#include <string>

#include "caffe.pb.h"

namespace db {

enum Mode { READ, WRITE, NEW };

class Cursor {
 public:
  Cursor() { }
  virtual ~Cursor() { }
  virtual void SeekToFirst() = 0;
  virtual void Next() = 0;
  virtual std::string key() = 0;
  virtual std::string value() = 0;
  virtual bool valid() = 0;

  //DISABLE_COPY_AND_ASSIGN(Cursor);
};

class Transaction {
 public:
  Transaction() { }
  virtual ~Transaction() { }
  virtual void Put(const std::string& key, const std::string& value) = 0;
  virtual void Commit() = 0;

  //DISABLE_COPY_AND_ASSIGN(Transaction);
};

class DB {
 public:
  DB() { }
  virtual ~DB() { }
  virtual void Open(const std::string& source, Mode mode) = 0;
  virtual void Close() = 0;
  virtual Cursor* NewCursor() = 0;
  virtual Transaction* NewTransaction() = 0;

  //DISABLE_COPY_AND_ASSIGN(DB);
};


}  // namespace db

#endif  // CAFFE_UTIL_DB_HPP
