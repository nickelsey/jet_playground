## copying to build directory

FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/settings )

CONFIGURE_FILE ( models.in.txt ${CMAKE_BINARY_DIR}/settings/models.txt )
CONFIGURE_FILE ( reader.in.txt ${CMAKE_BINARY_DIR}/settings/reader.txt )
CONFIGURE_FILE ( wsu_grid_geant.list ${CMAKE_BINARY_DIR}/settings/wsu_grid_geant.list )
CONFIGURE_FILE ( qwrap.sh ${CMAKE_BINARY_DIR}/settings/qwrap.sh )
CONFIGURE_FILE ( full_geant_single_pass.in.csh ${CMAKE_BINARY_DIR}/settings/full_geant_single_pass.csh)
