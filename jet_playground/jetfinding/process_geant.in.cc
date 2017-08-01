// code to perform jetfinding on the selected pythia&geant
// data, to extract relevant features that may be useful for
// training models, such as pt, eta, phi, constituent count, etc
// also extracts event information like refmult, etc

#include "event.hh"

#include <string>
#include <vector>

#include "TFile.h"

/** used to create the full path + name of output files */
std::string create_file_name(const std::string& algorithm, double resolution, bool inclusive,
                      bool charged );

/** the grid does not have std::to_string() for some ungodly reason
    replacing it here. Simply ostringstream
 */
namespace patch {
  template < typename T > std::string to_string( const T& n );
}

int main ( int argc, const char** argv ) {
  
  /**  Command line arguments
       1: jet algorithm ( antikt, kt, CA )
       2: resolution parameter
       3: inclusive or leading ( decides if we only save leading jet or all jets
       4: charged or full ( if we're looking at charged jets or full jets )
       5: path to the settings file for the reader
       6: path to the data the reader will use ( .root, .list, .txt)
   */
  
  std::string algorithm = "antikt";
  double resolution     = 0.4;
  bool inclusive_jets   = false;
  bool charged_jets     = false;
  std::string settings  = "${CMAKE_BINARY_DIR}/settings/reader.txt";
  std::string data      = "${CMAKE_SOURCE_DIR}/test_data/picoDst_25_35_0.root";
  
  switch ( argc ) {
    case 7 :
      algorithm = argv[1];
      resolution = std::stof( std::string(argv[2]) );
      
      if      ( std::string( argv[3] ) == "true"   ) inclusive_jets = true;
      else if ( std::string( argv[3] ) == "false"  ) inclusive_jets = false;
      else { std::cerr << "Error unrecognized argument for inclusive jets ( true or false ) " << std::endl;
             return -1; }
      
      if      ( std::string( argv[4] ) == "true"   ) charged_jets = true;
      else if ( std::string( argv[4] ) == "false"  ) charged_jets = false;
      else { std::cerr << "Error unrecognized argument for charged jets ( true or false ) " << std::endl;
        return -1; }
      
      settings = argv[5];
      data     = argv[6];
      
      break;
    case 1 :
      /** use defaults */
      break;
    default :
      /** save me from myself making stupid mistakes - exit */
      std::cerr << "Error: mismatch in number of command line arguments" << std::endl;
      return -1;
  }
  
  std::cout<<"algorithm: "<< algorithm << " R: "<<resolution <<std::endl;
  std::cout<<"inclusive jets: "<< inclusive_jets<<std::endl;
  std::cout<<"charged jets: "<< charged_jets<<std::endl;
  std::cout<<"settings: "<<settings<<std::endl;
  std::cout<<"data: "<<data<<std::endl;
  
  /** set some constants that won't need to be changed -
      kinemantic cuts for fastjet: jet pt cut,
      constituent pt cut, and eta acceptance, and settings
      for the area definition
   */
  
  /** eta acceptance ( STAR-like, |eta| < 1.0 ) */
  const double eta_acceptance_max   = 1.0;
  
  /** these are used to set up the active ghost area specification
      used to calculate jet areas. See fastjet manual for explanations,
      but its pretty self explanatory - ghosts should extend past the 
      acceptance by at least R, to keep area calculations robust
   */
  const double ghost_rap_max        = eta_acceptance_max + resolution;
  const int ghost_repeat            = 1;
  const double ghost_area           = 0.01;
  
  /** kinematic limits for jet and their constituents.
      constituents: |eta| < 1.0 ( STAR acceptance ), 
                     0.2 < | pt | < 30 GeV ( normal cuts for our analyses )
      jets:         |eta| < 1.0 - R ( normal, avoid boundary effects ) 
                     1.0 < | pt | < 100 GeV ( arbitrary, shouldn't hit upper bound )
   */
  const double const_eta_max = eta_acceptance_max;
  const double const_pt_min = 0.2;
  const double const_pt_max = 30.0;
  const double jet_eta_max = eta_acceptance_max - resolution;
  const double jet_pt_min = 1.0;
  const double jet_pt_max = 100.0;
  
  /** setup Fastjet - first, jet definition
      the clustering algorithm that is used to decide
      which pairs of pseudojets are combined at which step
      the three choices are all sequential recombination
      with different weightings by pT of the tracks - see
      FastJet manual online
   */
  fastjet::JetDefinition jet_def;
  if ( algorithm == "antikt" )
    jet_def = fastjet::JetDefinition( fastjet::antikt_algorithm, resolution );
  else if ( algorithm == "kt" )
    jet_def = fastjet::JetDefinition( fastjet::kt_algorithm, resolution );
  else if ( algorithm == "CA" )
    jet_def = fastjet::JetDefinition( fastjet::cambridge_algorithm, resolution );
  else { std::cerr << "unrecognized jet algorithm, exiting" << std::endl; return -1; }
  
  /** create a default area definition
   used to estimate the area of the jet cone
   using a large number of soft "ghosts"
   the fraction of ghosts in the clustered jet / total area = jet area
   */
  fastjet::GhostedAreaSpec ghost_area_spec( ghost_rap_max, ghost_repeat, ghost_area );
  fastjet::AreaDefinition area_def(  fastjet::active_area_explicit_ghosts, ghost_area_spec );
  
  /** track / jet selectors
   applied before clustering to tracks to select "good" tracks
   jet selector is applied to the clustered inclusive jets to
   select jets in our acceptance within a reasonable pt range
   */
  fastjet::Selector jet_selector    = fastjet::SelectorPtMin( jet_pt_min )   * fastjet::SelectorPtMax( jet_pt_max )   * fastjet::SelectorAbsRapMax( jet_eta_max );
  fastjet::Selector track_selector  = fastjet::SelectorPtMin( const_pt_min ) * fastjet::SelectorPtMax( const_pt_max ) * fastjet::SelectorAbsRapMax( const_eta_max );
  
  /** setup reader - its using the options from the default reader settings file
      to initialize the chain & event cuts
   */
  event event( data, settings );
  event.init_tree();
  
  /** loop over events */
  while ( event.next() ) {
    event.process_event( jet_def, area_def, track_selector, jet_selector, inclusive_jets, charged_jets  );
    event.fill_trees();
  }
  
  /** now build the paths for output */
  std::string output_name = create_file_name( algorithm, resolution, inclusive_jets,
                                             charged_jets  );
  TFile out( output_name.c_str(), "RECREATE" );
  
  event.write_tree();
  
  out.Close();
  
  
  return 0;
}

/** used to create the full path + name of output files */
std::string create_file_name( const std::string& algorithm, double resolution, bool inclusive,
                       bool charged ) {
  
  std::string base_dir = "${CMAKE_BINARY_DIR}/training/";
  
  return base_dir + algorithm + "_R_" + patch::to_string(resolution) + "_inc_" + patch::to_string(inclusive) + "_charged_" + patch::to_string(charged) + ".root";
  
  
}

/** the grid does not have std::to_string() for some ungodly reason
    replacing it here. Simply ostringstream
 */
namespace patch {
  template < typename T > std::string to_string( const T& n ) {
  
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
  }
}
