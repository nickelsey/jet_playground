// implementation for geant_reader class

#include "geant_reader.hh"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "TStarJetPicoEventCuts.h"
#include "TStarJetPicoTrackCuts.h"
#include "TStarJetPicoTowerCuts.h"
#include "TStarJetPicoUtils.h"

#include "TString.h"
#include "TFile.h"

/** helper function to split strings into substrings
    with respect to a specified character.
    used for parsing the settings file when initializing
    the readers
 */
std::vector<std::string> split_string( const std::string s, std::string split );

/** Used to understand which format of input file is being used
    ( .root file, .txt, .list, etc )
*/
bool HasEnding (std::string const &full_string, std::string const &ending);

/** default initializer that assumes an unmodified file structure in the 
    source directory, it will run with "normal" reader settings
 */
geant_reader::geant_reader() {
  settings_ = "/Users/nick/physics/analysis/jet_playground/build/settings/reader.txt";
  input_file_path_ = "";

}

/** allows the user to specify both a non-default settings file, and a 
    input file for the data trees
 */
geant_reader::geant_reader( const std::string& settings_doc, const std::string& input_file ) {
  if ( settings_doc == "" ) settings_ = "/Users/nick/physics/analysis/jet_playground/build/settings/reader.txt";
  else settings_ = settings_doc;
  input_file_path_ = input_file;
}

bool geant_reader::init() {
  
  // try to parse all settings found in
  // the settings file. Unknown strings are
  // exceptions, not ignored.
  int n_events = -1;
  try {
    n_events = parse_settings();
  } catch ( std::exception& e ) {
    return false;
  }
  
  // now build the file input chain from the provided string
  // Build our input now
  TChain* chain = new TChain( "JetTree" );
  TChain* mcChain = new TChain( "JetTreeMc" );
  
  if ( HasEnding( input_file_path_, ".root" ) ) {
    
    std::vector<std::string> input_files = parse_root_string();
  
    for ( unsigned i = 0; i < input_files.size(); ++i ) {
      
      chain->Add( input_files[i].c_str() );
      mcChain->Add( input_files[i].c_str() );
    }
  
  } else if ( HasEnding( input_file_path_.c_str(), ".txt" ) ||
              HasEnding( input_file_path_.c_str(), ".list" ) ) {
    
    chain = TStarJetPicoUtils::BuildChainFromFileList( input_file_path_.c_str(), "JetTree" );
    mcChain = TStarJetPicoUtils::BuildChainFromFileList( input_file_path_.c_str(), "JetTreeMc" );
  
  }
  
  geant_reader_.SetInputChain( chain );
  pythia_reader_.SetInputChain( mcChain );
  
  // set hadronic correction for both to 100%
  // to follow what others have done I set to 0.999
  // I am unsure if 1.0 causes an issue, or what
  // other reason there is for doing so.
  geant_reader_.SetApplyFractionHadronicCorrection( true );
  geant_reader_.SetFractionHadronicCorrection( 0.999 );
  pythia_reader_.SetApplyFractionHadronicCorrection( true );
  pythia_reader_.SetFractionHadronicCorrection( 0.999 );
  
  // finally, set a dummy hot tower list - geant doesnt simulate
  // hot towers
  geant_reader_.GetTowerCuts()->AddBadTowers( "/Users/nick/physics/analysis/jet_playground/jet_playground/jetfinding/dummy_tower_list.txt" );
  pythia_reader_.GetTowerCuts()->AddBadTowers( "/Users/nick/physics/analysis/jet_playground/jet_playground/jetfinding/dummy_tower_list.txt" );
  
  // and initialize to run over n_events
  pythia_reader_.Init( n_events );
  geant_reader_.Init( n_events );
  
  return true;
}


bool geant_reader::next() {
  
  // Print out reader status every 10 seconds
  geant_reader_.PrintStatus(20);

  int geant_status = geant_reader_.NextEvent();
  int pythia_status = 0;
  
  do {
    pythia_status = pythia_reader_.ReadEvent( geant_reader_.GetNOfCurrentEvent() );
    
    if ( pythia_status == 1 ) break;
    
    geant_reader_.NextEvent();
  } while ( geant_status );
  
  current_event_ = pythia_reader_.GetNOfCurrentEvent();
  
  return pythia_status*geant_status != 0;
}

int geant_reader::read_entry( unsigned int idx ) {
  
  int pythia_status = pythia_reader_.ReadEvent( idx );
  int geant_status = geant_reader_.ReadEvent( idx );
  
  if ( pythia_status == -1 ) { __ERR(Form("pythia reader: error reading in event #%u",idx)) return -1; }
  if ( geant_status == -1 ) { __ERR(Form("geant reader: error reading in event #%u",idx)) return -1; }
  
  current_event_ = idx;
  
  return pythia_status * geant_status;
  
}

/* this function is used to parse the settings file.
   its ugly and not great. First time writing a parser.
   it works as intended though! 
 */

int geant_reader::parse_settings() {
  
  // will return  the # of events to run over
  // by default run over all with -1
  int n_events = -1;
  
  std::string line;
  std::ifstream settings_file( settings_ );
  if ( settings_file.is_open() )
  {
    while ( getline( settings_file, line ) )
    {
      
      // if the line is a comment, ignore it
      if ( *(line.begin()) == '#' ) continue;
      
      // if the line is empty or simple new line or escape character, ignore it
      if ( *(line.begin()) == '\n' || line.size() == 0 || *(line.begin()) == '\t' ) continue;
      
      // split on :: character - this will split on the category of the following setting
      std::vector<std::string> init_tokens{ split_string( line, "::" ) };
      
      //the second token should be a variable & a value, separated by an equals.
      // splitting on the equals
      std::vector<std::string> tokens { split_string( init_tokens[1], "=" ) };
      
      // if there are not two tokens, or if there is more than one equals sign, throw an error
      if ( tokens.size() != 2 || init_tokens.size() != 2 ) { std::string msg = line + " is not properly formatted as a setting option";
        __ERR( msg.c_str() )
        throw std::exception();
      }
      // the first token should be a scope operator, one of: all, geant, or pythia.
      // that will define which readers the option is applied to.
      // the second token is then used to apply modifiers to the readers
      if  ( init_tokens[0] == "all" ) {
        if ( tokens[0] == "data" ) { if  (input_file_path_ == "") input_file_path_ = tokens[1]; }
        else if ( tokens[0] == "number_of_events" ) n_events = stoi( tokens[1] );
        else if ( tokens[0] == "trigger" ) {
          pythia_reader_.GetEventCuts()->SetTriggerSelection( tokens[1].c_str() );
          geant_reader_.GetEventCuts()->SetTriggerSelection( tokens[1].c_str() );
          
        } else if ( tokens[0] == "refmult_cut" ) {
        
          pythia_reader_.GetEventCuts()->SetRefMultCut( stof( tokens[1] ) );
          geant_reader_.GetEventCuts()->SetRefMultCut( stof( tokens[1] ) );
        
        } else if ( tokens[0] == "vz_cut" ) {
        
          pythia_reader_.GetEventCuts()->SetVertexZCut( stof( tokens[1] ) );
          geant_reader_.GetEventCuts()->SetVertexZCut( stof( tokens[1] ) );
        
        } else if ( tokens[0] == "vz_diff_cut" ) {
        
          pythia_reader_.GetEventCuts()->SetVertexZDiffCut( stof( tokens[1] ) );
          geant_reader_.GetEventCuts()->SetVertexZDiffCut( stof( tokens[1] ) );
        
        } else if ( tokens[0] == "event_pt_cut" ) {
        
          pythia_reader_.GetEventCuts()->SetMaxEventPtCut( stof( tokens[1] ) );
          geant_reader_.GetEventCuts()->SetMaxEventPtCut( stof( tokens[1] ) );
        
        } else if ( tokens[0] == "event_et_cut" ) {
          /* this one is special - in pythia, there are no towers
             so even though its set for all, it'll only be applied to
             geant
           */
          geant_reader_.GetEventCuts()->SetMaxEventEtCut( stof( tokens[1] ) );
        
        } else { std::string  msg = tokens[0] + " is not a option in all:: scope."; __ERR( msg.c_str() ); throw std::exception(); }
      
      }
      else if ( init_tokens[0] == "geant" ) {
        
        if ( tokens[0] == "dca_cut" ) {
      
          geant_reader_.GetTrackCuts()->SetDCACut( stof( tokens[1] ) );
          
        } else if ( tokens[0] == "min_fit_points" ) {
          
          geant_reader_.GetTrackCuts()->SetMinNFitPointsCut( stoi( tokens[1] ) );
        
        } else if ( tokens[0] == "min_fit_point_frac" ) {
          
          geant_reader_.GetTrackCuts()->SetFitOverMaxPointsCut( stof( tokens[1] ) );
        
        } else { std::string  msg = tokens[0] + " is not a option in geant:: scope."; __ERR( msg.c_str() ); throw std::exception(); }
        
      }
      else if ( init_tokens[0] == "pythia" ) {
        
        if ( tokens[0] == "dca_cut" ) {
          
          pythia_reader_.GetTrackCuts()->SetDCACut( stof( tokens[1] ) );
          
        } else if ( tokens[0] == "min_fit_points" ) {
          
          pythia_reader_.GetTrackCuts()->SetMinNFitPointsCut( stoi( tokens[1] ) );
          
        } else if ( tokens[0] == "min_fit_point_frac" ) {
          
          pythia_reader_.GetTrackCuts()->SetFitOverMaxPointsCut( stof( tokens[1] ) );
          
        } else { std::string  msg = tokens[0] + " is not a option in pythia:: scope."; __ERR( msg.c_str() ); throw std::exception(); }

        
      }
      else { std::string  msg = init_tokens[0] + " is not a valid scope."; __ERR( msg.c_str() ); throw std::exception(); }
    
      
    }
    settings_file.close();
  }
  
  return n_events;
}

std::vector<std::string> geant_reader::parse_root_string() {
  
  std::string work_string = input_file_path_;
  
  std::vector<std::string> rtn_strings;
  
  int base = 0;
  for ( unsigned i = 0; i < work_string.size(); ++i ) {
    std::string tmp = work_string.substr( base, i - base + 1 );
    
    // there are constraints of the length given by the required
    // .root ending
    if ( ( i - base + 1 ) < 5 || !HasEnding( tmp, ".root" )) continue;
    if ( i < work_string.size() -1 && work_string.substr( i+1, 1 ) != " " ) continue;
  
    
    // it ends in .root, we can assume its a proper substring, add it
    rtn_strings.push_back( tmp );
    
    // set base  & i to the next possible location
    i = i + 1;
    base = i + 1;
  }
  
  return rtn_strings;
}

std::vector<fastjet::PseudoJet> geant_reader::generate_pseudojets( TStarJetVectorContainer<TStarJetVector>* container ) {
  
  std::vector<fastjet::PseudoJet> tracks;
  
  // Transform TStarJetVectors into (FastJet) PseudoJets
  TStarJetVector* sv;
  for ( int i=0; i < container->GetEntries() ; ++i ){
    sv = container->Get(i);
    
    fastjet::PseudoJet tmpPJ = fastjet::PseudoJet( *sv );
    tmpPJ.set_user_index( sv->GetCharge() );
    tracks.push_back( tmpPJ );
    
  }
  
  return tracks;
}

double geant_reader::LookupXsec(  ){
  
  TString filename = geant_reader_.GetInputChain()->GetCurrentFile()->GetName();
  
  // Some data for geant
  // -------------------
  //cross-sections for simulated GEANT data sample
  // From Renee.
  // also available via
  // http://people.physics.tamu.edu/sakuma/star/jets/c101121_event_selection/s0150_mclist_001/web.php
  // Double_t MinbXsec=28.12;
  // Double_t Xsec[12];
  // Xsec[0]=28.11;//2
  // Xsec[1]=1.287;//3
  // Xsec[2]=0.3117;//4
  // Xsec[3]=0.1360;//5
  // Xsec[4]=0.02305;//7
  // Xsec[5]=0.005494;//9
  // Xsec[6]=0.002228;//11
  // Xsec[7]=0.0003895;//15
  // Xsec[8]=0.00001016;//25
  // Xsec[9]=0.0000005010;//35
  // Xsec[10]=0.0000000283;//45
  // Xsec[11]=0.000000001443;//55
  
  // static const Double_t MinbXsec=28.12;
  // static const Double_t Xsec[12] = {
  //   28.11,		// 2-3
  //   1.287,		// 3-4
  //   0.3117,		// 4-5
  //   0.1360,		// 5-7
  //   0.02305,		// 7-9
  //   0.005494,		// 9-11
  //   0.002228,		// 11-15
  //   0.0003895,		// 15-25
  //   0.00001016,		// 25-35
  //   0.0000005010,	// 35-45
  //   0.0000000283,	// 45-55
  //   0.000000001443	// 55-65
  // };
  
  static const Double_t Xsec[12] = {
    1.0,        // Placeholder for 2-3
    1.30E+09,	// 3-4
    3.15E+08,	// 4-5
    1.37E+08,	// 5-7
    2.30E+07,	// 7-9
    5.53E+06,	// 9-11
    2.22E+06,	// 11-15
    3.90E+05,	// 15-25
    1.02E+04,	// 25-35
    5.01E+02,	// 35-45
    2.86E+01,	// 45-55
    1.46E+00	// 55-65
  };
  
  static const Double_t Nmc[12] = {
    1,			// 2-3
    672518,		// 3-4
    672447,		// 4-5
    393498,		// 5-7
    417659,		// 7-9
    412652,		// 9-11
    419030,		// 11-15
    396744,		// 15-25
    399919,		// 25-35
    119995,		// 35-45
    117999,		// 45-55
    119999		// 55-65
  };
  
  Double_t w[12];
  for ( int i=0; i<12 ; ++i ){
    w[i] = Xsec[i] / Nmc[i];
    // w[i] = Nmc[i] / Xsec[i] ;
  }
  
  // static const Double_t w[12] = {
  //   1,			// Placeholder
  //   1.90E+03,
  //   6.30E+02,
  //   3.43E+02,
  //   5.49E+01,
  //   1.33E+01,
  //   5.30E+00,
  //   9.81E-01,
  //   2.56E-02,
  //   4.56E-03,
  //   2.43E-04,
  //   1.20E-05
  // };
  
  if ( filename.Contains("picoDst_3_4") ) return w[1];
  if ( filename.Contains("picoDst_4_5") ) return w[2];
  if ( filename.Contains("picoDst_5_7") ) return w[3];
  if ( filename.Contains("picoDst_7_9") ) return w[4];
  if ( filename.Contains("picoDst_9_11") ) return w[5];
  if ( filename.Contains("picoDst_11_15") ) return w[6];
  if ( filename.Contains("picoDst_15_25") ) return w[7];
  if ( filename.Contains("picoDst_25_35") ) return w[8];
  if ( filename.Contains("picoDst_35_45") ) return w[9];
  if ( filename.Contains("picoDst_45_55") ) return w[10];
  if ( filename.Contains("picoDst_55_65") ) return w[11];
  
  return 1;
  
}



std::vector<std::string> split_string( const std::string s, std::string split ) {
  
  std::string buffer{""};
  std::vector<std::string> tokens;
  
  int iter_length = split.size();
  
  for ( unsigned i = 0; i < s.size(); ++i ) {
    if ( s.substr( i, iter_length ) != split ) buffer += s[i];
    if ( s.substr( i, iter_length ) == split && buffer != "") { tokens.push_back( buffer ); i += iter_length-1; buffer = ""; }
  }
  if ( buffer != "" ) tokens.push_back( buffer );
  
  // strip whitespace from the characters - they won't be needed
  for ( unsigned i = 0; i < tokens.size(); ++i ) tokens[i].erase (std::remove( tokens[i].begin(), tokens[i].end(), ' ' ), tokens[i].end() );
  
  return tokens;
}

bool HasEnding (std::string const &full_string, std::string const &ending) {
    if (full_string.length() >= ending.length()) {
      return (0 == full_string.compare (full_string.length() - ending.length(), ending.length(), ending) );
    } else {
      return false;
    }
}




