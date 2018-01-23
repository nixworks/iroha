add_library(block_storage_nudb
        block_storage.cpp
        block_storage_nudb.cpp
        block_storage_nudb_utility.cpp
        )
target_link_libraries(block_storage_nudb
        nudb
        )