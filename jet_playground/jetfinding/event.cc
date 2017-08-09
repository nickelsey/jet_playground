
#include "event.hh"

#include "fastjet/ClusterSequenceArea.hh"

#include <exception>

event::event( const std::string& input_file,
              const std::string& settings_doc ) :
              geant_reader( settings_doc, input_file ) {}

event::~event() {
  
  if ( train_data_ ) delete train_data_;

}

bool event::process_event( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                          const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                          bool inclusive_jets, bool charged_jets ) {
  /** selects if we use the charged or full jet reconstruction */
  if ( charged_jets ) return process_charged( jet_def, area_def, constituent_selector,
                                              jet_selector, inclusive_jets );
  else                return process_full( jet_def, area_def, constituent_selector,
                                             jet_selector, inclusive_jets );
}

bool event::process_full( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                     const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                     bool inclusive_jets ) {
  
  /* clear any jets from the last event
     for cleanliness */
  geant_jets_.clear();
  pythia_jets_.clear();
  
  std::vector<fastjet::PseudoJet> geant_constituents = constituent_selector( geant_pseudojets() );
  std::vector<fastjet::PseudoJet> pythia_constituents = constituent_selector( pythia_pseudojets() );
  
  fastjet::ClusterSequenceArea cluster_geant( geant_constituents, jet_def, area_def );
  fastjet::ClusterSequenceArea cluster_pythia( pythia_constituents, jet_def, area_def );
  
  geant_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_geant.inclusive_jets() ) );
  pythia_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_pythia.inclusive_jets() ) );
  
  match_jets( jet_def.R() );
  
  if ( inclusive_jets ) fill_tree_inclusive( constituent_selector );
  else fill_tree( constituent_selector );
  
  return true;
}

bool event::process_charged( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                            const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                            bool inclusive_jets ) {
  
  /* clear any jets from the last event
   for cleanliness */
  geant_jets_.clear();
  pythia_jets_.clear();
  
  // create a selector that selects all tracks with user index ( charge ) of -1, 1
  fastjet::Selector charge_selector = SelectorUserIndex( std::vector<int> { -2, -1, 1, 2 } );
  
  std::vector<fastjet::PseudoJet> geant_constituents = charge_selector( constituent_selector( geant_pseudojets() ) );
  std::vector<fastjet::PseudoJet> pythia_constituents = charge_selector( constituent_selector( pythia_pseudojets() ) );
   
  fastjet::ClusterSequenceArea cluster_geant( geant_constituents, jet_def, area_def );
  fastjet::ClusterSequenceArea cluster_pythia( pythia_constituents, jet_def, area_def );
  
  geant_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_geant.inclusive_jets() ) );
  pythia_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_pythia.inclusive_jets() ) );
  
  match_jets( jet_def.R() );
  
  if ( inclusive_jets ) fill_tree_inclusive( constituent_selector );
  else fill_tree( constituent_selector );
  
  return true;
}

void event::init_tree() {
  
  // initialize the reader
  init();
  
  // and initialize the trees with default branches
  train_data_          = new TTree( "training", "training data" );
  
  // event information
  
  
  b_train_pt_          = train_data_->Branch("pt", &train_pt_ );
  b_train_phi_         = train_data_->Branch("phi", &train_phi_ );
  b_train_eta_         = train_data_->Branch("eta", &train_eta_ );
  b_train_area_        = train_data_->Branch("area", &train_area_ );
  b_train_npart_       = train_data_->Branch("ncharge", &train_npart_ );
  b_train_charge_frac_ = train_data_->Branch("charge_frac", &train_charge_frac_ );
  b_train_weight       = train_data_->Branch("weight", &train_weight );
  
  b_label_pt_          = train_data_->Branch( "reco_pt", &label_pt_ );
  b_label_phi_         = train_data_->Branch( "reco_phi", &label_phi_ );
  b_label_eta_         = train_data_->Branch( "reco_eta", &label_eta_ );
  
}

void event::fill_trees() {
  if ( train_data_ == nullptr ) { std::cerr << "ERROR: event::init_trees() must be called before trees can be filled" << std::endl; throw std::exception();}
 
  train_data_->Fill();
}


void event::write_tree() {
  if ( train_data_ == nullptr ) { std::cerr << "ERROR: event::init_trees() must be called before trees can be written to disk" << std::endl; throw std::exception();}
  train_data_->Write();
}

void event::match_jets( double radius ) {
  
  /** match jets such that the highest pt jets are matched preferentially,
      such that delta_R < resolution
   */
  std::vector<fastjet::PseudoJet> matched_geant;
  std::vector<fastjet::PseudoJet> matched_pythia;
  
  for ( unsigned i = 0; i < pythia_jets_.size(); ++i ) {
    /** match to the geant jet via radial distance, choosing high momentum jets
        preferentially, since both geant_jets and pythia_jets are sorted by pt
     */
    
    fastjet::PseudoJet pythia_jet = pythia_jets_[i];
    fastjet::Selector radial_selector = fastjet::SelectorCircle( radius );
    radial_selector.set_reference( pythia_jet );
    std::vector<fastjet::PseudoJet> matched = radial_selector( geant_jets_ );
    
    // check to make sure the matched jets werent used before...
    for ( unsigned j = 0; j < matched.size(); ++j ) {
      std::ptrdiff_t idx = std::find( matched_geant.begin(), matched_geant.end(), matched[j]  ) - matched_geant.begin();
      if ( idx < int ( matched_geant.size() ) ) {
        matched.erase( std::find( matched.begin(), matched.end(), matched_pythia[idx]  ) );
      }
    }
    
    // if no match exists, skip
    if ( matched.size() == 0 ) continue;
    
    // theres a match: save it
    matched_geant.push_back( matched[0] );
    matched_pythia.push_back( pythia_jet );
    
  }

  geant_jets_ = matched_geant;
  pythia_jets_ = matched_pythia;
  
}

void event::fill_tree( const fastjet::Selector& constituent_selector  ) {
  if ( geant_jets_.size() == 0 ||
      pythia_jets_.size() == 0  ) { return; }
  
  // for charged fraction need the charge selector
  fastjet::Selector charge_selector = SelectorUserIndex( std::vector<int> { -2, -1, 1, 2 } ) * constituent_selector;
  
  train_pt_ = geant_jets_[0].pt();
  train_eta_ = geant_jets_[0].eta();
  train_phi_ = geant_jets_[0].phi();
  train_area_ = geant_jets_[0].area();
  train_npart_ = charge_selector( constituent_selector( geant_jets_[0].constituents() ) ).size();
  train_charge_frac_ = ((double) charge_selector( geant_jets_[0].constituents() ).size() ) / constituent_selector( geant_jets_[0].constituents() ).size();
  train_weight = LookupXsec();
  
  label_pt_ = pythia_jets_[0].pt();
  label_eta_ = pythia_jets_[0].eta();
  label_phi_ = pythia_jets_[0].phi();
  
  train_data_->Fill();
  
}

void event::fill_tree_inclusive( const fastjet::Selector& constituent_selector  ) {
  if ( geant_jets_.size() != pythia_jets_.size() )
  { std::cerr << "error in jet number! exiting" << std::endl; throw -1; }
  
  // for charged fraction need the charge selector
  fastjet::Selector charge_selector = SelectorUserIndex( std::vector<int> { -2, -1, 1, 2 } ) * constituent_selector;
  
  for ( unsigned i = 0; i < geant_jets_.size(); ++i ) {
    train_pt_ = geant_jets_[i].pt();
    train_eta_ = geant_jets_[i].eta();
    train_phi_ = geant_jets_[i].phi();
    train_area_ = geant_jets_[i].area();
    train_npart_ = constituent_selector( geant_jets_[i].constituents() ).size();
    train_charge_frac_ = ((double) charge_selector( geant_jets_[i].constituents() ).size() ) / constituent_selector( geant_jets_[i].constituents() ).size();
    train_weight = LookupXsec();
    
    label_pt_ = pythia_jets_[i].pt();
    label_eta_ = pythia_jets_[i].eta();
    label_phi_ = pythia_jets_[i].phi();
    
    train_data_->Fill();
    
  }
}

//---------------------------------------------------
/** implementation of the selector for UserIdx
 */
fastjet::Selector SelectorUserIndex( const std::vector<int> usr_idx = std::vector<int>{ -2, -1, 0, 1, 2 } ) {
  return fastjet::Selector( new SelectorUserIndexWorker( usr_idx ) );
}



