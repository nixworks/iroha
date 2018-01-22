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

    using fs = boost::filesystem;
    using sys = boost::system;
    using Identifier = BlockStorage::Identifier;

    boost::optional<std::unique_ptr<BlockStorage>> BlockStorage::create(
        const std::string &path) {
      auto log_ = logger::log("BlockStorage");

      // first, check if directory exists. if not -- create.
      sys::error_code err;
      if (not fs::is_directory(path, err)
          and not fs::create_directory(path, err)) {
        log_->error("Cannot create storage dir: {}\n{}", path, err.message());
        return boost::none;
      }

      // paths to NuDB files
      fs::path dat = fs::path{path} / "iroha.dat";
      fs::path key = fs::path{path} / "iroha.key";
      fs::path log = fs::path{path} / "iroha.log";

      // try to open NuDB database
      nudb::error_code ec;
      auto db = std::make_unique<nudb::store>();
      db->open(dat.string(), key.string(), log.string(), ec);
      if (ec) {
        log_->info("no database at {}, creating new", path);

        // then no database is there. create new database.
        nudb::create<nudb::xxhasher>(dat.string(),
                                     key.string(),
                                     log.string(),
                                     Impl::appid_,
                                     nudb::make_salt(),
                                     sizeof(Identifier),
                                     nudb::block_size("."),
                                     Impl::load_factor_,
                                     ec);

        // and open again
        db->open(dat.string(), key.string(), log.string(), ec);
        if (ec) {
          // error again :sad:
          log_->critical("can not create NuDB database");
          return boost::none;
        }
      }

      log_->info("database at {} successfully opened", path);

      auto bs = std::make_unique<BlockStorage>();
      bs->p_->init(std::move(db));

      // at this point database should be open
      return bs;
    }

    bool BlockStorage::add(Identifier id, const std::vector<uint8_t> &blob) {
      return p_->add(id, blob);
    }

    boost::optional<std::vector<uint8_t>> BlockStorage::get(
        Identifier id) const {
      return p_->get(id);
    }

    boost::optional<Identifier> BlockStorage::last_id() const {
      return p_->last_id();
    }

    boost::optional<Identifier> check_consistency(
       const std::string &path){
      auto log = logger::log("FLAT_FILE");

      if (path.empty()) {
        log->error("check_consistency({}), not directory", path);
        return boost::none;
      }

      auto const files = [&path] {
        std::vector<fs::path> ps;
        std::copy(fs::directory_iterator{path},
                  fs::directory_iterator{},
                  std::back_inserter(ps));
        std::sort(ps.begin(), ps.end(), std::less<fs::path>());
        return ps;
      }();

      auto const missing = boost::range::find_if(
          files | boost::adaptors::indexed(1), [](const auto &it) {
            return BlockStorage::id_to_name(it.index()) != it.value().filename();
          });

      std::for_each(
          missing.get(), files.cend(), [](const boost::filesystem::path &p) {
            boost::filesystem::remove(p);
          });

      return missing.get() - files.cbegin();
    }

  }  // namespace ametsuchi
}  // namespace iroha