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

#include "Geometry/Geometry.h"
#include "RawData/RawDigit.h"
#include "MCBase/MCTrack.h"

#include "db_lmdb.h"
#include "caffe.pb.h"
#include <cstring>

#include "RawDigitsToImageI.h"
#include "Cropper.h"

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
  larcaffe::RawDigitsToImageI converter;
  larcaffe::Cropper cropper;

  // Required functions.
  void analyze(art::Event const & e) override;

  virtual void endJob();

private:

  // Declare member data here.
  db::LMDB* lmdb_;
  db::Transaction* txn_;
  int nfills_before_write;

  // FCL Parameters
  bool fCosmicsMode; // activates cosmics mode in the cropper -- crop around each track

};


Supera::Supera(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)  // ,
 // More initializers here.
{
  
  std::string dbname = p.get<std::string>("DatabaseName","output_supera.mdb");
  fCosmicsMode       = p.get<bool>("CosmicsMode",false); 
  
  // open the database
  std::cout << "[Supera] Make Database: " << dbname << std::endl;
  lmdb_ = new db::LMDB();
  lmdb_->Open( dbname, db::NEW );
  std::cout << "[Supera] Create transaction. " << std::endl;
  txn_ = lmdb_->NewTransaction();
  nfills_before_write = 0;

}

void Supera::analyze(art::Event const & e)
{
  // Implementation of required member function here.
  std::cout << "[Supera] Load RawDigits Handle" << std::endl;
  art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
  e.getByLabel("daq", digitVecHandle);

  art::Handle< std::vector<sim::MCTrack> > mctrackHandle;
  e.getByLabel( "mcreco", mctrackHandle );

  if ( !digitVecHandle.isValid() ) {
    std::cout << "Missing daq info. skipping." << std::endl;
    return;
  }

  // geometry service
  std::cout << "[Supera] Load Geometry Service" << std::endl;
  art::ServiceHandle<geo::Geometry> geom;

  // // data size
  //const unsigned int nticks = 4800; // 2.4 ms at 2 MHz
  //const unsigned int first_tick = 3200; // 1.6 ms from start of tpc acquisition
  const unsigned int nwfms = 3456;  // collection plane size
  const unsigned int first_col_wire = digitVecHandle->size() - nwfms; // a guess: fix this using geo service
  //float* image = new float[nticks*nwfms];
  //memset( image, 0, sizeof(float)*(nticks*nwfms) );
  //std::cout << "[Supera] image array ready " << image[0] << std::endl;

  larcaffe::Image image;
  converter.convert( *digitVecHandle, first_col_wire, first_col_wire+nwfms, image );


  std::cout << "[Supera] Call the cropper" << std::endl;
  cropper.fCosmicMode = fCosmicsMode;
  std::vector< larcaffe::MCImage > cropped_images;
  cropper.crop( *mctrackHandle, image, cropped_images );

  // Caffe Protobuf object which we will serialize and store
  for (int img=0; img<(int)cropped_images.size(); img++) {
    caffe::Datum data;
    //larcaffe::Image& cropped = cropped_images.at(img).precompressed_collection;
    larcaffe::Image& cropped = cropped_images.at(img).compressed_collection;
    std::cout << "[Supera] Processed cropped image " << &cropped << std::endl;
    data.set_channels( 1 );          // number of planes (only collection for now)
    data.set_height( (int)cropped.height() );  // number of ticks
    data.set_width(  (int)cropped.width() );    // number of wires
    data.set_label( 0 );             // set label: how is this done?  


    // copy into Datum
    std::cout << "[Supera] Copy into protobuf. H: " <<  cropped.height() << " W:" << cropped.width() << std::endl;
    for (int t=0; t<cropped.height(); t++) {
      for (int ch=0; ch<cropped.width(); ch++) {
	data.add_float_data( cropped.pixel(t,ch) );
      }
    }
    
    // now serialize and store
    std::cout << "[Supera] Serialize " << std::endl;
    std::string out;
    data.SerializeToString(&out);
    char eventid[200];
    sprintf( eventid, "run%06d_subrun%04d_event%06d_img%dof%d", e.run(), e.subRun(), e.event(), img,(int)cropped_images.size());
    std::string key_str = eventid;
    std::cout << "[Supera] Store in DB " << std::endl;
    txn_->Put(key_str, out);
    nfills_before_write++;
    if ( nfills_before_write==100 ) {
      txn_->Commit();
      delete txn_;
      txn_ = lmdb_->NewTransaction();
    }
  }//for total number of crops
  
  std::cout << "[Supera] analyze end." << std::endl;
}


void Supera::endJob() {
  txn_->Commit();
}


DEFINE_ART_MODULE(Supera)
