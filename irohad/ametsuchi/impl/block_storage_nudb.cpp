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

#include <boost/filesystem.hpp>
#include <iomanip>

namespace iroha {
  namespace ametsuchi {

    namespace fs = boost::filesystem;
    namespace sys = boost::system;
    using Identifier = BlockStorage::Identifier;

    std::string BlockStorage::id_to_name(Identifier id) {
      std::ostringstream os;
      os << std::setw(BlockStorage::DIGIT_CAPACITY) << std::setfill('0') << id;
      return os.str();
    }

    const std::string &BlockStorage::Impl::directory() const {
      return path_;
    }

    bool BlockStorage::Impl::init(std::unique_ptr<nudb::store> db,
                                  const std::string &path) {
      db_ = std::move(db);
      log_ = logger::log("BlockStorage::Impl::init()");

      nudb::error_code ec;
      total_blocks_ = count_blocks(ec);  // replaces check_consistency
      // TODO(warchant): validate blocks during initialization

      if (ec != nudb::error::key_not_found) {
        log_->critical("can not read database to count blocks: {}",
                       ec.message());
        return false;
      }

      path_ = path;

      return true;
    }

    bool BlockStorage::Impl::add(Identifier id,
                                 const std::vector<uint8_t> &blob) {
      nudb::error_code ec;
      db_->insert(serialize_uint32(id).data(), blob.data(), blob.size(), ec);
      if(ec){
        log_->error("BlockStorage::add(): {}", ec.message());
        return false;
      }
      total_blocks_++;
      return true;
    }

    boost::optional<std::vector<uint8_t>> BlockStorage::Impl::get(
        Identifier id) const {
      nudb::error_code ec;
      boost::optional<std::vector<uint8_t>> ret;
      db_->fetch(serialize_uint32(id).data(),
                 [&ret](const void *p, size_t size) {
                   if (size == 0) {
                     ret = boost::none;
                   } else {
                     const auto *c = static_cast<const char *>(p);
                     ret = std::vector<uint8_t>{c, c + size};
                   }
                 },
                 ec);
      if (ec) {
        log_->error("BlockStorage::get(): {}", ec.message());
        return boost::none;
      }

      return ret;
    }

    Identifier BlockStorage::Impl::last_id() const {
      if (total_blocks_ < BlockStorage::START_INDEX) {
        return 0;  // = no blocks in storage
      } else {
        return total_blocks_ + BlockStorage::START_INDEX;
      }
    }

    uint64_t BlockStorage::Impl::total_blocks() const {
      return total_blocks_;
    }

    bool BlockStorage::Impl::drop_db() {
      nudb::error_code ec1, ec2, ec3;

      nudb::erase_file(db_->dat_path(), ec1);
      nudb::erase_file(db_->log_path(), ec2);
      nudb::erase_file(db_->key_path(), ec3);

      return !(ec1 || ec2 || ec3);
    }

    uint32_t BlockStorage::Impl::count_blocks(nudb::error_code &ec) {
      BlockStorage::Identifier current = BlockStorage::START_INDEX;
      uint32_t total = 0;

      bool found_last = false;

      do {
        db_->fetch(
            serialize_uint32(current).data(),
            [&total, &found_last, &current](const void *value, size_t size) {
              // if we read 0 bytes, then there is no such key
              if (size == 0u) {
                // if total=3, then current=3 will read 0 bytes, so
                // total=current here
                total = current;
                found_last = true;
                return;
              }

              // go to the next key
              current++;
            },
            ec);
        if (ec == nudb::error::key_not_found) {
          return total;
        } else if (ec) {
          // some other error occurred
          return 0;
        }
      } while (!found_last);

      return total - BlockStorage::START_INDEX;
    }

    std::array<uint8_t, sizeof(uint32_t)> BlockStorage::Impl::serialize_uint32(
        uint32_t t) {
      std::array<uint8_t, sizeof(uint32_t)> b{};

      uint8_t i = 0;
      b[i++] = (t >> 24) & 0xFF;
      b[i++] = (t >> 16) & 0xFF;
      b[i++] = (t >> 8) & 0xFF;
      b[i++] = t & 0xFF;

      return b;
    }

  }  // namespace ametsuchi
}  // namespace iroha