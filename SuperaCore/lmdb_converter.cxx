#ifndef __SUPERA_IMG_HOLDER_CXX__
#define __SUPERA_IMG_HOLDER_CXX__

#include "lmdb_converter.h"
#include "supera_exception.h"
namespace larcaffe {

  namespace supera {

    lmdb_converter::lmdb_converter()
      : converter_base("lmdb_converter")
      , _lmdb(nullptr)
      , _dbname("output_supera.mdb")
    {}

    void lmdb_converter::initialize()
    {
      _lmdb = new db::LMDB();
      _lmdb->Open(_dbname, db::NEW);
    }

    void lmdb_converter::store()
    {
      if(!_txn) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__) << "Cannot execute transaction in lmdb before initialization (call initialize())" << std::endl;
	throw supera_exception();
      }
      
      std::string tmp_data;

      data().SerializeToString(&tmp_data);

      auto txn = _lmdb->NewTransaction();
      txn->Put(_event_key,tmp_data);
      txn->Commit();
      delete txn;
      
    }

    void lmdb_converter::finalize()
    {
      _lmdb->Close();
      delete _lmdb;
      _lmdb = nullptr;
    }
    
  }

}

#endif
