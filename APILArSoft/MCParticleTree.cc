#include "MCParticleTree.h"
#include <assert.h>

namespace larbys {
  namespace supera {
    
    // ----------------------------------------------------------------
    // MCPTInfo
    // ----------------------------------------------------------------

    unsigned int MCPTInfo::getID() {
      switch ( datatype ) {
      case kTrack:
	return thetrack->TrackID();
	break;
      case kShower:
	return theshower->TrackID();
	break;
      case kParticle:
	return theparticle->TrackId();
	break;
      default:
	assert(false);
	break;
      }
      return 0;
    }
    
    int MCPTInfo::getAncestorID() {
      switch ( datatype ) {
      case kTrack:
	return thetrack->AncestorTrackID();
	break;
      case kShower:
	return theshower->AncestorTrackID();
	break;
      case kParticle:
	return -1;
	break;
      default:
	assert(false);
	break;
      }
      return 0;
    }
    
    int MCPTInfo::getPDG() {
      switch ( datatype ) {
      case kTrack:
	return thetrack->PdgCode();
	break;
      case kShower:
	return theshower->PdgCode();
	break;
      case kParticle:
	return theparticle->PdgCode();
	break;
      default:
	assert(false);
	break;
      }
      return 0;
    }

    unsigned int MCPTInfo::getNumSteps() {
      switch ( datatype ) {
      case kTrack:
	return thetrack->size();
	break;
      case kShower:
	return 2; // start and end
	break;
      case kParticle:
	return theparticle->NumberTrajectoryPoints();
	break;
      default:
	assert(false);
	break;
      }
      return 0;
    }

    // Step data
    void MCPTInfo::step4Pos( int istep, float vec[] ) {
      switch ( datatype ) {
      case kTrack:
	vec[0] = thetrack->at(istep).X();
	vec[1] = thetrack->at(istep).Y();
	vec[2] = thetrack->at(istep).Z();
	vec[3] = thetrack->at(istep).T();
        break;
      case kShower:
	if ( istep==0 ) {
	  calcShowerStart( vec );
	}
	else if ( istep==1 ) {
	  calcShowerEnd( vec );
	}
        break;
      case kParticle:
	vec[0] = theparticle->Trajectory().X(istep);
	vec[1] = theparticle->Trajectory().Y(istep);
	vec[2] = theparticle->Trajectory().Z(istep);
	vec[3] = theparticle->Trajectory().T(istep);
        break;
      default:
        assert(false);
        break;
      }
    }
    
    void MCPTInfo::step4Mom( int istep, float vec[] ) {
      switch ( datatype ) {
      case kTrack:
	vec[0] = thetrack->at(istep).Px();
	vec[1] = thetrack->at(istep).Py();
	vec[2] = thetrack->at(istep).Pz();
	vec[3] = thetrack->at(istep).E();
        break;
      case kShower:
	vec[0] = theshower->DetProfile().Px();
	vec[1] = theshower->DetProfile().Py();
	vec[2] = theshower->DetProfile().Pz();
	vec[3] = theshower->DetProfile().E();
        break;
      case kParticle:
	vec[0] = theparticle->Trajectory().Px(istep);
	vec[1] = theparticle->Trajectory().Py(istep);
	vec[2] = theparticle->Trajectory().Pz(istep);
	vec[3] = theparticle->Trajectory().E(istep);
        break;
      default:
        assert(false);
        break;
      }
    }

    void MCPTInfo::calcShowerStart( float vec[] ) {
      const sim::MCStep& detprofile = theshower->DetProfile();
      vec[0] = detprofile.X();
      vec[1] = detprofile.Y();
      vec[2] = detprofile.Z();
      vec[3] = detprofile.T();
    }
    
    void MCPTInfo::calcShowerEnd( float vec[] ) {
      const sim::MCStep& detprofile = theshower->DetProfile();
      double energy = detprofile.E();
      if ( energy<1 )
	return calcShowerStart( vec );
      double showerlength = 100.0;
      double detprofnorm = sqrt( detprofile.Px()*detprofile.Px() + detprofile.Py()*detprofile.Py() + detprofile.Pz()*detprofile.Pz() );
      vec[0] = detprofile.X()+showerlength*(detprofile.Px()/detprofnorm);
      vec[1] = detprofile.Y()+showerlength*(detprofile.Py()/detprofnorm);
      vec[2] = detprofile.Z()+showerlength*(detprofile.Pz()/detprofnorm);
      vec[3] = detprofile.T();
    }

    // ----------------------------------------------------------------
    // MCParticleTree
    // ----------------------------------------------------------------
    
    void MCParticleTree::addNeutrino( const std::vector<simb::MCParticle>& particlemap ) {
      num_neutrinos++;
      int nu_id = -num_neutrinos;
      m_bundles.emplace( nu_id, std::vector<MCPTInfo>() ).first;
      for ( std::vector<simb::MCParticle>::const_iterator it_particle=particlemap.begin(); it_particle!=particlemap.end(); it_particle++ ) {
	MCPTInfo primary( it_particle-particlemap.begin(), &(*it_particle) );
	m_bundles[nu_id].emplace_back( primary );
      }      
    }
    
    void MCParticleTree::parse() {
      // we use the trackmap and showermap and make a bundle
      // we loop through and find ancestors

      // this is the dumbest loop possible

      // ------------------
      // determine sources
      // ------------------

      // mc particles: we store this in one bundle
      // in the future, this needs to handle multiple neutrinos

      // MC Tracks
      for ( std::vector<sim::MCTrack>::const_iterator it_track=trackmap->begin(); it_track!=trackmap->end(); it_track++ ) {
	
	if ( (*it_track).TrackID()!=(*it_track).AncestorTrackID() ) {
	  // not a "primary", skip it
	  continue;
	}
	
	// store the ancestor, make a bundle vector
	m_bundles.emplace( (*it_track).TrackID(), std::vector<MCPTInfo>() ).first;
	MCPTInfo ancestor( it_track-trackmap->begin(),&(*it_track) );
	m_bundles[(*it_track).TrackID()].emplace_back( ancestor );
      }
      
      // showers
      for ( std::vector<sim::MCShower>::const_iterator it_shower=showermap->begin(); it_shower!=showermap->end(); it_shower++ ) {

	if ( (*it_shower).TrackID()!=(*it_shower).AncestorTrackID() ) {
	  // not a "primary", skip it
	  continue;
	}

	// store the ancestor, make a bundle vector
	m_bundles.emplace( (*it_shower).TrackID(), std::vector<MCPTInfo>() ).first;
	MCPTInfo ancestor( it_shower-showermap->begin(),&(*it_shower) );
	m_bundles[(*it_shower).TrackID()].emplace_back( ancestor );
      }

      // Connect Daughters to ancestors
      // -------------------------------
      
      // find all daughters: tracks
      for ( std::vector<sim::MCTrack>::const_iterator it_sakura=trackmap->begin(); it_sakura!=trackmap->end(); it_sakura++ ) {
	int daughterid = (*it_sakura).TrackID();
	int motherid = (*it_sakura).AncestorTrackID();
	if ( daughterid==motherid )
	  continue; // a parent, which we've already stored above

	auto iter_ancestor_bundle = m_bundles.find( motherid );
	if ( iter_ancestor_bundle==m_bundles.end() ) {
	  std::cout << "daughter could not find mother. there should be no orphans? trackid=" << daughterid << " parentid=" << motherid << std::endl;
	  assert(false);
	}
	MCPTInfo particle( it_sakura-trackmap->begin(),&(*it_sakura) );
	m_bundles[motherid].emplace_back( particle );
      }
	
      // find all daughters: showers
      for ( std::vector<sim::MCShower>::const_iterator it_sakura=showermap->begin(); it_sakura!=showermap->end(); it_sakura++ ) {
	int daughterid = (*it_sakura).TrackID();
	int motherid = (*it_sakura).AncestorTrackID();
	if ( daughterid==motherid )
	  continue; // a parent, which we've already stored above

	auto iter_ancestor_bundle = m_bundles.find( motherid );
	if ( iter_ancestor_bundle==m_bundles.end() ) {
	  std::cout << "daughter could not find mother. there should be no orphans? trackid=" << daughterid << " parentid=" << motherid << std::endl;
	  assert(false);
	}
	MCPTInfo particle( it_sakura-showermap->begin(),&(*it_sakura) );
	m_bundles[motherid].emplace_back( particle );
      }
	
    }//end of mctrack/mcshower parser
    
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
	  else if ( (*it_sakura).isShower() ){
	    std::cout << "ShowerID: " << (*it_sakura).theshower->TrackID() << " pdg=" << (*it_sakura).theshower->PdgCode() 
			<< " ancestor=" << (*it_sakura).theshower->AncestorTrackID() << " " << (*it_sakura).theshower->Process();
	  }
	  else if ( (*it_sakura).isMCParticle() ) {
	    std::cout << "ParticleID: " << (*it_sakura).theparticle->TrackId() << " pdg=" << (*it_sakura).theparticle->PdgCode();
	  }
	  std::cout << std::endl;
	}//end of daughter loop
      }
    }
    
  }
}

