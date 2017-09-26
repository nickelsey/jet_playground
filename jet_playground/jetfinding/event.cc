
#include "event.hh"

#include "TSystem.h"

#include "fastjet/ClusterSequenceArea.hh"

#include <exception>
#include <functional>

TLorentzVector ConvertPseudoJet(const fastjet::PseudoJet& jet) {
  TLorentzVector tmp;
  tmp.SetPxPyPzE(jet.px(), jet.py(), jet.pz(), jet.E());
  return tmp;
}

event::event( const std::string& input_file,
              const std::string& settings_doc ) : geant_reader( settings_doc, input_file ),
              train_data_(nullptr), geant_jets_({}), pythia_jets_({}), geant_jet_(),
              pythia_jet_(), geant_constituents_(nullptr), pythia_constituents_(nullptr),
              eventID(0)
{ }

event::~event() {
  delete train_data_;
}

bool event::process_event( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                          const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector,
                          bool charged_jets ) {
  /** selects if we use the charged or full jet reconstruction */
  if ( charged_jets ) return process_charged( jet_def, area_def, constituent_selector,
                                              jet_selector );
  else                return process_full( jet_def, area_def, constituent_selector,
                                             jet_selector );
}

bool event::process_full( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                     const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector
                      ) {
  
  /* clear any jets from the last event
     for cleanliness */
  geant_jets_.clear();
  pythia_jets_.clear();
  
  /* clear the constituents */
  geant_constituents_->Clear();
  pythia_constituents_->Clear();
  
  std::vector<fastjet::PseudoJet> geant_constituents = constituent_selector( geant_pseudojets() );
  std::vector<fastjet::PseudoJet> pythia_constituents = constituent_selector( pythia_pseudojets() );
  
  fastjet::ClusterSequenceArea cluster_geant( geant_constituents, jet_def, area_def );
  fastjet::ClusterSequenceArea cluster_pythia( pythia_constituents, jet_def, area_def );
  
  geant_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_geant.inclusive_jets() ) );
  pythia_jets_ = fastjet::sorted_by_pt( jet_selector( cluster_pythia.inclusive_jets() ) );
  
  match_jets( jet_def.R() );
  
  fill_tree( constituent_selector );
  
  return true;
}

bool event::process_charged( const fastjet::JetDefinition& jet_def, const fastjet::AreaDefinition& area_def,
                            const fastjet::Selector& constituent_selector, const fastjet::Selector jet_selector
                             ) {
  
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
  
  fill_tree( constituent_selector );
  
  return true;
}

void event::init_tree() {
  
  // initialize the reader
  init();
  
  // and initialize the trees with default branches
  train_data_          = new TTree( "training", "training data" );
  
  // event information
  geant_jet_.Clear();
  pythia_jet_.Clear();
  geant_constituents_ = new TClonesArray("TLorentzVector", 100);
  pythia_constituents_ = new TClonesArray("TLorentzVector", 100);
  
  train_data_->Branch("djet", &geant_jet_);
  train_data_->Branch("pjet", &pythia_jet_);
  train_data_->Branch("dconst", &geant_constituents_);
  train_data_->Branch("pconst", &pythia_constituents_);
  
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
    /** match to pythia jet via radial distance, choosing high momentum jets
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
        matched.erase( std::find( matched.begin(), matched.end(), matched_geant[idx]  ) );
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
  if (geant_jets_.size() != pythia_jets_.size()) {
    std::cerr<<"error in matching"<<std::endl;
    return;
  }

  // create event ID first
  std::hash<std::string> hash;
  std::string id_string = std::to_string(get_geant_reader().GetEvent()->GetHeader()->GetRunId())
                        + std::to_string(get_geant_reader().GetEvent()->GetHeader()->GetEventId());
  eventID = hash(id_string);

  
  for (int i = 0; i < geant_jets_.size(); ++i) {
    geant_jet_ = ConvertPseudoJet(geant_jets_[i]);
    pythia_jet_ = ConvertPseudoJet(pythia_jets_[i]);
    
    std::vector<fastjet::PseudoJet> dconst = geant_jets_[i].constituents();
    std::vector<fastjet::PseudoJet> pconst = pythia_jets_[i].constituents();
    
    for (int j = 0; j < dconst[i].constituents().size();++j){
      new((*geant_constituents_)[j]) TLorentzVector(ConvertPseudoJet(dconst[j]));
    }
    for (int j = 0; j < pconst[i].constituents().size();++j){
      new((*pythia_constituents_)[j]) TLorentzVector(ConvertPseudoJet(pconst[j]));
    }
    
    
    train_data_->Fill();
    geant_constituents_->Clear();
    pythia_constituents_->Clear();
  }
  
  
}

//---------------------------------------------------
/** implementation of the selector for UserIdx
 */
fastjet::Selector SelectorUserIndex( const std::vector<int> usr_idx = std::vector<int>{ -2, -1, 0, 1, 2 } ) {
  return fastjet::Selector( new SelectorUserIndexWorker( usr_idx ) );
}


