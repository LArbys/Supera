////////////////////////////////////////////////////////////////////////
// Class:       Supera
// Module Type: analyzer
// File:        Supera_module.cc
//
// Generated at Wed Dec 30 15:55:44 2015 by Taritree Wongjirad using artmod
// from cetpkgsupport v1_10_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <limits>
#include <climits>

#include "Geometry/Geometry.h"
#include "Utilities/DetectorProperties.h"
#include "RawData/RawDigit.h"
#include "Simulation/SimChannel.h"
#include "ConverterAPI.h"
#include "SuperaCore/lmdb_converter.h"
#include "Cropper.h"
#include "LArCaffe/larbys.h"

const size_t LMDB_MAP_SIZE = 1099511627776;  // 1 TB

class Supera;

class Supera : public art::EDAnalyzer {
public:
  explicit Supera(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  Supera(Supera const &) = delete;
  Supera(Supera &&) = delete;
  Supera & operator = (Supera const &) = delete;
  Supera & operator = (Supera &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;
  void beginJob();
  void endJob();
  void finalize();

  void MCRegion(const std::vector<sim::SimChannel>& simch_v,
		::larcaffe::RangeArray_t& wire_range_v,
		::larcaffe::RangeArray_t& time_range_v) const;
  
public:
  enum CropperType_t {
    kNoCropper,
    kPerEvent,
    kPerInteraction
  };

private:

  std::vector<larcaffe::RangeArray_t> ImageArray(const art::Event& e);

  // Declare member data here.
  std::vector<std::vector<larcaffe::supera::converter_base*> > _db_v;
  ::larcaffe::supera::ConverterAPI _lar_api;
  ::larcaffe::supera::Cropper _cropper;
  CropperType_t _cropper_type;
  ::larcaffe::logger _logger;
  ::larcaffe::RangeArray_t _wiretime_range_hard_v;
  std::vector<bool> _plane_enable_v;
  int _nfills_before_write;
  std::vector<std::string> _producer_v;
};

Supera::Supera(fhicl::ParameterSet const & p)
  : EDAnalyzer(p)
  , _cropper_type((CropperType_t)(p.get<unsigned short>("CropperType")))
  , _logger("Supera")
  , _plane_enable_v(p.get<std::vector<bool> >("EnablePlane"))
  , _nfills_before_write(0)
  , _producer_v(p.get<std::vector<std::string> >("Producers"))
{

  art::ServiceHandle<geo::Geometry> geom;
  if(_plane_enable_v.size() != geom->Nplanes()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "EnablePlane parameter must be length " << geom->Nplanes() << " (# of planes) boolean array!" << std::endl;
    throw ::larcaffe::larbys();
  }

  ::larcaffe::msg::Level_t vlevel = (larcaffe::msg::Level_t)(p.get<unsigned short>("Verbosity",1));
  _logger.set(vlevel);
  _lar_api.set_verbosity(vlevel);
  _cropper.set_verbosity(vlevel);

  std::string dbname = p.get<std::string>("DatabaseName","output_supera.mdb");
  if(dbname.empty()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "DatabaseName parameter cannot be an empty string!" << std::endl;
    throw ::larcaffe::larbys();
  }

  std::vector<std::pair<int,int> > range_v = p.get<std::vector<std::pair<int,int> > >("HardLimitRange");
  if(range_v.size() != (geom->Nplanes() + 1)) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "HardLimitRange must be length " << geom->Nplanes() + 1 << " (wire plane count + 1 for time)!" << std::endl;
    throw ::larcaffe::larbys();
  }
  for(size_t plane=0; plane<range_v.size(); ++plane)
    _lar_api.SetRange(range_v[plane].first, range_v[plane].second, plane);

  _wiretime_range_hard_v = _lar_api.Ranges();

  std::vector<unsigned int> cropper_config = p.get<std::vector<unsigned int> >("CropperConfig");
  if(cropper_config.empty()) {
    if(_cropper_type != kNoCropper) {
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	<< "Cropper requested but configuration is empty!" << std::endl;
      throw ::larcaffe::larbys();
    }
    else if(_logger.normal()) {
      _logger.LOG(::larcaffe::msg::kNORMAL,__FUNCTION__,__LINE__)
	<< "There will be no cropping done for this process." << std::endl;
    }
  }else if(cropper_config.size()==4){
    _cropper.configure(cropper_config[0],
		       cropper_config[1],
		       cropper_config[2],
		       cropper_config[3]);
  }else{
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "Unexpected length of cropper configuration (length=" << cropper_config.size() 
      << ") ... must be length 4 unsigned int array" << std::endl;
    throw ::larcaffe::larbys();
  }

  if(_producer_v.size()!=3) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "Producers parameter must be length 3 string array (producer names for raw digit, wire, and hit respectively)!" << std::endl
      << "Leave empty string to skip using specific product. An array of 3 empty strings will also cause this message!" << std::endl;
    throw ::larcaffe::larbys();
  }
  _logger.LOG(::larcaffe::msg::kNORMAL,__FUNCTION__,__LINE__)
    << "Using data producers: (RawDigit, Wire, Hit) = (" 
    << _producer_v[0] <<", " <<  _producer_v[1] << ", " << _producer_v[2] << ")" << std::endl;

  for(size_t i=0; i<_producer_v.size(); ++i) {

    _db_v.push_back(std::vector<larcaffe::supera::converter_base*>(geom->Nplanes(),nullptr));

    if(_producer_v[i].empty()) continue;

    auto& db_v = _db_v.back();

    for(size_t plane=0; plane<geom->Nplanes(); ++plane) {

      if(!_plane_enable_v[plane]) continue;

      std::string dbname = "lmdb.";
      dbname += _producer_v[i];
      dbname += ".plane";
      dbname += std::to_string((int)plane);
      dbname += ".dat";

      db_v[plane] = new ::larcaffe::supera::lmdb_converter(dbname);
      db_v[plane]->set_verbosity(vlevel);
    }
  }
}
			   
void Supera::beginJob() {
  
  for(auto& p_v : _db_v) 
    for(auto& p : p_v) 
      if(p) p->initialize();
}

void Supera::MCRegion(const std::vector<sim::SimChannel>& simch_v,
		      ::larcaffe::RangeArray_t& wire_range_v,
		      ::larcaffe::RangeArray_t& time_range_v) const
{ return; }

std::vector<larcaffe::RangeArray_t> Supera::ImageArray(const art::Event& e) 
{
  std::vector<larcaffe::RangeArray_t> image_v;

  if(_cropper_type == kNoCropper)

    image_v.push_back(_wiretime_range_hard_v);

  else if(_cropper_type == kPerEvent) {

    art::Handle<std::vector<sim::SimChannel> > simchHandle;
    e.getByLabel("largeant",simchHandle);
    if(!simchHandle.isValid()) {
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Missing SimChannel info (cannot apply MC region cut!" << std::endl;
      throw ::larcaffe::larbys();
    }
    
    image_v.push_back(_cropper.Format(_cropper.WireTimeBoundary((*simchHandle))));

  }else if(_cropper_type == kPerInteraction) {

    art::Handle<std::vector<sim::MCTrack> > mctHandle;
    e.getByLabel("mcreco",mctHandle);
    if(!mctHandle.isValid()) {
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Missing MCTrack info (cannot make image-per-interaction)!" << std::endl;
      throw ::larcaffe::larbys();
    }
    
    std::map<unsigned int,std::vector<sim::MCTrack> > interaction_m;
    for(auto const& mct : *mctHandle) {
      
      auto iter = interaction_m.find(mct.AncestorTrackID());
      
      if(iter == interaction_m.end())
	
	iter = interaction_m.emplace(mct.AncestorTrackID(),std::vector<sim::MCTrack>()).first;

      (*iter).second.push_back(mct);
      
    }
    
    for(auto const& int_pair : interaction_m)

      image_v.push_back(_cropper.Format(_cropper.WireTimeBoundary(int_pair.second)));

  }else{

    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "Unknown cropper type: " << _cropper_type << std::endl;

    throw ::larcaffe::larbys();

  }

  return image_v;
}
			   

void Supera::analyze(art::Event const & e)
{
  // Implementation of required member function here.
  if(_logger.debug())
    _logger.LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__) << "Load RawDigits Handle" << std::endl;

  //
  // Determine region size
  //
  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Extracting image array" << std::endl;
  auto region_v = ImageArray(e);
  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Extracted " << region_v.size() << " images!" << std::endl;

  //
  // Save region image
  //

  art::ServiceHandle<geo::Geometry> geom;

  char key_char[100];
  sprintf( key_char, "%07d_%05d_%06d", e.run(), e.subRun(), e.event() );
  std::string key_str = key_char;

  if(!_producer_v[0].empty()) {
    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Saving RawDigit... " << std::endl;

    art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
    e.getByLabel(_producer_v[0], digitVecHandle);
    if ( !digitVecHandle.isValid() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) 
	<< "Missing RawDigits by " << _producer_v[0] << " Skipping." << std::endl;
    else if ( digitVecHandle->empty() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) << "Empty RawDigits info. skipping." << std::endl;
    else {
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	<< "Extracting " << region_v.size() << " images for RawDigit " << std::endl;

      for(size_t range_index=0; range_index < region_v.size(); ++range_index) {

	auto const& range_v = region_v[range_index];

	auto const& time_range = range_v.back();

	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {
	  
	  auto& db = _db_v[0][plane];
	  if(!db) continue;

	  auto const& wire_range = range_v[plane];

	  for(size_t i=0; i<= geom->Nplanes(); ++i) _lar_api.SetRange(0,0,i);
	  _lar_api.SetRange(wire_range.first,wire_range.second,plane);
	  _lar_api.SetRange(time_range.first,time_range.second,geom->Nplanes());

	  db->set_image_size(time_range.second - time_range.first + 1,
			     wire_range.second - wire_range.first + 1);

	  _lar_api.Copy(*digitVecHandle,*db);

	  std::string tmp_key = key_str + "_" + std::to_string(range_index);

	  db->store_image(tmp_key);
	}
      }
    }
  }

  if(!_producer_v[1].empty()) {
    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Saving Wire... " << std::endl;

    art::Handle< std::vector<recob::Wire> > wireVecHandle;
    e.getByLabel(_producer_v[1], wireVecHandle);
    if ( !wireVecHandle.isValid() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) 
	<< "Missing Wires by " << _producer_v[1]<< " Skipping." << std::endl;
    else if ( wireVecHandle->empty() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) << "Empty Wires info. skipping." << std::endl;
    else {
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	<< "Extracting " << region_v.size() << " images for Wire!" << std::endl;      
      for(size_t range_index=0; range_index < region_v.size(); ++range_index) {

	auto const& range_v = region_v[range_index];

	auto const& time_range = range_v.back();

	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {
	  
	  auto& db = _db_v[1][plane];
	  if(!db) continue;

	  auto const& wire_range = range_v[plane];

	  for(size_t i=0; i<= geom->Nplanes(); ++i) _lar_api.SetRange(0,0,i);
	  _lar_api.SetRange(wire_range.first,wire_range.second,plane);
	  _lar_api.SetRange(time_range.first,time_range.second,geom->Nplanes());

	  db->set_image_size(time_range.second - time_range.first + 1,
			     wire_range.second - wire_range.first + 1);

	  _lar_api.Copy(*wireVecHandle,*db);

	  std::string tmp_key = key_str + "_" + std::to_string(range_index);

	  db->store_image(tmp_key);
	}
      }
    }
  }

  if(!_producer_v[2].empty()) {
    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Saving Hit... " << std::endl;

    art::Handle< std::vector<recob::Hit> > hitVecHandle;
    e.getByLabel(_producer_v[1], hitVecHandle);
    if ( !hitVecHandle.isValid() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) 
	<< "Missing Hits by " << _producer_v[1]<< " Skipping." << std::endl;
    else if ( hitVecHandle->empty() )
      _logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) << "Empty Hits info. skipping." << std::endl;
    else {
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	<< "Extracting " << region_v.size() << " images for Hit " << std::endl;
      for(size_t range_index=0; range_index < region_v.size(); ++range_index) {

	auto const& range_v = region_v[range_index];

	auto const& time_range = range_v.back();

	for(size_t plane=0; plane < geom->Nplanes(); ++plane) {
	  
	  auto& db = _db_v[2][plane];
	  if(!db) continue;

	  auto const& wire_range = range_v[plane];

	  for(size_t i=0; i<= geom->Nplanes(); ++i) _lar_api.SetRange(0,0,i);
	  _lar_api.SetRange(wire_range.first,wire_range.second,plane);
	  _lar_api.SetRange(time_range.first,time_range.second,geom->Nplanes());

	  db->set_image_size(time_range.second - time_range.first + 1,
			     wire_range.second - wire_range.first + 1);

	  _lar_api.Copy(*hitVecHandle,*db);

	  std::string tmp_key = key_str + "_" + std::to_string(range_index);

	  db->store_image(tmp_key);
	}
      }
    }
  }

  ++_nfills_before_write;
  if(_nfills_before_write && _nfills_before_write%100 == 0) {

    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Writing output... " << std::endl;    
    for(auto& p_v : _db_v) 
      for(auto& p : p_v) 
	if(p) p->write();

  }
  
}


void Supera::endJob() {

  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Writing output... " << std::endl;      

  for(auto& p_v : _db_v) {
    for(auto& p : p_v) {

      if(!p) continue;
      
      p->write();
      p->finalize();
      
    }
  }

}


DEFINE_ART_MODULE(Supera)