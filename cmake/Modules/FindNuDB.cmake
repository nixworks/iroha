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


set(URL https://github.com/vinniefalco/NuDB.git)
set(VERSION 6b4779f737d72038c716d0ecbc0701411dff083e) # latest master, 1.0.2
set_target_description(nudb "Fast KV database" ${URL} ${VERSION})


if (NOT nudb_FOUND)
    externalproject_add(vinniefalco_nudb
            GIT_REPOSITORY ${URL}
            GIT_TAG        ${VERSION}
            CONFIGURE_COMMAND "" # remove configure step
            BUILD_COMMAND     "" # remove build step
            INSTALL_COMMAND   "" # remove install step
            UPDATE_COMMAND    "" # remove update step
            TEST_COMMAND      "" # remove test step
            )
    externalproject_get_property(vinniefalco_nudb source_dir)
    set(nudb_INCLUDE_DIR ${source_dir}/include)
    file(MAKE_DIRECTORY ${nudb_INCLUDE_DIR})

    add_dependencies(nudb vinniefalco_nudb)
endif ()


# TODO: potential bug: if Boost_USE_MULTITHREADED is ON, then list should have suffix -mt, e.g. boost_system-mt

set_target_properties(nudb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${nudb_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "boost_system;boost_thread-mt"
        )

