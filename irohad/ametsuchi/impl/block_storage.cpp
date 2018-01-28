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

#include <boost/filesystem.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include "ametsuchi/impl/block_storage_nudb.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    namespace fs = boost::filesystem;
    namespace sys = boost::system;
    using Identifier = BlockStorage::Identifier;

    BlockStorage::BlockStorage() : p_(new Impl()){};

    boost::optional<std::unique_ptr<BlockStorage>> BlockStorage::create(
        const std::string &path) {
      return BlockStorage::Impl::create(path);
    }

    bool BlockStorage::add(Identifier id, const std::vector<uint8_t> &blob) {
      return p_->add(id, blob);
    }

    boost::optional<std::vector<uint8_t>> BlockStorage::get(
        Identifier id) const {
      return p_->get(id);
    }

    bool BlockStorage::drop_all() {
      return p_->drop_db();
    }

    const std::string& BlockStorage::directory() const {
      return p_->directory();
    }

    size_t BlockStorage::total_keys() const {
      return p_->total_keys();
    }


  }  // namespace ametsuchi
}  // namespace iroha
