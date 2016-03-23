#ifndef __LARCV_IOMANAGER_CXX__
#define __LARCV_IOMANAGER_CXX__

#include "IOManager.h"
#include "larbys.h"
#include "ProductMap.h"

namespace larcv {

  template <class T>
  bool IOManager<T>::initialize()
  {
    if(_io_mode != kREAD) {
      
      if(_out_file_name.empty()) throw larbys("Must set output file name!");
      _out_file = TFile::Open(_out_file_name.c_str(),"RECREATE");
      
    }
    
    if(_io_mode != kWRITE) {
      
      prepare_input();
      if(!_tree_entries) {
	LARCV_ERROR() << "Found 0 entries from input files..." << std::endl;
	return false;
      }
    }

    _tree_index = 0;
    _prepared = true;
    return true;
  }

  template <class T>
  size_t IOManager<T>::register_producer(const std::string& name)
  {
    auto in_iter = _key_m.find(name);
    std::string tree_name = ProductName<T>() + "_" + name + "_tree";
    std::string tree_desc = name + " tree";
    std::string br_name = ProductName<T>() + "_" + name + "_branch";
    
    if(in_iter != _key_m.end()) return (*in_iter).second;
    size_t id = _in_tree_v.size();
    _key_m.emplace(name,id);
    
    auto data_ptr = new std::vector<T>;
    _product_v.emplace_back(data_ptr);
    
    if(_io_mode != kWRITE) {
      auto in_tree_ptr = new TChain(tree_name.c_str(),tree_desc.c_str());
      in_tree_ptr->SetBranchAddress(br_name.c_str(), &data_ptr);
      _in_tree_v.emplace_back(in_tree_ptr);
      _in_tree_index_v.emplace_back(kINVALID_SIZE);
    }	
    
    if(_io_mode != kREAD) {
      _out_file->cd();
      auto out_tree_ptr = new TTree(tree_name.c_str(),tree_desc.c_str());
      out_tree_ptr->SetBranchAddress(br_name.c_str(), &data_ptr);
      _out_tree_v.emplace_back(out_tree_ptr);
    }
    
    return id;
  }

  template <class T>
  void IOManager<T>::prepare_input()
  {
    if(_in_tree_v.size()) {
      LARCV_CRITICAL() << "Cannot call prepare_input before calling reset()!" << std::endl;
      throw larbys();
    }

    for(size_t i=0; i<_in_file_v.size(); ++i) {
      
      auto const& fname = _in_file_v[i];
      auto const& dname = _in_dir_v[i];
      
      TFile *fin = TFile::Open(fname.c_str(),"READ");
      if(!fin) {
	LARCV_CRITICAL() << "Open attempt failed for a file: " << fname << std::endl;
	throw larbys();
      }
      
      LARCV_NORMAL() << "Opening a file in READ mode: " << fname << std::endl;
      TDirectoryFile* fin_dir = 0;
      if(dname.empty()) fin_dir = fin;
      else {
	TObject* obj = fin->Get(dname.c_str());
	
	if(!obj || std::string(obj->ClassName())!="TDirectoryFile") {
	  LARCV_CRITICAL() << "Could not locate TDirectoryFile: " << dname << std::endl;
	  throw larbys();
	}
	
	fin_dir = (TDirectoryFile*)obj;
      }
      
      TList* key_list = fin_dir->GetListOfKeys();
      TIter key_iter(key_list);
      while(1) {
	TObject* obj = key_iter.Next();
	if(!obj) break;
	obj = fin_dir->Get(obj->GetName());
	LARCV_DEBUG() << "Found object " << obj->GetName() << " (type=" << obj->ClassName() << ")" << std::endl;
	
	if(std::string(obj->ClassName())!="TTree") {
	  LARCV_DEBUG() << "Skipping " << obj->GetName() << " ... (not TTree)" << std::endl;
	  continue;
	}
	
	std::string obj_name = obj->GetName();
	
	char c[2] = "_";
	if(obj_name.find_first_of(c) > obj_name.size() ||
	   obj_name.find_first_of(c) == obj_name.find_last_of(c)) {
	    LARCV_INFO() << "Skipping " << obj->GetName() << " ... (not LArCV TTree)" << std::endl;
	    continue;
	}
	
	std::string type_name( obj_name.substr(0,obj_name.find_first_of(c)) );
	std::string suffix( obj_name.substr(obj_name.find_last_of(c)+1, obj_name.size()-obj_name.find_last_of(c)) );
	std::string producer_name( obj_name.substr(obj_name.find_first_of(c)+1,obj_name.find_last_of(c)-obj_name.find_first_of(c)-1) );
	
	if(suffix != "tree") {
	  LARCV_INFO() << "Skipping " << obj->GetName() << " ... (not LArCV TTree)" << std::endl;
	  continue;
	}
	
	if(type_name != ProductName<T>()) {
	  LARCV_INFO() << "Ignoring TTree " << obj->GetName() << " (not " << ProductName<T>() << " type)" << std::endl;
	  continue;
	}

	auto id = register_producer(producer_name);
	_in_tree_v[id]->AddFile(fname.c_str());
	break;
      }
    }

    if(_in_tree_v.empty()) {
      _tree_entries = 0;
      return;
    }

    // Get tree entries
    _tree_entries = kINVALID_SIZE;
    for(auto const& t : _in_tree_v) { _tree_entries = std::min(_tree_entries,t->GetEntries()); }

  }

  template <class T>
  bool IOManager<T>::read_entry(const size_t index)
  {
    if(_io_mode == kWRITE) {
      LARCV_WARNING() << "Nothing to read in kWRITE mode..." << std::endl;
      return false;
    }
    if(!_prepared) {
      LARCV_CRITICAL() << "Cannot be called before initialize()!" << std::endl;
      throw larbys();
    }
    if(index >= _tree_entries) {
      LARCV_ERROR() << "Input only has " << _tree_entries << " entries!" << std::endl;
      return false;
    }
    _tree_index = index;
  }

  template <class T>
  bool IOManager<T>::save_entry()
  {
    if(!_prepared) {
      LARCV_CRITICAL() << "Cannot be called before initialize()!" << std::endl;
      throw larbys();
    }

    if(_io_mode == kREAD) {
      LARCV_ERROR() << "Cannot save in READ mode..." << std::endl;
      return false;
    }

    for( auto& t : _out_tree_v ) t->Fill();

    if(_io_mode == kWRITE) {
      _tree_index   += 1;
      _tree_entries += 1;
    }

    return true;
  }

  template <class T>
  size_t IOManager<T>::producer_id(const std::string& producer) const
  {
    auto iter = _key_m.find(producer);
    if(iter == _key_m.end()) return kINVALID_SIZE;
    return (*iter).second;
  }

  template <class T>
  std::vector<T>* IOManager<T>::get_data(const std::string& producer)
  {
    if(producer.empty()) {
      LARCV_CRITICAL() << "Empty producer name (invalid)" << std::endl;
      throw larbys();
    }

    auto id = producer_id(producer);

    if(id == kINVALID_SIZE) {
      if(_io_mode == kREAD) {
	LARCV_ERROR() << "Invalid producer requested:" << producer << std::endl;
	return nullptr;
      }
      id = register_producer(producer);
      for(size_t i=0; i<_tree_entries; ++i) _out_tree_v[id]->Fill();
      _out_tree_v[id]->GetEntry(_tree_index);
    }

    return get_data(id);
  }

  template <class T>
  std::vector<T>* IOManager<T>::get_data(const size_t id)
  {
    if(id >= _product_v.size()) {
      LARCV_ERROR() << "Invalid producer ID requested:" << id << std::endl;
      return nullptr;
    }

    if(_io_mode != kWRITE && _in_tree_index_v[id] != _tree_index) {
      _in_tree_v[id]->GetEntry(_tree_index);
      _in_tree_index_v[id] =_tree_index;
    }
    return _product_v[id];
  }

  template <class T>
  void IOManager<T>::finalize()
  {
    if(_io_mode != kWRITE) for(auto& t : _in_tree_v) {delete t;};

    if(_io_mode != kREAD) {
      _out_file->cd();
      for(auto& t : _out_tree_v) { t->Write(); }
      _out_file->Close();
      _out_file = nullptr;
    }

    for(auto& p : _product_v) { delete p; }

    reset();
  }

  template <class T>
  void IOManager<T>::reset()
  {
    _in_tree_v.clear();
    _in_tree_index_v.clear();
    _out_tree_v.clear();
    _product_v.clear();    
    _tree_index = kINVALID_SIZE;
    _tree_entries = 0;
    _prepared = false;
    _out_file_name = "";
    _in_file_v.clear();
    _in_dir_v.clear();
    _key_m.clear();

  }

}
#endif
