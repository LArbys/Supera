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
#include "SuperaCore/converter_base.h"

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
      lmdb_converter(std::string dbname="default_db");
      
      /// Default destructor
      ~lmdb_converter(){}

      void initialize();

      void store_image(const std::string& key);

      void write();

      void finalize();

    private:

      db::LMDB *_lmdb;
      db::Transaction *_txn;
      std::string _dbname;
    };
  }
}

#endif
/** @} */ // end of doxygen group 

