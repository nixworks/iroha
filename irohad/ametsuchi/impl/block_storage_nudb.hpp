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

#include <array>
#include <nudb/nudb.hpp>
#include "ametsuchi/impl/block_storage.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class BlockStorage::Impl {
     public:
      /** interface **/

      Impl();

      static boost::optional<std::unique_ptr<BlockStorage>> create(
          const std::string &path);

      bool init(std::unique_ptr<nudb::store> db, const std::string &path);

      bool add(Identifier id, const std::vector<uint8_t> &blob);

      boost::optional<std::vector<uint8_t>> get(Identifier id) const;

      size_t total_keys() const;

      bool drop_db();

      const std::string &directory() const;

      uint32_t count_blocks(nudb::error_code &ec);

      static std::array<uint8_t, sizeof(uint32_t)> serialize_uint32(uint32_t t);

      //< arbitrary number, app-specific
      static constexpr size_t appid_{0x1337u};
      //< load factor for basket
      static constexpr float load_factor_{0.5f};

     private:
      //< total number of blocks in database
      uint32_t total_blocks_{0};
      std::unique_ptr<nudb::store> db_;
      logger::Logger log_;
      std::string path_;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORAGE_NUDB_HPP
