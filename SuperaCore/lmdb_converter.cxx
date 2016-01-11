#ifndef __SUPERA_IMG_HOLDER_CXX__
#define __SUPERA_IMG_HOLDER_CXX__

#include "lmdb_converter.h"
#include "LArCaffe/larbys.h"
namespace larcaffe {

  namespace supera {

    lmdb_converter::lmdb_converter(const std::string dbname)
      : converter_base("lmdb_converter")
      , _lmdb(nullptr)
      , _txn(nullptr)
      , _dbname(dbname)
    {}

    void lmdb_converter::initialize()
    {
      _lmdb = new db::LMDB();
      _lmdb->Open(_dbname, db::NEW);
    }

    void lmdb_converter::store_image(const std::string& key)
    {
      if(key.empty()) {
	logger().LOG(msg::kCRITICAL,__FUNCTION__)
	  << "Cannot have an empty key!" << std::endl;
	throw larbys();
      }
      if(!_txn) _txn = _lmdb->NewTransaction();
      
      std::string tmp_data;

      data().SerializeToString(&tmp_data);

      _data.clear_float_data();

      _txn->Put(key,tmp_data);
    }

    void lmdb_converter::write()
    {
      if(!_txn) {
	logger().LOG(msg::kWARNING,__FUNCTION__) << "No transaction to be stored..." << std::endl;
	return;
      }

      _txn->Commit();
      delete _txn;
      _txn = nullptr;
      
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
