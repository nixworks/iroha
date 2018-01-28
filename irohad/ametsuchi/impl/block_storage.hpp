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

#ifndef IROHA_BLOCK_STORE_HPP
#define IROHA_BLOCK_STORE_HPP

#include <boost/optional.hpp>
#include <memory>
#include <string>
#include <vector>

namespace iroha {
  namespace ametsuchi {

    class BlockStorage {
     public:
      /**
       * Type of storage key
       * @note if you change this, also change DIGITAL_CAPACITY
       */
      using Identifier = uint32_t;

      /**
       * @brief String representation of Identifier can be up to DIGIT_CAPACITY
       * bytes.
       */
      static const uint32_t DIGIT_CAPACITY{10u};

      /**
       * Convert id to a string representation. The string representation is
       * always DIGIT_CAPACITY-character width regardless of the value of `id`.
       * If the length of the string representation of `id` is less than
       * DIGIT_CAPACITY, then the returned value is filled with leading zeros.
       *
       * For example, if str_rep(`id`) is "123", then the returned value is
       * "0000000000000123".
       *
       * @param id - for conversion
       * @return string repr of identifier
       */
      static std::string id_to_name(Identifier id);

      /**
       * Create storage in paths
       * @param path - target path for creating
       * @return created storage
       */
      static boost::optional<std::unique_ptr<BlockStorage>> create(
          const std::string &path);

      /**
       * Add entity with binary data
       * @param id - reference key
       * @param blob - data associated with key
       */
      bool add(Identifier id, const std::vector<uint8_t> &blob);

      const std::string& directory() const;

      /**
       * Get data associated with
       * @param id - reference key
       * @return - blob, if exists
       */
      boost::optional<std::vector<uint8_t>> get(Identifier id) const;

      /**
       * @return number of identifiers in block storage.
       */
      size_t total_keys() const;

      /**
       * @brief Clear database.
       * @return true, if success, false otherwise.
       */
      bool drop_all();

      // ----------| modify operations |----------

      BlockStorage(const BlockStorage &rhs) = delete;
      BlockStorage(BlockStorage &&rhs) = delete;
      BlockStorage &operator=(const BlockStorage &rhs) = delete;
      BlockStorage &operator=(BlockStorage &&rhs) = delete;

     private:
      BlockStorage();

      class Impl;
      std::unique_ptr<Impl> p_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORE_HPP
