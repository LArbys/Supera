#include "MCParticleTree.h"

namespace larbys {
  namespace supera {

    void MCParticleTree::parse() {
      // we use the trackmap and showermap and make a bundle
      // we loop through and find ancestors

      // this is the dumbest loop possible

      for ( std::vector<sim::MCTrack>::const_iterator it_track=trackmap->begin(); it_track!=trackmap->end(); it_track++ ) {

	if ( (*it_track).TrackID()!=(*it_track).AncestorTrackID() ) {
	  // not a "primary", skip it
	  continue;
	}

	// store the ancestor, make a bundle vector
	m_bundles.emplace( (*it_track).TrackID(), std::vector<MCPTInfo>() ).first;
	MCPTInfo ancestor( it_track-trackmap->begin(),&(*it_track) );
	m_bundles[(*it_track).TrackID()].emplace_back( ancestor );
	
	// find all daughters: tracks
	for ( std::vector<sim::MCTrack>::const_iterator it_sakura=trackmap->begin(); it_sakura!=trackmap->end(); it_sakura++ ) {
	  if ( (*it_sakura).AncestorTrackID()==(*it_track).TrackID() && (*it_sakura).TrackID()!=(*it_track).TrackID() ) {
	    MCPTInfo particle( it_track-trackmap->begin(),&(*it_sakura) );
	    m_bundles[(*it_track).TrackID()].emplace_back( particle );
	  }
	}

	// find all daughters: showers
	for ( std::vector<sim::MCShower>::const_iterator it_sakura=showermap->begin(); it_sakura!=showermap->end(); it_sakura++ ) {
	  if ( (*it_sakura).AncestorTrackID()==(*it_track).TrackID() && (*it_sakura).TrackID()!=(*it_track).TrackID() ) {
	    MCPTInfo particle( it_track-trackmap->begin(),&(*it_sakura) );
	    m_bundles[(*it_track).TrackID()].emplace_back( particle );
	  }
	}
	
      }//trackmap iterator
    }//end of parser

    void MCParticleTree::boom() {
      std::cout << "[PARTICLE TREE: full tree]" << std::endl;
      for ( std::map< int, std::vector<MCPTInfo> >::iterator it=m_bundles.begin(); it!=m_bundles.end(); it++ ) {
	std::cout << "ANCESTOR TRACKID=" << (*it).first << std::endl;
	for ( std::vector<MCPTInfo>::iterator it_sakura=(*it).second.begin(); it_sakura!=(*it).second.end(); it_sakura++) {
	  std::cout << "   ";
	  if ( (*it_sakura).isTrack() ) {
	    std::cout << "TrackID " << (*it_sakura).thetrack->TrackID() << " pdg=" << (*it_sakura).thetrack->PdgCode() 
		      << " ancestor=" << (*it_sakura).thetrack->AncestorTrackID() << " " << (*it_sakura).thetrack->Process();
	  }
	  else {
	    std::cout << "ShowerID: " << (*it_sakura).theshower->TrackID() << " pdg=" << (*it_sakura).theshower->PdgCode() 
		      << " ancestor=" << (*it_sakura).theshower->AncestorTrackID() << " " << (*it_sakura).theshower->Process();
	  }
	  std::cout << std::endl;
	}
      }
    }
    
  }
}

