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

#include "ametsuchi/impl/block_storage_nudb.hpp"
#include "ametsuchi/impl/block_storage.hpp"
#include "ametsuchi/impl/block_storage_nudb_utility.hpp"

#include <boost/filesystem.hpp>
#include <iomanip>
#include <sstream>
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    using fs = boost::filesystem;
    using sys = boost::system;

    std::string BlockStorage::id_to_name(Identifier id) {
      std::ostringstream os;
      os << std::setw(BlockStorage::DIGIT_CAPACITY) << std::setfill('0') << id;
      return os.str();
    }

    void BlockStorage::Impl::init(std::unique_ptr<nudb::store> db) {
      db_ = std::move(db);
      log_ = logger::log("BlockStorage::Impl::init()");

      nudb::error_code ec;
      total_blocks_ = count_blocks(*db_, ec);
      if (ec) {
        log_->critical("can not read database to count blocks");
        // TODO(warchant): react somehow. exception?
      }
    }

    bool BlockStorage::Impl::add(Identifier id,
                                 const std::vector<uint8_t> &blob) {
      nudb::error_code ec;
      db_->insert(serialize_uint32(id), blob.data(), blob.size(), ec);
      return !ec;  // if ec is true, then add returns false (error happened!)
    }

    boost::optional<std::vector<uint8_t>> BlockStorage::Impl::get(
        Identifier id) const {
      nudb::error_code ec;
      boost::optional<std::vector<uint8_t>> ret;
      db_->fetch(serialize_uint32(id),
                 [&ret](const void *p, size_t size) {
                   if (size == 0) {
                     ret = boost::none;
                   } else {
                     ret = std::vector<uint8_t>{p, p + size};
                   }
                 },
                 ec);
      if (ec) {
        return boost::none;
      }

      return ret;
    }

    boost::optional<Identifier> BlockStorage::Impl::last_id() const {
      return total_blocks_ < START_INDEX ? boost::none
                                         : total_blocks_ + START_INDEX;
    }

    uint64_t BlockStorage::Impl::total_blocks() const {
      return total_blocks_;
    }

    bool BlockStorage::Impl::drop_db(){

    }

  }  // namespace ametsuchi
}  // namespace iroha