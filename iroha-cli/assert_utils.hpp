/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_CLI_ASSERT_UTILS_HPP
#define IROHA_CLI_ASSERT_UTILS_HPP

#include <string>
#include <stdexcept>

namespace iroha_cli {
  /**
   * shuts down process when some error occurs.
   * @param error - error message
   */
  inline void fatal_error(std::string const &error) {
    throw std::runtime_error(error);
  }

  /**
   * shuts down process if a given condition is false.
   * @param condition
   * @param error - error message
   */
  inline void assert_fatal(bool condition, std::string const &error) {
    if (!condition) {
      fatal_error(error);
    }
  }
}  // namespace iroha_cli

#endif  // IROHA_CLI_ASSERT_UTILS_HPP
