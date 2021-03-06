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
#include <set>

#include <TStopwatch.h>

#include "Geometry/Geometry.h" //LArCore
#include "Utilities/DetectorProperties.h" // LArData
#include "Utilities/LArProperties.h"
#include "RawData/RawDigit.h" // LArData
#include "Simulation/SimChannel.h" // LArSim
#include "SimulationBase/GTruth.h" // NuTools
#include "SimulationBase/MCTruth.h" // NuTools
#include "SimulationBase/MCNeutrino.h" // NuTools
#include "MCBase/MCTrack.h" // LArData
#include "MCBase/MCShower.h" // LArData

#include "ConverterAPI.h"
#include "SuperaCore/lmdb_converter.h"
#include "Cropper.h"
#include "LArCaffe/larbys.h"
#include "LArCaffe/LArCaffeUtils.h"
#include "FilterBase.h"
#include "ImageExtractor.h"

#include "TTree.h" // ROOT

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

  std::vector<larcaffe::RangeArray_t> setEventCrop(const art::Event& e);
  std::vector<larcaffe::RangeArray_t> findBoundingBoxes(const art::Event& e);
  void getMCTruth( art::Event const & e );
  void clearBoundingBoxes();

  // Declare member data here.
  //std::vector<std::vector<larcaffe::supera::converter_base*> > _db_v;
  //::larcaffe::supera::ConverterAPI _lar_api;
  ::larcaffe::supera::Cropper _cropper;
  ::larcaffe::supera::Cropper _cropper_interaction;
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
  void setupTrees( int nplanes );

  TTree* m_ImageTree;
  TTree* m_BBTree;
  std::vector<int>** m_planeImages; //< [planeid][row-major 2D image]
  int m_event; //< event ID
  int m_subrun; //< subrun ID
  int m_run; //< run ID
  int m_nfilledboxes;
  float m_Enu; //< neutrino energy (0 if cosmic)
  std::vector<float> m_vertex;
  std::vector<int> m_vertex_tw;
  int m_mode; //< interaction mode (-1 if cosmic)
  int m_nuscatter; // neutrino scattering code
  int m_flavor; //< neutrino flavor (-1 if cosmic)
  int m_interaction; //< neutrino interaction type (-1 cosmics)
  int m_nticks; //< image height in time ticks
  int* m_wires;  //< image width in wires [planeid]
  int fNPlanes; //< number of wire planes
  bool fUseWire;
  bool fGroupAllInteractions;
  bool fUseSimChannel;
  bool fCosmicsMode;
  bool fSingleParticleMode;
  std::string singlepname;

  // bounding boxes: stuck doing this because want to make flat branch tree
  
  // for events
  std::vector<int>** m_plane_bb_loleft_t;
  std::vector<int>** m_plane_bb_loleft_w;
  std::vector<int>** m_plane_bb_hileft_t;
  std::vector<int>** m_plane_bb_hileft_w;
  std::vector<int>** m_plane_bb_loright_t;
  std::vector<int>** m_plane_bb_loright_w;
  std::vector<int>** m_plane_bb_hiright_t;
  std::vector<int>** m_plane_bb_hiright_w;
  std::vector<std::string>* m_bbox_label;
  std::vector<int>** m_bb_planeImages;
  char m_label[50];

  // for individual bounding boxes
  int* m_plane_bbinteraction_loleft_t;
  int* m_plane_bbinteraction_loleft_w;
  int* m_plane_bbinteraction_hiright_t;
  int* m_plane_bbinteraction_hiright_w;
  char m_bblabel[100];
  int m_bb_nticks;
  int m_bb_nwires;
  
  std::vector< std::string > m_interaction_list; ///< labels for bounding boxes


};

Yolo::Yolo(fhicl::ParameterSet const & p)
  : EDAnalyzer(p)
  , _cropper_type((CropperType_t)(p.get<unsigned short>("CropperType")))
  , _logger("Yolo")
  , _plane_enable_v(p.get<std::vector<bool> >("EnablePlane"))
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
  //_lar_api.set_verbosity(vlevel);
  _cropper.set_verbosity(vlevel);

  // setup tree
  art::ServiceHandle<art::TFileService> ana_file;
  m_ImageTree = ana_file->make<TTree>( "imgtree", "Tree Containing Image and its MC Truth Info" );  // image of the entire event
  m_BBTree    = ana_file->make<TTree>( "bbtree", "Bounding Box Tree and its label" ); // images of each interaction within events
  
  // Allocate vectors for planes
  m_planeImages = new std::vector<int>*[fNPlanes];
  for (int p=0; p<fNPlanes; p++) {
    m_planeImages[p] = new std::vector<int>;
  }
  m_bb_planeImages = new std::vector<int>*[fNPlanes];
  for (int p=0; p<fNPlanes; p++) {
    m_bb_planeImages[p] = new std::vector<int>;
  }

  // setup branches
  setupTrees( fNPlanes );

  // cropper parameters
  fhicl::ParameterSet cropper_params = p.get<fhicl::ParameterSet>("EventCropperConfig");
  fhicl::ParameterSet interaction_cropper_params = p.get<fhicl::ParameterSet>("InteractionCropperConfig");
							       
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
    // fill cropper parameters

    // event cropper
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
	<< "Unexpected length of event cropper configuration (length=" << cropper_config.size() 
	<< ") ... must be length 5 unsigned int array" << std::endl;
      throw ::larcaffe::larbys();
    }

    cropper_config.clear();
    cropper_config.push_back( interaction_cropper_params.get< unsigned int >( "TimePadding" ) );
    cropper_config.push_back( interaction_cropper_params.get< unsigned int >( "WirePadding" ) );
    cropper_config.push_back( interaction_cropper_params.get< unsigned int >( "TimeTargetSize" ) );
    cropper_config.push_back( interaction_cropper_params.get< unsigned int >( "WireTargetSize" ) );
    cropper_config.push_back( interaction_cropper_params.get< unsigned int >( "CompressionFactor" ) );
    if(cropper_config.size()==5){
      // we got them all. setup the cropper.
      _cropper_interaction.configure(cropper_config[0],
				     cropper_config[1],
				     cropper_config[2],
				     cropper_config[3],
				     cropper_config[4]);
    }else{
      _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
	<< "Unexpected length of interaction cropper configuration (length=" << cropper_config.size() 
	<< ") ... must be length 5 unsigned int array" << std::endl;
      throw ::larcaffe::larbys();
    }
  }//end of if cropper_param not empty
  
  // group all interactions into one big bounding box (for neutrino mode)
  fGroupAllInteractions = p.get<bool>( "GroupAllInteractions", false );
  fUseSimChannel = p.get<bool>( "UseSimChannel", false );
  fCosmicsMode = p.get<bool>( "CosmicsMode", false );
  fSingleParticleMode = p.get<bool>("SingleParticleMode",false);
  fUseWire = p.get<bool>("UseWire",true);
  singlepname = p.get<std::string>("SingleParticleName","");

  // cropper hard limit (dependent on cropper params)
  std::vector<std::pair<int,int> > range_v = p.get<std::vector<std::pair<int,int> > >("HardLimitRange");
  if(range_v.size() != (geom->Nplanes() + 1)) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__)
      << "HardLimitRange must be length " << geom->Nplanes() + 1 << " (wire plane count + 1 for time)!" << std::endl;
    throw ::larcaffe::larbys();
  }
  _wiretime_range_hard_v.resize( range_v.size() );
  for(size_t plane=0; plane<range_v.size(); ++plane) {
    if ( range_v.at(plane).first<0 ) {
      // unspecified
      if ( (int)plane<fNPlanes ) {
	// unspecified wires: we take all of them (that will fit in multiple of target size)
	_wiretime_range_hard_v.at(plane).start = 0;
	int nfactors = (geom->Nwires(plane))/_cropper.TargetWidth();
	_wiretime_range_hard_v.at(plane).end = nfactors*_cropper.TargetWidth() - 1;
	// maybe i shuld center this
      }
      else {
	_logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Time ticks need hard range" << std::endl;
	throw ::larcaffe::larbys();
      }
    }
    else {
      // specified range
      _wiretime_range_hard_v.at(plane).start  = (unsigned int)range_v.at(plane).first;
      _wiretime_range_hard_v.at(plane).end = (unsigned int)range_v.at(plane).second;
    }
    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
      << "setting hard limit for plane=" << plane 
      << ": " << _wiretime_range_hard_v.at(plane).start << " " << _wiretime_range_hard_v.at(plane).end << std::endl;
  }
  // setup filters: can remove images
  std::vector<std::string> filter_names = p.get< std::vector<std::string> >( "ImageFilters" );
  fhicl::ParameterSet filter_params = p.get< fhicl::ParameterSet >( "FilterConfigs" );
  for ( std::vector<std::string>::iterator it_string=filter_names.begin(); it_string!=filter_names.end(); it_string++ ) {
    std::string filter_name = *it_string;
    ::larcaffe::supera::FilterBase* filter = ::larcaffe::supera::FilterFactory::get()->create( filter_name );
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
  image_v.push_back( _wiretime_range_hard_v );
  
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
  art::Handle<std::vector<sim::MCShower>> mcshHandle;
  e.getByLabel("mcreco",mcshHandle);
  if(!mcshHandle.isValid()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Missing MCShower info (cannot make image-per-interaction)!" << std::endl;
    throw ::larcaffe::larbys();
  }

  art::Handle<std::vector<sim::SimChannel> > simchHandle;
  e.getByLabel("largeant",simchHandle);
  if(!simchHandle.isValid()) {
    _logger.LOG(::larcaffe::msg::kCRITICAL,__FUNCTION__,__LINE__) << "Missing SimChannel info (cannot apply MC region cut!" << std::endl;
    throw ::larcaffe::larbys();
  }

  // this is a mess! rewrite while jobs are running
  if ( fUseSimChannel ) {
    auto range_array = _cropper.Format(_cropper.WireTimeBoundary((*simchHandle)));
    image_v.push_back(range_array);
    std::stringstream ss;
    if ( fSingleParticleMode ) {
      ss << singlepname;
    }
    else {
      ss << "neutrino_mode" << m_mode << "_flux" << m_flavor;
    }
    m_interaction_list.push_back( ss.str() );
  }
  else {
    // create sets of interactions

    // start with tracks
    std::map<unsigned int,std::vector<sim::MCTrack> > interaction_m;

    for(auto const& mct : *mctHandle) {
    
      //std::cout << "mctrack: id=" << mct.TrackID() << " pdg=" << mct.PdgCode() << " " << mct.Process() << " parent=" << mct.AncestorTrackID()  << std::endl;
      if ( fCosmicsMode ) {
	// we skip the neutrons for now, else its a fucking mess -- if we get smarter, could be interesting
	if ( mct.PdgCode()==2112 )
	  continue;
      }

      if ( fGroupAllInteractions ) {
	// look to see if ancestor in map
	auto iter = interaction_m.find(0);
	if ( iter==interaction_m.end() )
	  iter = interaction_m.emplace( 0, std::vector<sim::MCTrack>() ).first;
	(*iter).second.push_back(mct);
      }
      else {
	auto iter = interaction_m.find(mct.AncestorTrackID());
      
	// if no ancestor found, add it to map
	if( iter == interaction_m.end() )
	  iter = interaction_m.emplace(mct.AncestorTrackID(),std::vector<sim::MCTrack>()).first;
      
	// otherwise, add it to the vector of mctracks
	(*iter).second.push_back(mct);
      }
    }

    _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Made " << interaction_m.size() << " interaction groups to crop around" << std::endl;

    // get shower info
    std::set< unsigned int > decaytracks; // decay information
    std::map<unsigned int,std::vector<sim::MCShower> > shower_m;
    shower_m.emplace( 0, std::vector<sim::MCShower>() );
    for(auto const& mcsh : *mcshHandle) {
      //std::cout << "mcshower: id=" << mcsh.TrackID() << " pdg=" << mcsh.PdgCode() << " " << mcsh.Process() << " parent=" << mcsh.AncestorTrackID()  << std::endl;
      // look for decays
      if ( std::string(mcsh.Process())=="Decay" ) {
	auto ittrack=interaction_m.find( mcsh.AncestorTrackID() );
	if ( ittrack!=interaction_m.end() ) {
	  // matching
	  decaytracks.insert( mcsh.AncestorTrackID() );
	}
      }
      // load all showers in neutrino mode
      if ( fGroupAllInteractions ) {
	auto iter = shower_m.find(0);
	(*iter).second.push_back(mcsh);
      }
    }

  
    // now find bounding box of each interaction
    m_interaction_list.clear();
    for(auto const& int_pair : interaction_m) {
    
      auto range_array = _cropper_interaction.Format(_cropper_interaction.WireTimeBoundary(int_pair.second, shower_m[0]));

      // determine what the interaction is
      std::stringstream ss;
      if ( fSingleParticleMode ) {
	ss << singlepname;
      }
      else if ( fCosmicsMode ){
	const sim::MCTrack& mct0 = int_pair.second.at(0);
	if ( std::abs(mct0.PdgCode())==13 ) {
	  // muon
	  ss << "cosmic_muon";
	  if ( int_pair.second.size()>1 ) {
	    auto itmatch = decaytracks.find( int_pair.first );
	    if ( itmatch!=decaytracks.end() )
	      ss << "_decay";
	  }//end of if not single muon only
	}//end of if muon
	else if ( std::abs(mct0.PdgCode())==2212 )  {
	  ss << "cosmic_proton";
	}
	else if ( std::abs(mct0.PdgCode())==11 ) {
	  ss << "cosmic_shower";
	}
	else 
	  ss << "cosmic_other";
      }//end of if cosmics
      else {
	// neutrino mode
	ss << "neutrino_mode" << m_mode << "_flux" << m_flavor;
      }

      m_interaction_list.push_back( ss.str() );

      auto const& time_range = range_array.back();
    
      for( size_t plane=0; plane < geom->Nplanes(); ++plane ) {

	auto const& wire_range = range_array[plane];

	if(wire_range.start < wire_range.end && time_range.start < time_range.end) {
	
	  image_v.push_back(range_array);

	  break;
	
	}
      
      }
    
    }//end of interaction loop
  }//end of if not group all interactions

  return image_v;
}//end of findboundingboxes
	
void Yolo::getMCTruth( art::Event const & e ) {
  // Sets the MC truth variables to be stored in m_ImageTree
  
  // GENIE data to get interaction mode and neutrino energy if possible
  art::Handle< std::vector<simb::MCTruth> > gentruth;
  e.getByLabel( "generator", gentruth );
  if ( fCosmicsMode || fSingleParticleMode || !gentruth.isValid() ) {
    _logger.LOG(::larcaffe::msg::kINFO, __FUNCTION__,__LINE__) << "No GENIE Truth. Must be Cosmic Event" << std::endl;
    m_flavor = -1;
    m_mode = -1;
    m_interaction = -1;
    m_nuscatter = -1;
    m_Enu = 0.0;
    const std::vector<simb::MCTruth>& genie = *gentruth;
    m_Enu = genie.at(0).GetParticle(0).E();
    m_vertex.resize(4, 0.0 );
    m_vertex.at(0) = genie.at(0).GetParticle(0).Vx();
    m_vertex.at(1) = genie.at(0).GetParticle(0).Vy();
    m_vertex.at(2) = genie.at(0).GetParticle(0).Vz();
    m_vertex.at(3) = genie.at(0).GetParticle(0).T();

  }
  else {
  
    const std::vector<simb::MCTruth>& genie = *gentruth;
    if ( genie.size()!=1 ) {
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Unexpected number of MCTruth objects = " << genie.size() << std::endl;
      if ( genie.size()==0)
	throw ::larcaffe::larbys();
    }
    m_mode = genie.at(0).GetNeutrino().Mode();
    m_nuscatter = genie.at(0).GetNeutrino().CCNC();
    m_interaction = genie.at(0).GetNeutrino().InteractionType();
    m_flavor = genie.at(0).GetNeutrino().Nu().PdgCode();
    
    // Get neutrino energy
    m_Enu = genie.at(0).GetNeutrino().Nu().E();

    // Get Vertex
    m_vertex.resize(4, 0.0);
    m_vertex.at(0) = genie.at(0).GetParticle(0).Vx();
    m_vertex.at(1) = genie.at(0).GetParticle(0).Vy();
    m_vertex.at(2) = genie.at(0).GetParticle(0).Vz();
    m_vertex.at(3) = genie.at(0).GetParticle(0).T();

  }

  // Get Vertex in terms of time and wire
  art::ServiceHandle<geo::Geometry> geom;
  art::ServiceHandle<util::LArProperties> larp;
  art::ServiceHandle<util::TimeService> ts;
  //art::ServiceHandle<util::DetectorProperties> detp;
  const double drift_velocity = larp->DriftVelocity()*1.0e-3; // make it cm/ns                                                                                                                     
  //const int tick_max = detp->NumberTimeSamples();

  m_vertex_tw.resize(geom->Nplanes()+1,0);
  for (size_t iplane=0; iplane<geom->Nplanes(); iplane++) {
    geo::WireID wire_id;
    try {
      wire_id = geom->NearestWireID(m_vertex.data(), iplane);
    }
    catch (geo::InvalidWireIDError& err) {
      //std::cout << "out of bounds. using better number" << std::endl;
      wire_id.Wire = err.better_wire_number;
    }
    m_vertex_tw.at(iplane) = wire_id.Wire;
  }  
  int tick = (int)(ts->TPCG4Time2Tick(m_vertex.at(3) + (m_vertex.at(0) / drift_velocity))) + 1;
  m_vertex_tw.at(3) = tick;
  
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
  if ( fSingleParticleMode )
    sprintf(m_label, "%s_%d_%d_%d", singlepname.c_str(), m_run, m_subrun, m_event );
  else if ( fCosmicsMode )
    sprintf(m_label, "cosmics_%d_%d_%d", m_run, m_subrun, m_event );
  else
    sprintf(m_label, "nu_%d_%d_%d_mode_%d", m_run, m_subrun, m_event, m_mode );
  
  //
  // Save region image and bounding boxes
  //
  art::ServiceHandle<geo::Geometry> geom;
  
  
  if(!_producer_v[0].empty()) {
    if(_logger.info())
      _logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) << "Saving RawDigit... " << std::endl;
    
    pWatchLArIO.Start();
    art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
    art::Handle< std::vector<recob::Wire> > wireVecHandle;
    if ( fUseWire )
      e.getByLabel(_producer_v[1], wireVecHandle );
    else
      e.getByLabel(_producer_v[0], digitVecHandle);
    _time_prof_v[kIO_LARSOFT] += pWatchLArIO.RealTime();
    
    if ( fUseWire ) {
      if ( !wireVecHandle.isValid() ) {// no rawdigits 
	_logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) 
	  << "Missing Wires by " << _producer_v[1] << " Skipping." << std::endl;
	return;
      }
      else if ( wireVecHandle->empty() ) // empty rawdigits
	_logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) << "Empty Wire info. skipping." << std::endl;
      else {
	_logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Extracting " << region_v.size() << " images for Wire " << std::endl;
      }
    }
    else {
      if ( !digitVecHandle.isValid() ) { // no rawdigits
	_logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) 
	  << "Missing RawDigits by " << _producer_v[0] << " Skipping." << std::endl;
	return;
      }
      else if ( digitVecHandle->empty() ) // empty rawdigits
	_logger.LOG(::larcaffe::msg::kWARNING,__FUNCTION__,__LINE__) << "Empty RawDigits info. skipping." << std::endl;
      else {
	_logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	  << "Extracting " << region_v.size() << " images for RawDigit " << std::endl;
      }
    }
    
    // clear bounding boxes
    clearBoundingBoxes();
    // place to store image scale-down, if any
    std::vector<int> plane_compression;
    plane_compression.resize(fNPlanes+1,1);
    
    // Make an extractor
    larcaffe::supera::ImageExtractor extractor;
    
    // loop through cropping regions (should only be one for Yolo Module)
    for(size_t range_index=0; range_index < region_v.size(); ++range_index) {
      
      auto const& range_v = region_v[range_index];
      
      auto const& time_range = range_v.back();
      
      for(size_t plane=0; plane < geom->Nplanes(); ++plane) {
	
	auto const& wire_range = range_v[plane];
	_logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__, __LINE__) << "fill wire range " << wire_range.start << " " << wire_range.end << std::endl;
	
	if(wire_range.start == wire_range.end && time_range.start == time_range.end) continue;
	
	// copy data into image
	larcaffe::Image img;
	if ( fUseWire )
	  img = extractor.Extract( plane, wire_range, time_range, *wireVecHandle );
	else
	  img = extractor.Extract( plane, wire_range, time_range, *digitVecHandle );
	
	m_nticks = time_range.end-time_range.start+1;
	m_wires[plane]  = wire_range.end-wire_range.start+1;
	
	//check image
	// std::cout << "[Check Pre-Compressed Image]" << std::endl;
	// for (int t=0; t<(int)(time_range.end-time_range.start+1);t++) {
	//   std::cout << img.pixel( t, 1 ) << " ";
	// }
	// std::cout << std::endl;
	
	// filter: add the ability to reject images
	// bool keep = true;
	// for ( std::vector< larcaffe::supera::FilterBase* >::iterator it_filters=_filter_list.begin(); it_filters!=_filter_list.end(); it_filters++ ) {
	//   if( !(*it_filters)->doWeKeep( *db ) ) {
	//     keep = false;
	//     break;
	//   }
	// }
	// if ( !keep ) {
	//   continue;
	//}
	
	// compress image
	if ( _cropper.TargetWidth() < img.width() || _cropper.TargetHeight() < img.height() ) {
	  plane_compression[plane] = img.width()/_cropper.TargetWidth();
	  plane_compression[fNPlanes] = img.height()/_cropper.TargetHeight();
	  img.compress( _cropper.TargetHeight(), _cropper.TargetWidth(), larcaffe::Image::kMaxPool );
	}
	
	// also need to transform vertex_tw into (time,wire) coordinates of this cropped image (filled already in getMC above)
	m_vertex_tw.at(plane) = (int)(m_vertex_tw.at(plane)-wire_range.start)/plane_compression[plane];

	// std::cout << "[Check Compressed Image]" << std::endl;
	// for (int t=0; t<(int)(img.height());t++) {
	//   std::cout << img.pixel( t, 1 ) << " ";
	// }
	// std::cout << std::endl;
	
	
	// transfer image to ROOT variables
	m_planeImages[plane]->resize( img.height()*img.width() );
	for ( int w=0; w<(int)img.width(); w++) {
	  for (int t=0; t<(int)img.height(); t++) {
	    m_planeImages[plane]->at( img.height()*w + t ) = (int)img.pixel( t, w );
	  }
	}
	
	// std::cout << "[Check Plane Image]" << std::endl;
	// for (int t=0; t<(int)(img.height());t++) {
	//   std::cout << m_planeImages[plane]->at( img.height()*1+t);
	// }
	// std::cout << std::endl;	      
	
	
	_logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) <<"image " << range_index << " @ " << plane << " extracted..." << std::endl;
      }//end of loop over planes
      
      // convert time coordinate
      m_vertex_tw.at(fNPlanes) = (int)(m_vertex_tw.at(fNPlanes)-time_range.start)/plane_compression[fNPlanes];
      
    }// end of image crop ranges
    
    //
    // Determine bounding boxes
    //
    auto bboxes = findBoundingBoxes(e);
    // each interaction has bounding boxes in each plane
    auto const& the_range_v = region_v[0];
    
    int ibox = -1;
    m_nfilledboxes = 0;
    for ( auto const& range : bboxes ) {
      // range is a RangeArray_t which is a vector< pair<int,int> >
      // (x,y) = (wire, time 0toX)
      ibox++;
      
      // stay within bounds
      if ( range[fNPlanes].end<the_range_v[fNPlanes].start 
	   || range[fNPlanes].start>the_range_v[fNPlanes].end )
	continue;
      
      // calculate time bounds in cropped images coordinates
      int t_lo = (int)range[fNPlanes].start  - (int)the_range_v[fNPlanes].start;
      int t_hi = (int)range[fNPlanes].end - (int)the_range_v[fNPlanes].start;
      
      // enforce time bounds
      t_lo = std::max( t_lo, 0 );
      t_hi = std::min( t_hi, (int)the_range_v[fNPlanes].end-(int)the_range_v[fNPlanes].start );
      
      for (int plane=0; plane<fNPlanes; plane++) {

	// need to account for compression
	// bounding box goes counter clockwise from origin
	
	int w_lo = (int)range[plane].start  - (int)the_range_v[plane].start;
	int w_hi = (int)range[plane].end - (int)the_range_v[plane].start;
	
	// enforce image bounds
	w_lo = std::max( w_lo, 0 );
	w_hi = std::min( w_hi, (int)the_range_v[plane].end-(int)the_range_v[plane].start);
	
	_logger.LOG(::larcaffe::msg::kINFO,__FUNCTION__,__LINE__) 
	  << "[Plane " << plane << " BBOX]"
	  << " t=[" << (int)the_range_v[fNPlanes].start+t_lo << ", " << (int)the_range_v[fNPlanes].start+t_hi << "]"
	  << " w=[" << (int)the_range_v[plane].start+w_lo << ", " << (int)the_range_v[plane].start+w_hi << "]"
	  << " image bound: t=[" << the_range_v[fNPlanes].start << ", " << the_range_v[fNPlanes].end <<"]"
	  << " w=[" << the_range_v[plane].start << ", " << the_range_v[plane].end << "]" << std::endl;

	m_plane_bb_loleft_t[plane]->push_back( t_lo/plane_compression[fNPlanes] );
	m_plane_bb_loleft_w[plane]->push_back( w_lo/plane_compression[plane] );

	m_plane_bb_loright_t[plane]->push_back( t_lo/plane_compression[fNPlanes] );
	m_plane_bb_loright_w[plane]->push_back( w_hi/plane_compression[plane] );

	m_plane_bb_hiright_t[plane]->push_back( t_hi/plane_compression[fNPlanes] );
	m_plane_bb_hiright_w[plane]->push_back( w_hi/plane_compression[plane] );

	m_plane_bb_hileft_t[plane]->push_back( t_hi/plane_compression[fNPlanes] );
	m_plane_bb_hileft_w[plane]->push_back( w_lo/plane_compression[plane] );

	// save image for bbox
	larcaffe::Image bbimg;
	if ( fUseWire )
	  bbimg = extractor.Extract( plane, range[plane], range[fNPlanes], *wireVecHandle );
	else
	  bbimg = extractor.Extract( plane, range[plane], range[fNPlanes], *digitVecHandle );
	
	// compress image and bounding boxes
	if ( _cropper_interaction.TargetWidth() < bbimg.width() || _cropper_interaction.TargetHeight() < bbimg.height() ) {
	  bbimg.compress( _cropper_interaction.TargetHeight(), _cropper_interaction.TargetWidth(), larcaffe::Image::kMaxPool );
	}
	m_bb_nticks = bbimg.height();
	m_bb_nwires = bbimg.width();

	// transfer image to ROOT variables
	m_bb_planeImages[plane]->resize( bbimg.height()*bbimg.width() );
	for ( int w=0; w<(int)bbimg.width(); w++) {
	  for (int t=0; t<(int)bbimg.height(); t++) {
	    m_bb_planeImages[plane]->at( bbimg.height()*w + t ) = (int)bbimg.pixel( t, w );
	  }
	}

      }//end of planes loop to fill bounding boxes
      
      // save this bbox!
      m_bbox_label->push_back( m_interaction_list.at(ibox) );
      sprintf( m_bblabel, m_interaction_list.at(ibox).c_str() );
      m_BBTree->Fill();
      m_nfilledboxes++;
      
    }//end of loop over sets of bounding boxes for a given interaction
    
    // Finally fill the tree for this event
    m_ImageTree->Fill();

  }// if RawDigits producer has been specified
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
}

void Yolo::setupTrees(int nplanes) {
  // ImageTree
  // ---------
    // add the plane images
  for (int i=0; i<nplanes; i++) {
    char branchname[20];
    sprintf( branchname, "img_plane%d", i );
    m_ImageTree->Branch( branchname, &(*m_planeImages[i]) );
    m_BBTree->Branch( branchname, &(*m_bb_planeImages[i]) );
  }
  m_ImageTree->Branch( "run", &m_run, "run/I" );
  m_ImageTree->Branch( "subrun", &m_subrun, "subrun/I" );
  m_ImageTree->Branch( "event", &m_event, "event/I" );
  m_ImageTree->Branch( "Enu", &m_Enu, "Enu/F" );
  m_ImageTree->Branch( "vertex", &m_vertex );
  m_ImageTree->Branch( "vertex_tw", &m_vertex_tw );
  m_ImageTree->Branch( "mode", &m_mode, "mode/I" );
  m_ImageTree->Branch( "nuscatter", &m_nuscatter, "nuscatter/I" );
  m_ImageTree->Branch( "interaction", &m_interaction, "interaction/I" );
  m_ImageTree->Branch( "flavor", &m_flavor, "flavor/I" );
  m_ImageTree->Branch( "nticks", &m_nticks, "nticks/I" );
  char chr_mwires[20];
  sprintf( chr_mwires, "wires[%d]/I",fNPlanes );
  m_wires = new int[fNPlanes];
  m_ImageTree->Branch( "nwires", m_wires, chr_mwires );
  m_ImageTree->Branch( "label", m_label, "label[50]/C" );

  // Bounding Box Tree
  m_BBTree->Branch( "run", &m_run, "run/I" );
  m_BBTree->Branch( "subrun", &m_subrun, "subrun/I" );
  m_BBTree->Branch( "event", &m_event, "event/I" );
  m_BBTree->Branch( "ibox", &m_nfilledboxes, "ibox/I" );
  m_BBTree->Branch( "label", m_bblabel, "label[100]/C" );
  m_BBTree->Branch( "Enu", &m_Enu, "Enu/F" );
  m_BBTree->Branch( "vertex", &m_vertex );
  m_BBTree->Branch( "vertex_tw", &m_vertex_tw );

  // Bounding Box Branches: one for each plane
  // ugh, these branches are such bad code...
  m_plane_bb_loleft_t = new std::vector<int>*[nplanes];
  m_plane_bb_loleft_w = new std::vector<int>*[nplanes];
  m_plane_bb_loright_t = new std::vector<int>*[nplanes];
  m_plane_bb_loright_w = new std::vector<int>*[nplanes];
  m_plane_bb_hiright_t = new std::vector<int>*[nplanes];
  m_plane_bb_hiright_w = new std::vector<int>*[nplanes];
  m_plane_bb_hileft_t = new std::vector<int>*[nplanes];
  m_plane_bb_hileft_w = new std::vector<int>*[nplanes];
  m_plane_bbinteraction_loleft_t = new int[nplanes];
  m_plane_bbinteraction_loleft_w = new int[nplanes];
  m_plane_bbinteraction_hiright_t = new int[nplanes];
  m_plane_bbinteraction_hiright_w = new int[nplanes];
  for (int plane=0; plane<nplanes; plane++) {
    m_plane_bb_loleft_t[plane] = new std::vector<int>;
    m_plane_bb_loleft_w[plane] = new std::vector<int>;
    m_plane_bb_hileft_t[plane] = new std::vector<int>;
    m_plane_bb_hileft_w[plane] = new std::vector<int>;
    m_plane_bb_loright_t[plane] = new std::vector<int>;
    m_plane_bb_loright_w[plane] = new std::vector<int>;
    m_plane_bb_hiright_t[plane] = new std::vector<int>;
    m_plane_bb_hiright_w[plane] = new std::vector<int>;

    // set the ttree branch
    char branchname[100];

    sprintf( branchname, "LoLeft_t_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_loleft_t[plane]) );
    m_BBTree->Branch( branchname, &m_plane_bbinteraction_loleft_t[plane] );
    sprintf( branchname, "LoLeft_w_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_loleft_w[plane]) );
    m_BBTree->Branch( branchname, &m_plane_bbinteraction_loleft_w[plane] );

    sprintf( branchname, "LoRight_t_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_loright_t[plane]) );
    sprintf( branchname, "LoRight_w_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_loright_w[plane]) );

    sprintf( branchname, "HiRight_t_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_hiright_t[plane]) );
    m_BBTree->Branch( branchname, &m_plane_bbinteraction_hiright_t[plane] );
    sprintf( branchname, "HiRight_w_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_hiright_w[plane]) );
    m_BBTree->Branch( branchname, &m_plane_bbinteraction_hiright_w[plane] );

    sprintf( branchname, "HiLeft_t_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_hileft_t[plane]) );
    sprintf( branchname, "HiLeft_w_plane%d", plane );
    m_ImageTree->Branch( branchname, &(*m_plane_bb_hileft_w[plane]) );
  }
  
  m_bbox_label = new std::vector<std::string>;
  m_ImageTree->Branch( "bblabels", m_bbox_label );
}

void Yolo::clearBoundingBoxes() {
  
  m_bbox_label->clear();
  for (int plane=0; plane<fNPlanes; plane++) {
    m_plane_bb_loleft_t[plane]->clear();
    m_plane_bb_loleft_w[plane]->clear();
    m_plane_bb_loright_t[plane]->clear();
    m_plane_bb_loright_w[plane]->clear();
    m_plane_bb_hiright_t[plane]->clear();
    m_plane_bb_hiright_w[plane]->clear();
    m_plane_bb_hileft_t[plane]->clear();
    m_plane_bb_hileft_w[plane]->clear();
  }
  
}

DEFINE_ART_MODULE(Yolo)
