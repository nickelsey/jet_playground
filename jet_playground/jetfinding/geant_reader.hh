// Nick Elsey
// 06 - 18 - 17

/*  A wrapper for two TStarJetPicoReaders used to keep the two
    distinct trees in sync on which entry they're reading, and
    handling the differences between the two data sets without
    user intervention
 */

#include "base.hh"

#include "TStarJetPicoReader.h"
#include "TStarJetPicoEvent.h"
#include "TStarJetVectorContainer.h"
#include "TStarJetVector.h"
#include "TStarJetPicoEventHeader.h"

#include "TClonesArray.h"

#include "fastjet/PseudoJet.hh"

#include <string>
#include <vector>

#ifndef JETFINDING_GEANT_READER
#define JETFINDING_GEANT_READER

class geant_reader {

public:
  /** uses default reader settings file for event & track cuts
      - event trigger, track pt, et, DCA, etc
      defaults can be set for data location, tree name, 
      number of events, etc.
   */
  geant_reader();
  
  /** allows user to specify a different settings file, if the
      user wants. syntax is described in the reader.txt settings
      file, and can be copied. Can also use this to specify input
      data file, which will be used to build the TChain. If an empty
      string is passed to settings_doc, a default is used.
   */
  geant_reader( const std::string& settings_doc, const std::string& input_file = "" );
  
  /** default destructor */
  virtual ~geant_reader() {};
  
  /** set the file path for the settings file - this file
      contains options for the reader, such as the number of events
      to run over, and event quality/kinematic cuts. See the example
      file settings/reader.txt for an explation
   */
  void set_settings_file( const std::string& settings_file_path ) { settings_ = settings_file_path; }
  
  /** set the file path for input - can be .root, .txt, .list.
   if a txt or list file is given, the a chain will be built,
   out of the list of input files ( which need to be specified
   with full path, of course )
   */
  void set_input_file( const std::string& input_file_path ) { input_file_path_ = input_file_path; }
  
  
  /** init() must be called before data is read via next(), it sets the
      chain and initializes the readers. If failure is reported via the
      internal TStarJetPicoReaders it is propagated through to the 
      interface. Failure can also come from malformed options specified 
      in the reader.txt settings file.
   */
  bool init();
  
  /** useful for when one must manipulate the readers individually in a
      way not supported by the wrapper. Returns a reference to the readers,
      the assumption being there is no reason to delete the wrapper or readers.
   */
  TStarJetPicoReader& get_geant_reader()     { return geant_reader_; }
  TStarJetPicoReader& get_pythia_reader()    { return pythia_reader_; }
  
  /** will pull the next event until error or reaches the end of the tree
      will attempt to keep both readers in sync by matching tree entry #
      this could be necessary for trigger requirements
   */
  bool next();
 
  /** reads in the tree entry idx. returns: error == -1; event did not
      pass cuts == 0; event successfully loaded == 1
   */
  int read_entry( unsigned int idx );
  
  /** access to the event track information. Use these instead of the primary tracks
      & towers for analyses because the towers have the hadronic correction applied.
      can access either the TStarJetVector or, an STL vector of fastjet::Pseudojets
      The user info of each pseudojet is set to the recorded charge
   */
  TStarJetVectorContainer<TStarJetVector>* pythia_tracks()   { return pythia_reader_.GetOutputContainer(); }
  TStarJetVectorContainer<TStarJetVector>* geant_tracks()    { return geant_reader_.GetOutputContainer( ); }
  
  std::vector<fastjet::PseudoJet> pythia_pseudojets()        { return generate_pseudojets( pythia_tracks() ); }
  std::vector<fastjet::PseudoJet> geant_pseudojets()         { return generate_pseudojets( geant_tracks( ) ); }
  
  /** access to the number of events, the current event number, etc */
  unsigned int current_event()                               { return current_event_; }
  unsigned int total_events()                                { return geant_reader_.GetNOfEvents(); }
  unsigned int accepted_events()                             { return geant_reader_.GetNOfAcceptedEvents(); }
  
private:
  
  
  TStarJetPicoReader geant_reader_;
  TStarJetPicoReader pythia_reader_;
  
  /** the string containing the path to the txt file containing settings
      for the readers.
   */
  std::string settings_;
  
  /** global event counter - both readers should be on the same event at all
      times.
   */
  unsigned int current_event_;
  
  /** path to the root file or the text file containing a path
      to root files where the trees are stored
   */
  std::string input_file_path_;
  
  /** this function is the parser for the settings file - this is part
      of the initialization chain performed by the init() function. Examples
      of how to change the initialization settings can be seen in
      reader.txt file, where the settings are stored
   */
  int parse_settings();
  
  /** we are able to take a string of root files as input to allow
      for wildcard use in passing input via command line or via
      settings file. This function gives that functionality by trying
      to distinguish between escaped spaces and true spaces in the
      string
   */
  std::vector<std::string> parse_root_string();
  
  /** used to process TStarJetVectors into fastjet::Pseudojets
      useful for the jetfinding process, where an std::vector<pseudojet>
      is required
   */
  std::vector<fastjet::PseudoJet> generate_pseudojets( TStarJetVectorContainer<TStarJetVector>* tracks );
  
protected:
  /** used to get the relative weight for each event ( high pT jets are
      oversampled in the Geant data to get weight in the tail of the distribution )
   */
  double LookupXsec ();
  
};

#endif // JETFINDING_GEANT_READER

