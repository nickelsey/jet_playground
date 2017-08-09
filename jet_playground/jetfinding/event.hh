// Nick Elsey
// 06 - 18 - 17

/**  A wrapper for the event info I will
     be saving to use in the neural network
 */

#include "geant_reader.hh"

#include "TTree.h"
#include "TBranch.h"

#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/Selector.hh"

#include <string>

#ifndef EVENT_HH
#define EVENT_HH

class event : public geant_reader {

public:
  
  /** the event takes 5 arguments for construction,
      everything else is set via a call to init.
      train_tree & label_tree are where the output ( training data
      for my models ) are stored in TTree format
      naive_tree is an optional output where a detector
      tracking efficiency is used for comparison.
      The last two are used to setup the input options 
      & TChain in the reader
   */
  event( const std::string& input_file = "", const std::string& settings_doc = "" );
  
  /** destructor */
  ~event();
  
  /** initialization function that initializes both the
   reader & the TTree used to store output
   */
  void init_tree();
  
  /** processes the geant & pythia data to produce a list of candidate jets
      constituent_selector is applied to the constituents before clustering,
      jet_selector is applied to the jets after clustering
   */
  bool process_event( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                    const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                    bool inclusive_jets, bool charged_jets );
  
  /** performs the clustering and matching for full jets ( charged and neutral) */
  bool process_full( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                bool inclusive_jets );
  
  /** same methodology but uses only charged constituents */
  bool process_charged( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                       const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                       bool inclusive_jets );
  
  /** call to propogate current event to the tree. If its not
      called, no information from the current event will be 
      saved.
   */
  void fill_trees();
  
  /** write the tree to current ROOT directory/file */
  void write_tree();
  
  /**  get for TTree, this is implemented so that
       branches can be added by the user, and doesn't 
       require a rewrite of the implementation.
   */
  TTree* get_train_tree()                       { return train_data_; }
  
  
private:
  
  /** the tree for recording the training data
      including event & lead jet information (where the jet 
      has been matched to the leading jet from pythia ). 
      its possible that the tree is not owned by the event class,
      in which case we don't delete or write it
   */
  TTree* train_data_;
  bool own_training;
  
  
  /** holders for the clustered jets so they can be manipulated 
      before written to file 
   */
  std::vector<fastjet::PseudoJet> geant_jets_;
  std::vector<fastjet::PseudoJet> pythia_jets_;
  
  /** function used to match jets with a radial distance metric
      matches highest energy jets with R < resolution
      discards jets that are not matched
  */
  void match_jets( double radius );
  
  /** variables to generate the branches for the ttree
      including jet level & event level information,
      as well as the truth labels
   */
  TClonesArray geant_array_, pythia_array_;
  double train_eta_, train_phi_, train_pt_, train_area_, train_npart_, train_charge_frac_, train_weight;
  double label_eta_, label_phi_, label_pt_;
  
  /** used to fill jets & event info to the ttrees
   can either write all jets ( inclusive ) or
   writes the leading jet
   */
  void fill_tree( const fastjet::Selector& constituent_selector );
  void fill_tree_inclusive( const fastjet::Selector& constituent_selector );
  
};




//______________________________________________________________
/** helper Selector to select on user supplied user_info */
class SelectorUserIndexWorker : public fastjet::SelectorWorker {
public:
  
  SelectorUserIndexWorker( const std::vector<int>& idx ) : user_idx( idx ) {};
  
  /// the selector's description
  std::string description() const{
    std::ostringstream oss;
    oss.str("");
    oss << " user index = ";
    
    return oss.str() ;
  };

  bool pass( const fastjet::PseudoJet& p ) const {
    return ( int ( user_idx.size() ) > std::find( user_idx.begin(), user_idx.end(), p.user_index() ) - user_idx.begin() );
  }
  
private:
  const std::vector<int> user_idx;
  
  
};

/** the selector built from the above worker */
fastjet::Selector SelectorUserIndex( const std::vector<int> usr_idx );

//________________________________________________________________
/** class that allows a pseudojet to be written directly to a TClonesArray */
class TPseudoJet : public fastjet::PseudoJet, public TObject {
public:
  
  TPseudoJet();
  TPseudoJet( const fastjet::PseudoJet& ps );
  
};


#endif
