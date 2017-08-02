## moving scripts to build/bin

FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/models )

CONFIGURE_FILE ( readTree.in.py ${CMAKE_BINARY_DIR}/bin/models/readTree.py )
CONFIGURE_FILE ( tools.in.py ${CMAKE_BINARY_DIR}/bin/models/tools.py )
CONFIGURE_FILE ( train_forest.in.py ${CMAKE_BINARY_DIR}/bin/models/train_forest.py )
CONFIGURE_FILE ( train_tree.in.py ${CMAKE_BINARY_DIR}/bin/models/train_tree.py )
CONFIGURE_FILE ( train_linear.in.py ${CMAKE_BINARY_DIR}/bin/models/train_linear.py )
CONFIGURE_FILE ( train_kn.in.py ${CMAKE_BINARY_DIR}/bin/models/train_kn.py )
CONFIGURE_FILE ( train_nn.in.py ${CMAKE_BINARY_DIR}/bin/models/train_nn.py )
CONFIGURE_FILE ( test_models.in.py ${CMAKE_BINARY_DIR}/bin/models/test_models.py )
CONFIGURE_FILE ( transforms.in.py ${CMAKE_BINARY_DIR}/bin/models/transforms.py )
