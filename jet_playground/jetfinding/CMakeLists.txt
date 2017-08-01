########################################
## building the geant/pythia comparison analysis
## including two wrappers - one for the TStarJetPicoReader
## and one for the fastjet interface

CONFIGURE_FILE ( geant_reader.in.cc ${CMAKE_CURRENT_SOURCE_DIR}/geant_reader.cc )
CONFIGURE_FILE ( process_geant.in.cc ${CMAKE_CURRENT_SOURCE_DIR}/process_geant.cc )
SET ( GEANT_READER_SRCS geant_reader.cc geant_reader.hh )
SET ( EVENT_SRCS event.cc event.hh )
SET ( PROCESS_GEANT_SRCS process_geant.cc )
ADD_EXECUTABLE ( process_geant ${GEANT_READER_SRCS} ${PROCESS_GEANT_SRCS} ${EVENT_SRCS} )
TARGET_LINK_LIBRARIES ( process_geant ${FASTJET_LIBRARIES} ${TSTARJETPICO_LIBRARY} ${ROOT_LIBRARIES} )
## putting executables into bin/
SET_TARGET_PROPERTIES( process_geant PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/jetfinding/ )
