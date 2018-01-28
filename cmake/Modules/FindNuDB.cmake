#
# Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
# http://soramitsu.co.jp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

add_library(nudb INTERFACE IMPORTED)

find_path(nudb_INCLUDE_DIR nudb/nudb.hpp)
mark_as_advanced(nudb_INCLUDE_DIR)

find_package_handle_standard_args(nudb DEFAULT_MSG
        nudb_INCLUDE_DIR
        )


set(URL https://github.com/warchant/NuDB.git)
set(VERSION 4535601bb71c578061edd3c3c9098d828ae12b5b) # latest master, 1.0.2
set_target_description(nudb "Fast KV database" ${URL} ${VERSION})


if (NOT nudb_FOUND)
    externalproject_add(warchant_nudb
            GIT_REPOSITORY ${URL}
            GIT_TAG        ${VERSION}
            GIT_SUBMODULES "doc/docca" # disable submodules.
            CMAKE_ARGS     -DBoost_NO_SYSTEM_PATHS=OFF
            CONFIGURE_COMMAND "" # remove configure step
            BUILD_COMMAND     "" # remove build step
            INSTALL_COMMAND   "" # remove install step
            UPDATE_COMMAND    "" # remove update step
            TEST_COMMAND      "" # remove test step
            )
    externalproject_get_property(warchant_nudb source_dir)
    set(nudb_INCLUDE_DIR ${source_dir}/include)
    file(MAKE_DIRECTORY ${nudb_INCLUDE_DIR})

    add_dependencies(nudb warchant_nudb)
endif ()

if(APPLE)
    set(BOOST_THREAD "boost_thread-mt")
else()
    set(BOOST_THREAD "boost_thread")
endif()

set_target_properties(nudb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${nudb_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "boost_system;${BOOST_THREAD}"
        )
