#include "MCParticleTree.h"
#include <assert.h>

namespace larbys {
  namespace supera {
    
    // ----------------------------------------------------------------
    // MCPTInfo
    // ----------------------------------------------------------------

    unsigned int MCPTInfo::getID() const {
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
    
    int MCPTInfo::getAncestorID() const {
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

    std::string MCPTInfo::getProcess() const {
      switch ( datatype ) {
      case kTrack:
	return thetrack->Process();
	break;
      case kShower:
	return theshower->Process(); // start and end
	break;
      case kParticle:
	return "primary";
	break;
      default:
	assert(false);
	break;
      }
      return "larbys";
    }

    // Step data
    void MCPTInfo::step4Pos( int istep, float vec[] ) const {
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

    // Step data
    void MCPTInfo::start4Pos( float vec[] ) const {
      switch ( datatype ) {
      case kTrack:
	if ( thetrack->size()>0 ) {
	  vec[0] = thetrack->at(0).X();
	  vec[1] = thetrack->at(0).Y();
	  vec[2] = thetrack->at(0).Z();
	  vec[3] = thetrack->at(0).T();
	}
	else {
	  vec[0] = vec[1] = vec[2] = vec[3] = 0.0;
	}
        break;
      case kShower:
	calcShowerStart( vec );
        break;
      case kParticle:
	vec[0] = theparticle->Trajectory().X(0);
	vec[1] = theparticle->Trajectory().Y(0);
	vec[2] = theparticle->Trajectory().Z(0);
	vec[3] = theparticle->Trajectory().T(0);
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

    void MCPTInfo::calcShowerStart( float vec[] ) const {
      const sim::MCStep& detprofile = theshower->DetProfile();
      vec[0] = detprofile.X();
      vec[1] = detprofile.Y();
      vec[2] = detprofile.Z();
      vec[3] = detprofile.T();
    }
    
    void MCPTInfo::calcShowerEnd( float vec[] ) const {
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
    
    void MCParticleTree::addNeutrinoInteraction( const simb::MCTruth& interaction ) {

      std::vector< MCPTInfo > interaction_out;
      for ( int iparticle=0; iparticle<interaction.NParticles(); iparticle++ ) {
	//std::cout << "MCParticleTree, adding neutrino interactoin particle: " << interaction.GetParticle(iparticle) << std::endl;
	if ( interaction.GetParticle(iparticle).StatusCode()==1 ) {
	  MCPTInfo primary( iparticle, &(interaction.GetParticle(iparticle)) );
	  interaction_out.emplace_back( primary );
	}
      }
      if ( interaction_out.size() ) {
	num_neutrinos++;
	int nu_id = -num_neutrinos;
	m_bundles.emplace( nu_id, interaction_out ).first;
	m_neutrino_interaction_particles[nu_id] = (int)interaction_out.size();
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

      // note: origin=1 means neutrino. for microboone, there is only one.  in sbnd, there might be two. but right now
      // the mctrack/mcshower is not filled correctly to separate neutrino interactions
      // so if we have origin one, we add to the neutrino group

      // MC Tracks
      for ( std::vector<sim::MCTrack>::const_iterator it_track=trackmap->begin(); it_track!=trackmap->end(); it_track++ ) {
	
	if ( (*it_track).TrackID()!=(*it_track).AncestorTrackID() ) {
	  // not a "primary", skip it
	  continue;
	}

	if ( (*it_track).Origin()==1 ) {
	  // neutrinos
	  int nuid = -1;
	  if ( m_bundles.find(nuid)==m_bundles.end() ) {
	    m_bundles.emplace( -1, std::vector<MCPTInfo>() ).first;
	  }
	  MCPTInfo ancestor( it_track-trackmap->begin(),&(*it_track) );
	  m_bundles[nuid].emplace_back( ancestor );
	}
	else {
	  // store the ancestor, make a bundle vector
	  m_bundles.emplace( (*it_track).TrackID(), std::vector<MCPTInfo>() ).first;
	  MCPTInfo ancestor( it_track-trackmap->begin(),&(*it_track) );
	  m_bundles[(*it_track).TrackID()].emplace_back( ancestor );
	}
      }
      
      // showers
      for ( std::vector<sim::MCShower>::const_iterator it_shower=showermap->begin(); it_shower!=showermap->end(); it_shower++ ) {

	if ( (*it_shower).TrackID()!=(*it_shower).AncestorTrackID() ) {
	  // not a "primary", skip it
	  continue;
	}

	if ( (*it_shower).Origin()==1 ) {
	  // neutrinos
	  int nuid = -1;
	  if ( m_bundles.find(nuid)==m_bundles.end() ) {
	    m_bundles.emplace( -1, std::vector<MCPTInfo>() ).first;
	  }
	  MCPTInfo ancestor( it_shower-showermap->begin(),&(*it_shower) );
	  m_bundles[nuid].emplace_back( ancestor );
	}
	else {
	  // store the ancestor, make a bundle vector
	  m_bundles.emplace( (*it_shower).TrackID(), std::vector<MCPTInfo>() ).first;
	  MCPTInfo ancestor( it_shower-showermap->begin(),&(*it_shower) );
	  m_bundles[(*it_shower).TrackID()].emplace_back( ancestor );
	}
      }

      // Connect Daughters to ancestors
      // -------------------------------
      
      // find all daughters: tracks
      for ( std::vector<sim::MCTrack>::const_iterator it_sakura=trackmap->begin(); it_sakura!=trackmap->end(); it_sakura++ ) {
	int daughterid = (*it_sakura).TrackID();
	int motherid = (*it_sakura).AncestorTrackID();
	if ( daughterid==motherid || (*it_sakura).Origin()==1 )
	  continue; // a parent or neutrino, which we've already stored above

	int origin = (*it_sakura).Origin();
	if (origin==1 ) {
	  // direct neutrino interaction daughter
	  matchTrackToNeutrino( *it_sakura );
	}
	else {
	  // cosmics daugher
	  MCPTInfo particle( it_sakura-trackmap->begin(),&(*it_sakura) );
	  m_bundles[motherid].emplace_back( particle );
	}
      }
	
      // find all daughters: showers
      for ( std::vector<sim::MCShower>::const_iterator it_sakura=showermap->begin(); it_sakura!=showermap->end(); it_sakura++ ) {
	int daughterid = (*it_sakura).TrackID();
	int motherid = (*it_sakura).AncestorTrackID();
	if ( daughterid==motherid || (*it_sakura).Origin()==1 )
	  continue; // a parent, which we've already stored above

	int origin = (*it_sakura).Origin();
	if (origin==1 ) {
	  // direct neutrino interaction daughter
	  matchShowerToNeutrino( *it_sakura );
	}
	else {
	  auto iter_ancestor_bundle = m_bundles.find( motherid );
	  if ( iter_ancestor_bundle==m_bundles.end() ) {
	    std::cout << "daughter could not find mother. there should be no orphans? trackid=" << daughterid << " parentid=" << motherid << std::endl;
	    assert(false);
	  }

	  MCPTInfo particle( it_sakura-showermap->begin(),&(*it_sakura) );
	  m_bundles[motherid].emplace_back( particle );
	}
      }
	
    }//end of mctrack/mcshower parser

    void MCParticleTree::matchTrackToNeutrino( const sim::MCTrack& track ) {
      for (int ineutrino=0; ineutrino<num_neutrinos; ineutrino++) {
	int nuid = -(ineutrino+1);
	if ( (int)track.AncestorTrackID()<m_neutrino_interaction_particles[nuid] ) {
	  MCPTInfo particle( 0, &track );
	  m_bundles[nuid].emplace_back( particle );
	}
      }
    }

    void MCParticleTree::matchShowerToNeutrino( const sim::MCShower& shower ) {
      for (int ineutrino=0; ineutrino<num_neutrinos; ineutrino++) {
	int nuid = -(ineutrino+1);
	if ( (int)shower.AncestorTrackID()<m_neutrino_interaction_particles[nuid] ) {
	  MCPTInfo particle( 0, &shower );
	  m_bundles[nuid].emplace_back( particle );
	}
      }
    }

    void MCParticleTree::determineVertex( float vertex[], const std::vector< MCPTInfo >& particles ) const {
      for ( std::vector< MCPTInfo >::const_iterator it_particle=particles.begin(); it_particle!=particles.end(); it_particle++ ) {
	if ( (int)(*it_particle).getID()==(int)(*it_particle).getAncestorID() ) {
	  (*it_particle).start4Pos( vertex );
	  break;
	}
      }
    }
    
    void MCParticleTree::boom() {
      std::cout << "[PARTICLE TREE: full tree]" << std::endl;
      for ( std::map< int, std::vector<MCPTInfo> >::iterator it=m_bundles.begin(); it!=m_bundles.end(); it++ ) {
	std::cout << "ANCESTOR TRACKID=" << (*it).first << std::endl;
	for ( std::vector<MCPTInfo>::iterator it_sakura=(*it).second.begin(); it_sakura!=(*it).second.end(); it_sakura++) {
	  std::cout << "   ";
	  if ( (*it_sakura).isTrack() ) {
	    std::cout << "TrackID " << (*it_sakura).thetrack->TrackID() << " pdg=" << (*it_sakura).thetrack->PdgCode() 
		      << " origin=" << (*it_sakura).thetrack->Origin()
		      << " ancestor=" << (*it_sakura).thetrack->AncestorTrackID() << " " << (*it_sakura).thetrack->Process();
	  }
	  else if ( (*it_sakura).isShower() ){
	    std::cout << "ShowerID: " << (*it_sakura).theshower->TrackID() << " pdg=" << (*it_sakura).theshower->PdgCode() 
		      << " origin=" << (*it_sakura).theshower->Origin()
		      << " ancestor=" << (*it_sakura).theshower->AncestorTrackID() << " " << (*it_sakura).theshower->Process();
	  }
	  else if ( (*it_sakura).isMCParticle() ) {
	    std::cout << "ParticleID: " << (*it_sakura).theparticle->TrackId() 
		      << " pdg=" << (*it_sakura).theparticle->PdgCode()
		      << " status=" << (*it_sakura).theparticle->StatusCode();
	      }
	  std::cout << std::endl;
	}//end of daughter loop
      }
    }
    
  }
}

