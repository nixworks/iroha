/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_BLOCK_STORAGE_NUDB_HPP
#define IROHA_BLOCK_STORAGE_NUDB_HPP

#include "ametsuchi/impl/block_storage.hpp"
#include <nudb/nudb.hpp>
#include "logger/logger.hpp"

namespace iroha {
 namespace ametsuchi {

  class BlockStorage::Impl {
   public:
    /** interface **/
    void init(std::unique_ptr<nudb::store> db);
    bool add(Identifier id, const std::vector<uint8_t> &blob);
    boost::optional<std::vector<uint8_t>> get(Identifier id) const;
    boost::optional<Identifier> last_id() const;
    uint64_t total_blocks() const;
    bool drop_db();

    //< arbitrary number, app-specific
    static const size_t appid_{1337u};
    //< load factor for basket
    static const float load_factor_{0.5f};

   private:
    /** state **/
    //< total number of blocks in database
    uint64_t total_blocks_{0};
    std::unique_ptr<nudb::store> db_;
    logger::Logger log_;
  };

 }
}

#endif //IROHA_BLOCK_STORAGE_NUDB_HPP
