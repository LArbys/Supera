#ifndef __MCPARTICLETREE_
#define __MCPARTICLETREE_

// this is to do very rudimentary particle graph work. initial aim is to do bounding box.
// these assume they own nothing

#include <map>
#include <string>
#include "MCBase/MCTrack.h"
#include "MCBase/MCShower.h"
#include "SimulationBase/MCTruth.h"

namespace larbys {
  namespace supera {

    class MCPTInfo {
      // this is ugly.
      // a smart person would make class that inherited from common baseclass for storage and from either MCTrack or MCShower
    public:

      typedef enum {kTrack, kShower, kParticle} DataType_t;

      MCPTInfo( int trackindex, const sim::MCTrack* track ) {
	index = trackindex;
	thetrack = track;
	theshower = NULL;
	theparticle = NULL;
	datatype = kTrack;
      };
      MCPTInfo( int showerindex, const sim::MCShower* shower ) {
	index = showerindex;
	thetrack = NULL;
	theshower = shower;
	theparticle = NULL;
	datatype = kShower;	
      };
      MCPTInfo( int partindex, const simb::MCParticle* particle ) {
	index = partindex;
	thetrack = NULL;
	theshower = NULL;
	theparticle = particle;
	datatype = kParticle;	
      };
      virtual ~MCPTInfo() {};

      bool isTrack() const { 
	if (datatype==kTrack) 
	  return true; 
	else return false; 
      };
      
      bool isShower() const { 
	if (datatype==kShower) 
	  return true; 
	else return false; 
      };

      bool isMCParticle() const { 
	if ( datatype==kParticle ) return true; else return false; 
      };
      
      unsigned int getID() const;
      int getAncestorID() const;
      int getPDG() const;
      unsigned int getNumSteps();
      std::string getProcess() const;
      void step4Pos( int istep, float vec[] ) const;
      void step4Mom( int istep, float vec[] );
      void start4Pos( float vec[] ) const;
      void calcShowerStart( float vec[] ) const;
      void calcShowerEnd( float vec[] ) const;
      
      DataType_t datatype;
      const sim::MCTrack* thetrack;
      const sim::MCShower* theshower;
      const simb::MCParticle* theparticle;
      int index;

    };
    
    class MCParticleTree {

    public:
      MCParticleTree( const std::vector<sim::MCTrack>& initial_trackmap, 
		      const std::vector<sim::MCShower>& initial_showermap ) {
	trackmap = &initial_trackmap;
	showermap = &initial_showermap;
	num_neutrinos = 0;
      };
      virtual ~MCParticleTree() {};
      
      void addNeutrinoInteraction( const simb::MCTruth& neutrino_interaction );
      void parse(); //< parse list of tracks and showers
      void boom();  //< dump the batches
      void matchTrackToNeutrino( const sim::MCTrack& track );
      void matchShowerToNeutrino( const sim::MCShower& shower );
      void determineVertex( float vertex[], const std::vector< MCPTInfo >& particles ) const;
      
      int num_neutrinos;
      std::map< int, std::vector<MCPTInfo> > m_bundles;
      std::map< int, int > m_neutrino_interaction_particles; // number of interaction particles during initial fill (not counting attached daughters)
      const std::vector<sim::MCTrack>* trackmap;
      const std::vector<sim::MCShower>* showermap;
      
    };
    
  }
}


#endif
