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

#include "ametsuchi/impl/block_storage_nudb_utility.hpp"
#include "ametsuchi/impl/block_storage.hpp"

namespace iroha {
  namespace ametsuchi {

    uint64_t count_blocks(nudb::store &db, nudb::error_code &ec) {
      BlockStorage::Identifier current = BlockStorage::START_INDEX;
      uint64_t total = 0;

      bool found_last = false;

      do {
        db.fetch(
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
        if (ec) {
          // in case of error
          return 0;
        }
      } while (!found_last);

      return total - BlockStorage::START_INDEX;
    }

    std::array<uint8_t, sizeof(uint32_t)> serialize_uint32(uint32_t t) {
      std::array<uint8_t, sizeof(uint32_t)> b{};

      uint8_t i = 0;
      b[i++] = (t >> 24) & 0xFF;
      b[i++] = (t >> 16) & 0xFF;
      b[i++] = (t >> 8) & 0xFF;
      b[i++] = t & 0xFF;

      return b;
    }

    uint32_t deserialize_uint32(const void *p) {
      uint32_t t = 0;
      const auto *b = static_cast<const char *>(p);

      uint8_t i = 0;
      t |= b[i++] << 24;
      t |= b[i++] << 16;
      t |= b[i++] << 8;
      t |= b[i++];

      return t;
    }
  }  // namespace ametsuchi
}  // namespace iroha