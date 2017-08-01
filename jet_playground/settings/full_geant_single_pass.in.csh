#!/bin/csh

set arg = "antikt 0.4 false false ${CMAKE_BINARY_DIR}/settings/reader.txt ${CMAKE_BINARY_DIR}/settings/wsu_grid_geant.list"
qsub -V -q erhiq -lmem=10GB -lnodes=1:ppn=4 -o ${CMAKE_BINARY_DIR}/log/out.log -e ${CMAKE_BINARY_DIR}/log/out.err -N geant_single_pass -- ${CMAKE_BINARY_DIR}/settings/qwrap.sh ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin/jetfinding/process_geant $arg
