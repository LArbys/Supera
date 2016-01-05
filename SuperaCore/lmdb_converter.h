/**
 * \file supera_lmdb_converter.h
 *
 * \ingroup SuperaCore
 * 
 * \brief Class def header for a class lmdb_converter
 *
 * @author kazuhiro
 */

/** \addtogroup SuperaCore

    @{*/
#ifndef SUPERA_LMDB_CONVERTER_H
#define SUPERA_LMDB_CONVERTER_H

#include <iostream>
#include "converter_base.h"

namespace db {
  class LMDB;
  class Transaction;
}

#ifndef __CINT__
#include "db_lmdb.h"
#include "caffe.pb.h"
#endif

namespace larcaffe {

  namespace supera {
    /**
       \class lmdb_converter
       User defined class lmdb_converter ... these comments are used to generate
       doxygen documentation!
    */
    class lmdb_converter : public converter_base {
      
    public:
      
      /// Default constructor
      lmdb_converter();
      
      /// Default destructor
      ~lmdb_converter(){}

      void set_database(const std::string dbname)
      { _dbname = dbname; }

      void set_key(const std::string key)
      { _event_key = key; }

      void initialize();

      void store();

      void finalize();

    private:

      db::LMDB *_lmdb;
      db::Transaction *_txn;
      std::string _dbname,_event_key;

    };
  }
}

#endif
/** @} */ // end of doxygen group 

