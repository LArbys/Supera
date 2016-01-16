////////////////////////////////////////////////////////////////////////
// Class:       Yolo
// Module Type: analyzer
// File:        Yolo_module.cc
//
// Generated at Wed Dec 30 15:55:44 2015 by Taritree Wongjirad using artmod
// from cetpkgsupport v1_10_01.
////////////////////////////////////////////////////////////////////////

/*
  The YOLO module behaves slightly differently than the Supera module,
    at least enough to warrant an experimental module.
    
  YOLO does a fix crop of an image and downsamples it to 448x448 (the input for the darknet yolo network)
  It does a fix crop (specified in FCL file) of the entire event and finds the bounding boxes for interactions.
  It scales the above down to a 448x448 image.
  Along with the image, bounding boxes around the different interactions are found.
  All of this is saved in a ROOT file, which stores MCTruth info, the bounding box info, and the image itself.
  Scripts can be used to augment and manipulate this data.
  A script will also be used to output the data in the format YOLO wants, which is a folder of images, a text file
    with the names of the images, and a folder of text files with the label and bounding box information.
 */

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Optional/TFileService.h"

#include <limits>
#include <climits>

#include <TStopwatch.h>

#include "Geometry/Geometry.h" //LArCore
#include "Utilities/DetectorProperties.h" // LArData
#include "RawData/RawDigit.h" // LArData
#include "Simulation/SimChannel.h" // LArSim
#include "SimulationBase/GTruth.h" // NuTools

#include "ConverterAPI.h"
#include "SuperaCore/lmdb_converter.h"
#include "Cropper.h"
#include "LArCaffe/larbys.h"
#include "LArCaffe/LArCaffeUtils.h"
#include "FilterBase.h"

const size_t LMDB_MAP_SIZE = 1099511627776;  // 1 TB

class Yolo;

class Yolo : public art::EDAnalyzer {
public:
  explicit Yolo(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  Yolo(Yolo const &) = delete;
  Yolo(Yolo &&) = delete;
  Yolo & operator = (Yolo const &) = delete;
  Yolo & operator = (Yolo &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;
  void beginJob();
  void endJob();
  void finalize();
  
public:
  enum CropperType_t {
    kNoCropper,
    kPerEvent,
    kPerInteraction
  };

  enum TimeProfCategory_t {
    kIO_LARSOFT,
    kIO_DB,
    kIO_DATUM,
    kANALYZE_TOTAL,
    kTIMEPROFCATEGORYMAX
  };

private:

  std::vector<larcaffe::RangeArray_t> ImageArray(const art::Event& e);
  std::vector<larcaffe::RangeArray_t> findBoundingBoxes(const art::Event& e);
  void getMCTruth( art::Event const & e );

  // Declare member data here.
  //std::vector<std::vector<larcaffe::supera::converter_base*> > _db_v;
  //::larcaffe::supera::ConverterAPI _lar_api;
  ::larcaffe::supera::Cropper _cropper;
  CropperType_t _cropper_type;
  ::larcaffe::logger _logger;
  ::larcaffe::RangeArray_t _wiretime_range_hard_v;
  std::vector<bool> _plane_enable_v;
  //int _nfills_before_write;
  std::vector<std::string> _producer_v;
  std::vector<double> _time_prof_v;
  double _event_counter;
  std::vector< larcaffe::supera::FilterBase* > _filter_list;

  // ROOT tree members
  void setupTrees();

  TTree* m_ImageTree;
  std::vector<short>** m_planeImages; //< [planeid][row-major 2D image]
  int m_event; //< event ID
  int m_subrun; //< subrun ID
  int m_run; //< run ID
  float m_Enu; //< neutrino energy (0 if cosmic)
  int m_mode; //< interaction mode (-1 if cosmic)
  int m_nuscatter; // neutrino scattering code
  int m_flavor; //< neutrino flavor (-1 if cosmic)
  int m_nticks; //< image height in time ticks
  int m_wires;  //< image width in wires
  int fNPlanes; //< number of wire planes

  TTree* m_BBTree; // bouding box tree
  short m_UpLeft[2];
  short m_UpRight[2];
  short m_LoRight[2];
  short m_LoLeft[2];
  char m_label[50];
  


};

Yolo::Yolo(fhicl::ParameterSet const & p)
  : EDAnalyzer(p)
  , _cropper_type((CropperType_t)(p.get<unsigned short>("CropperType")))
  , _logger("Yolo")
  , _plane_enable_v(p.get<std::vector<bool> >("EnablePlane"))
  , _nfills_before_write(0)
  , _producer_v(p.get<std::vector<std::string> >("Producers"))
  , _time_prof_v(kTIMEPROFCATEGORYMAX,0.)
  , _event_counter(0)
{

  // set number of planes
  art::ServiceHandle<geo::Geometry> geom;
  if(_plane_enable_v.size() != geom->Nplanes()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "EnablePlane parameter must be length " << geom->Nplanes() << " (# of planes) boolean array!" << std::endl;
    throw ::larcaffe::larbys();
  }
  fNPlanes = geom->Nplanes();

  // set verbosity
  ::larcaffe::msg::Level_t vlevel = (larcaffe::msg::Level_t)(p.get<unsigned short>("Verbosity",1));
  _logger.set(vlevel);
  _lar_api.set_verbosity(vlevel);
  _cropper.set_verbosity(vlevel);

  // setup tree
  art::ServiceHandle<art::TFileService> ana_file;
  m_ImageTree = ana_file->make<TTree>( "imgtree", "Tree Containing Image and its MC Truth Info" );
  m_BBTree = ana_file->make<TTree>( "bbtree", "Bounding Box Tree and its label" );
  
  // Allocate vectors for planes
  m_planeImages = new std::vector<short>*[fNPlanes];
  for (int p=0; p<fNPlanes; p++) {
    m_planeImages[p] = new std::vector<short>;
  }

  // setup branches
  setupTrees( fNPlanes );

  // cropper hard limit
  std::vector<std::pair<int,int> > range_v = p.get<std::vector<std::pair<int,int> > >("HardLimitRange");
  if(range_v.size() != (geom->Nplanes() + 1)) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "HardLimitRange must be length " << geom->Nplanes() + 1 << " (wire plane count + 1 for time)!" << std::endl;
    throw ::larcaffe::larbys();
  }
  for(size_t plane=0; plane<range_v.size(); ++plane)
    _lar_api.SetRange(range_v[plane].first, range_v[plane].second, plane);

  _wiretime_range_hard_v = _lar_api.Ranges();

  // cropper parameters
  fhicl::ParameterSet cropper_params = p.get<fhicl::ParameterSet>("CropperConfig");
							       
  if(cropper_params.is_empty()) {
    if(_cropper_type != kNoCropper) {
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	<< "Cropper requested but configuration is empty!" << std::endl;
      throw ::larcaffe::larbys();
    }
    else if(_logger.normal()) {
      _logger.LOG(::larcaffe::msg::kNORMAL,__FUNCTION__,__LINE__)
	<< "There will be no cropping done for this process." << std::endl;
    }
  }else {
    // attempt to fill the parameters
    std::vector<unsigned int> cropper_config;
    cropper_config.push_back( cropper_params.get< unsigned int >( "TimePadding" ) );
    cropper_config.push_back( cropper_params.get< unsigned int >( "WirePadding" ) );
    cropper_config.push_back( cropper_params.get< unsigned int >( "TimeTargetSize" ) );
    cropper_config.push_back( cropper_params.get< unsigned int >( "WireTargetSize" ) );
    cropper_config.push_back( cropper_params.get< unsigned int >( "CompressionFactor" ) );
    if(cropper_config.size()==5){
      // we got them all. setup the cropper.
      _cropper.configure(cropper_config[0],
			 cropper_config[1],
			 cropper_config[2],
			 cropper_config[3],
			 cropper_config[4]);
    }else{
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	<< "Unexpected length of cropper configuration (length=" << cropper_config.size() 
	<< ") ... must be length 5 unsigned int array" << std::endl;
      throw ::larcaffe::larbys();
    }
  }//end of if cropper_param not empty

  // setup filters: can remove images
  std::vector<std::string> filter_names = p.get< std::vector<std::string> >( "ImageFilters" );
  fhicl::ParameterSet filter_params = p.get< fhicl::ParameterSet >( "FilterConfigs" );
  for ( std::vector<std::string>::iterator it_string=filter_names.begin(); it_string!=filter_names.end(); it_string++ ) {
    std::string filter_name = *it_string;
    ::larcaffe::supera::FilterBase* filter = ::larcaffe::supera::FilterBase::create( filter_name );
    fhicl::ParameterSet params = filter_params.get< fhicl::ParameterSet >( filter_name );
    filter->configure( params );
    _filter_list.emplace_back( filter );
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
  
}
			   
void Yolo::beginJob() {
}

std::vector<larcaffe::RangeArray_t> Yolo::setEventCrop(const art::Event& e) 
{
  art::ServiceHandle<geo::Geometry> geom;

  std::vector<larcaffe::RangeArray_t> image_v;

  // the fixed crop
  image_v.push_back(_wiretime_range_hard_v);
  
  return image_v;
}

std::vector<larcaffe::RangeArray_t> Yolo::findBoundingBoxes(const art::Event& e) 
{
  // we group tracks belong to same interaction and make a bounding box for it

  art::ServiceHandle<geo::Geometry> geom;

  std::vector<larcaffe::RangeArray_t> image_v;

  TStopwatch fWatch; fWatch.Start();
  art::Handle<std::vector<sim::MCTrack> > mctHandle;
  e.getByLabel("mcreco",mctHandle);
  _time_prof_v[kIO_LARSOFT] += fWatch.RealTime();
  if(!mctHandle.isValid()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Missing MCTrack info (cannot make image-per-interaction)!" << std::endl;
    throw ::larcaffe::larbys();
  }
   
  // create sets of interactions
  std::map<unsigned int,std::vector<sim::MCTrack> > interaction_m;
  for(auto const& mct : *mctHandle) {
  
    // look to see if ancestor in map
    auto iter = interaction_m.find(mct.AncestorTrackID());
    
    // if no ancestor found, add it to map
    if(iter == interaction_m.end())
      iter = interaction_m.emplace(mct.AncestorTrackID(),std::vector<sim::MCTrack>()).first;
    // otherwise, add it to the vector of mctracks
    (*iter).second.push_back(mct);
  }
    
  // now find bounding box of each interaction
  for(auto const& int_pair : interaction_m) {

    auto range_array = _cropper.Format(_cropper.WireTimeBoundary(int_pair.second));

    auto const& time_range = range_array.back();
      
    for( size_t plane=0; plane < geom->Nplanes(); ++plane ) {

      auto const& wire_range = range_array[plane];

      if(wire_range.first < wire_range.second && time_range.first < time_range.second) {
	
	image_v.push_back(range_array);

	break;
	
      }
      
    }
    
  }//end of interaction loop

  return image_v;
}//end of findboundingboxes
	
void Yolo::getMCTruth( art::Event const & e ) {
  // Sets the MC truth variables to be stored in m_ImageTree
  
  // GENIE data to get interaction mode and neutrino energy if possible
  art::Handle< std::vector<simb::GTruth> > genietruth;
  art::Handle( "generator", genietruth );
  if ( !genietruth.isValid() ) {
    _logger.LOG(::larcaffe::msg::kINFO, __FUNCTION__,__LINE__) << "No GENIE Truth. Must be Cosmic Event" << std::endl;
    m_flavor = -1;
    m_mode = -1;
    m_Enu = 0.0;
    return;
  }

  if ( genietruth.size()!=1 ) {
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Unexpected number of GTruth objectS" << std::endl;
    throw ::larcaffe::larbys();
  }
  m_mode = genietruth.at(0).fGint;
  m_nuscatter = genietruth.at(0).fGscatter;
  m_flavor = genietruth.at(0).fProbePDG; // a guess

  // Get neutrino energy
  m_Enu = genietruth.at(0).fProbeP4.E();

}

		   

void Yolo::analyze(art::Event const & e)
{
  TStopwatch pWatchAnalyze,pWatchDB,pWatchDatum,pWatchLArIO;
  pWatchAnalyze.Start();

  // Implementation of required member function here.
  if(_logger.debug())
    _logger.LOG(::larcaffe::msg::kDEBUG,__FUNCTION__,__LINE__) << "Load RawDigits Handle" << std::endl;

  //
  // Determine region size
  //
  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Extracting image array" << std::endl;
  auto region_v = setEventCrop(e);
  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Extracted " << region_v.size() << " images!" << std::endl;

  // Event Code
  m_event  = (int)e.event();
  m_run    = (int)e.run();
  m_subrun = (int)e.subRun();

  // Get MC Truth
  getMCTruth( e );
  
  // Label
  if ( m_mode==-1 )
    sprintf(m_label, "%d_%d_%d_cosmic", m_run, m_subrun, m_event );
  else
    sprintf(m_label, "%d_%d_%d_nu_%d", m_run, m_subrun, m_event, m_mode );

  //
  // Determine bounding boxes
  //
  auto bboxes = findBoundingBoxes(e);
  for ( auto const& range : bboxes ) {
    // (x,y) = (wire, time 0toX)
    m_UpLeft[0] = range.at(1).at(0);
    m_UpLeft[1] = range.at(0).at(1);

    m_LoLeft[0] = range.at(1).at(0);
    m_LoLeft[1] = range.at(0).at(0);

    m_UpRight[0] = range.at(1).at(1);
    m_UpRight[1] = range.at(0).at(1);

    m_LoRight[0] = range.at(1).at(1);
    m_LoRight[1] = range.at(0).at(0);
    
    m_BBTree->Fill();
  }

  //
  // Save region image
  //

  art::ServiceHandle<geo::Geometry> geom;

  if(!_producer_v[0].empty()) {
    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Saving RawDigit... " << std::endl;

    pWatchLArIO.Start();
    art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
    e.getByLabel(_producer_v[0], digitVecHandle);
    _time_prof_v[kIO_LARSOFT] += pWatchLArIO.RealTime();
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
	  
	  auto const& wire_range = range_v[plane];

	  if(wire_range.first == wire_range.second && time_range.first == time_range.second) continue;

	  // copy data into image
	  m_nticks = time_range.second-time_range.first+1;
	  m_wires  = wire_range.second-wire_range.first+1;
	  
	  larcaffe::Image img( m_nticks, m_wires );
	  
	  for ( auto const& wf : *digitVecHandle ) {
	    unsigned int ch = wf.Channel();
	    auto const wire_id = geom->ChannelToWire(ch).front();
	    if(wf.NADC() <= time_range.second) {
	      logger().LOG(::larcaffe::msg::kERROR,__FUNCTION__,__LINE__)
		<< "Found an waveform length " << wf.NADC()
		<< " which is shorter than set limit max " << time_range.second
		<< std::endl;
	      throw ::larcaffe::larbys();
	    }
	    
	    bool inrange = (  wire_range.first <= wire_id.Wire && wire_range.second >= wire_id.Wire );

	    if (!inrange )
	      continue;

	    img( 0, wire_id.Wire - wire_range.first,
		 (short*)(&(wf.ADCs()[time_range.first])),
		 time_range.second - time_range.first+1 );

	    
	  }//end of wire loop

	  // filter: add the ability to reject images
	  bool keep = true;
	  for ( std::vector< larcaffe::supera::FilterBase* >::iterator it_filters=_filter_list.begin(); it_filters!=_filter_list.end(); it_filters++ ) {
	    if( !(*it_filters)->doWeKeep( *db ) ) {
	      keep = false;
	      break;
	    }
	  }
	  if ( !keep ) {
	    continue;
	  }

	  // compress image and bounding boxes
	  if ( _cropper.TargetWidth() < img.width() || _cropper.TargetHeight() < img.height() ) {
	    img.compress( _cropper.TargetHeight(), _cropper.TargetWidth() );
	  }

	  // transfer image to ROOT variables
	  m_planeImages[plane].resize( img.height()*img.width() );
	  for ( int w=0; w<img.width(); w++) {
	    for (int t=0; t<img.height(); t++) {
	      m_planeImages[plane].at( img.height()*w + t ) = img.pixel( t, w );
	    }
	  }

	}//end of loop over planes

	// fill tree
	
	std::cout<<"image " << range_index << " @ " << plane << "saved..." << std::endl;
	// cout about bounding boxes

      }// end of image crop ranges
    }//if tests ok

    m_ImageTree->Fill();

  }// if RawDigits is available

  

  // deal with using Wire or Hits later
  // Code goes here
  
  _time_prof_v[kANALYZE_TOTAL] += pWatchAnalyze.RealTime();
  _event_counter += 1;
}


void Yolo::endJob() {

  if(_logger.info())
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Writing output... " << std::endl;      

//   TStopwatch pWatchDB;
//   pWatchDB.Start();
//   for(auto& p_v : _db_v) {
//     for(auto& p : p_v) {

//       if(!p) continue;
      
//       p->write();
//       p->finalize();
      
//     }
//   }
//   double time = pWatchDB.RealTime();
//   _time_prof_v[kIO_DB] += time;
//   _time_prof_v[kANALYZE_TOTAL] += time;

  if(_logger.normal()){

    double diff=_time_prof_v[kANALYZE_TOTAL];
    for(size_t i=0; i<_time_prof_v.size(); ++i) {
      if(i == kANALYZE_TOTAL) continue;
      diff -= _time_prof_v[i];
    }

    _logger.LOG(::larcaffe::msg::kNORMAL,__FUNCTION__,__LINE__)
      << "Simple time profile record" << std::endl
      << "        [0] analyze time total ... " << _time_prof_v[kANALYZE_TOTAL] / _event_counter << " [sec/event] " << std::endl
      << "        [1] IO LArSoft ........... " << _time_prof_v[kIO_LARSOFT]    / _event_counter << " [sec/event] " << std::endl
      << "        [2] IO Output DB  ........ " << _time_prof_v[kIO_DB]         / _event_counter << " [sec/event] " << std::endl
      << "        [3] Datum Buffer Fill .... " << _time_prof_v[kIO_DATUM]      / _event_counter << " [sec/event] " << std::endl
      << std::endl
      << "        [*] Unaccounted time ([0]-rest) ... " << diff / _event_counter << " [sec/event] " << std::endl
      << std::endl;

  } 

  Yolo::setupTrees(int nplanes) {
    // ImageTree
    // ---------
    // add the plane images
    for (int i=0; i<nplanes; i++) {
      char branchname[20];
      sprintf( branchname, "img_plane%d", i );
      m_ImageTree->Branch( branchname, &(*m_planeImages[i]) );
    }
    m_ImageTree->Branch( "run", &m_run, "run/I" );
    m_ImageTree->Branch( "subrun", &m_subrun, "subrun/I" );
    m_ImageTree->Branch( "event", &m_event, "event/I" );
    m_ImageTree->Branch( "Enu", &m_Enu, "Enu/F" );
    m_ImageTree->Branch( "mode", &m_mode, "mode/I" );
    m_ImageTree->Branch( "nuscatter", &m_nuscatter, "nuscatter/I" );
    m_ImageTree->Branch( "flavor", &m_flavor, "flavor/I" );
    m_ImageTree->Branch( "nticks", &m_nticks, "nticks/I" );
    m_ImageTree->Branch( "nwires", &m_wires, "nwires/I" );

    // Bounding Box Tree
    // -----------------
    m_BBTree->Branch( "UpLeft", m_UpLeft, "UpLeft[2]/S" );
    m_BBTree->Branch( "UpRight", m_UpRight, "UpRight[2]/S" );
    m_BBTree->Branch( "LoRight", m_LoRight, "LoRight[2]/S" );
    m_BBTree->Branch( "LoLeft", m_LoLeft, "LoLeft[2]/S" );
    m_BBTree->Branch( "label", m_label, "label[50]/C" );

  }
}


DEFINE_ART_MODULE(Yolo)
