# add subdirectories

ADD_SUBDIRECTORY ( test )
ADD_SUBDIRECTORY ( jetfinding )
ADD_SUBDIRECTORY ( models )
ADD_SUBDIRECTORY ( settings )

## make the output file for training data
FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/training )
FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/log )
