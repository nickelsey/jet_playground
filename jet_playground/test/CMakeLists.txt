########################################
## simple test cases (mainly for testing cmake at this point)

## tests if I have done something terrible to cmake ...
SET ( INIT_TESTING_SRCS init_test.cc )
ADD_EXECUTABLE ( init_test ${INIT_TESTING_SRCS} )
## putting executables into bin/
SET_TARGET_PROPERTIES( init_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test/ )

## tests if ROOT include directory and libraries are set properly
SET ( ROOT_TESTING_SRCS root_test.cc )
ADD_EXECUTABLE ( root_test ${ROOT_TESTING_SRCS} )
TARGET_LINK_LIBRARIES( root_test ${ROOT_LIBRARIES} )
SET_TARGET_PROPERTIES( root_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test/ )

## tests if FastJet include directory and libraries are set properly
SET ( FASTJET_TESTING_SRCS fastjet_test.cc )
ADD_EXECUTABLE ( fastjet_test ${FASTJET_TESTING_SRCS} )
TARGET_LINK_LIBRARIES ( fastjet_test ${FASTJET_LIBRARIES} )
SET_TARGET_PROPERTIES( fastjet_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test/ )

## tests if TStarJetPico include directory and libraries are set properly
SET ( TSTAR_TESTING_SRCS tstar_test.cc )
ADD_EXECUTABLE ( tstar_test ${TSTAR_TESTING_SRCS} )
TARGET_LINK_LIBRARIES ( tstar_test ${TSTARJETPICO_LIBRARY} ${ROOT_LIBRARIES} )
SET_TARGET_PROPERTIES ( tstar_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test/ )

## tests if I can get cmake regex replace working correctly
CONFIGURE_FILE ( configure_test.cc.in ${CMAKE_CURRENT_SOURCE_DIR}/configure_test.cc )
SET ( CONF_FILE_SRCS configure_test.cc )
ADD_EXECUTABLE ( configure_test ${CONF_FILE_SRCS} )
SET_TARGET_PROPERTIES ( configure_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test/ )
