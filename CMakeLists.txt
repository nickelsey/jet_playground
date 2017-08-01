########################################
## root CMake file for jet playground ##
##              Nick Elsey            ##
########################################

CMAKE_MINIMUM_REQUIRED (VERSION 3.6)

PROJECT ( jet_playground )

## Set CMake search paths for user defined modules
## These are in the project's source dir
SET ( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

########################################
## Set up ROOT environment - package is
## Built into ROOT6 I didn't check for ROOT5
LIST ( APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS} )
FIND_PACKAGE ( ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net )
INCLUDE ( ${ROOT_USE_FILE} )
MESSAGE ( STATUS "Found ROOT" )

########################################
## Setup fastjet environment
FIND_PACKAGE ( FastJet REQUIRED )

########################################
## setup environment variables for TStarJetPico Library
FIND_PACKAGE ( StarJetPico REQUIRED )

#######################################
## Setup pythia environment
FIND_PACKAGE ( Pythia8 REQUIRED )

#######################################
## Setup boost environment
FIND_PACKAGE ( Boost REQUIRED )

########################################
## Including subdirectories
ADD_SUBDIRECTORY ( jet_playground/ )

