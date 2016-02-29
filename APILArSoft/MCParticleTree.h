#ifndef __MCPARTICLETREE_
#define __MCPARTICLETREE_

// this is to do very rudimentary particle graph work. initial aim is to do bounding box.
// these assume they own nothing

#include <map>
#include <string>
#include "MCBase/MCTrack.h"
#include "MCBase/MCShower.h"

namespace larbys {
  namespace supera {

    class MCPTInfo {
      // this is ugly.
      // a smart person would make class that inherited from common baseclass for storage and from either MCTrack or MCShower
    public:
      MCPTInfo( int trackindex, const sim::MCTrack* track ) {
	thetrack = track;
	theshower = NULL;
	datatype = kTrack;
      };
      MCPTInfo( int showerindex, const sim::MCShower* shower ) {
	thetrack = NULL;
	theshower = shower;
	datatype = kShower;	
      };
      virtual ~MCPTInfo() {};

      bool isTrack() { 
	if (datatype==kTrack) 
	  return true; 
	else return false; 
      };
      
      bool isShower() { 
	if (datatype==kShower) 
	  return true; 
	else return false; 
      };
      
      unsigned int getID() { 
	if ( isTrack() ) return thetrack->TrackID();
	else return theshower->TrackID();
      };
      
      unsigned int getAncestor() {
	if ( isTrack() ) return thetrack->AncestorTrackID();
	else return theshower->AncestorTrackID();
      };
      
      typedef enum {kTrack, kShower} DataType_t;
      DataType_t datatype;
      const sim::MCTrack* thetrack;
      const sim::MCShower* theshower;

    };

    class MCParticleTree {
    public:
      MCParticleTree( const std::vector<sim::MCTrack>& initial_trackmap, 
		      const std::vector<sim::MCShower>& initial_showermap ) {
	trackmap = &initial_trackmap;
	showermap = &initial_showermap;
      };
      virtual ~MCParticleTree() {};

      void parse();
      void boom();
      
      std::map< int, std::vector<MCPTInfo> > m_bundles;
      const std::vector<sim::MCTrack>* trackmap;
      const std::vector<sim::MCShower>* showermap;
      
    };
    
    

  }
}


#endif
