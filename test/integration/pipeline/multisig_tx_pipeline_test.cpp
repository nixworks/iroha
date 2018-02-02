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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include "crypto/keys_manager_impl.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"
#include "model/generators/command_generator.hpp"
#include "model/generators/query_generator.hpp"
#include "model/model_crypto_provider_impl.hpp"

class MstPipelineTest : public TxPipelineIntegrationTestFixture {
 public:
  void SetUp() override {
    iroha::ametsuchi::AmetsuchiTest::SetUp();

    auto genesis_tx =
        TransactionGenerator().generateGenesisTransaction(0, {"0.0.0.0:10001"});
    genesis_tx.quorum = 1;
    genesis_block =
        iroha::model::generators::BlockGenerator().generateGenesisBlock(
            0, {genesis_tx});

    manager = std::make_shared<iroha::KeysManagerImpl>("node0");
    auto keypair = manager->loadKeys().value();

    irohad = std::make_shared<TestIrohad>(block_store_path,
                                          redishost_,
                                          redisport_,
                                          pgopt_,
                                          0,
                                          10001,
                                          10,
                                          kProposalDelay,
                                          kVoteDelay,
                                          kLoadDelay,
                                          keypair);

    ASSERT_TRUE(irohad->storage);

    irohad->storage->insertBlock(genesis_block);
    irohad->init();
    irohad->run();
  }

  void TearDown() override {
    iroha::ametsuchi::AmetsuchiTest::TearDown();
    std::remove("node0.pub");
    std::remove("node0.priv");
    std::remove("admin@test.pub");
    std::remove("admin@test.priv");
    std::remove("test@test.pub");
    std::remove("test@test.priv");
    std::remove("multi@test.pub");
    std::remove("multi@test.priv");
  }

  void createMultiQuorumAccount(std::string account,
                                std::initializer_list<std::string> signers,
                                uint32_t quorum) {
    CommandGenerator gen;
    auto domain = "@test";
    auto keypair = createNewAccountKeypair(account + domain);
    iroha::KeysManagerImpl manager("admin@test");
    Transaction tx =
        iroha::model::generators::TransactionGenerator().generateTransaction(
            "admin@test",
            1,
            {gen.generateCreateAccount(account, "test", keypair.pubkey),
             gen.generateSetQuorum(account, quorum)});
    for (auto s : signers) {
      auto key = createNewAccountKeypair(s);
      tx.commands.push_back(gen.generateAddSignatory(s, key.pubkey));
    }

    auto admin_keypair = manager.loadKeys().value();
    iroha::model::ModelCryptoProviderImpl(admin_keypair).sign(tx);
    sendForCommit(tx);
  }

  void sendBlocking(const Transaction &tx) {
    std::unique_lock<std::mutex> lk(send_mutex);
    auto pb_tx = iroha::model::converters::PbTransactionFactory().serialize(tx);

    google::protobuf::Empty response;
    irohad->getCommandService()->ToriiAsync(pb_tx, response);
  }

  void sendForCommit(const Transaction &tx) {
    std::unique_lock<std::mutex> lk(m);
    irohad->getPeerCommunicationService()->on_commit().subscribe(
        [this](auto) { cv.notify_one(); });
    sendBlocking(tx);
    EXPECT_EQ(std::cv_status::no_timeout, cv.wait_for(lk, 20s));
  }

  std::mutex send_mutex;
  const std::chrono::milliseconds kProposalDelay = 1500ms;
  const std::chrono::milliseconds kVoteDelay = 1500ms;
  const std::chrono::milliseconds kLoadDelay = 1500ms;
};

void sign(Transaction &tx, const std::string &account) {
  iroha::KeysManagerImpl manager(account);
  ASSERT_TRUE(manager.loadKeys().has_value());
  auto keypair = manager.loadKeys().value();
  iroha::model::ModelCryptoProviderImpl provider(keypair);
  provider.sign(tx);
}

TEST_F(MstPipelineTest, OnePeerSendsTest) {
  auto account = "multi@test";
  auto signer = "signer@signer";
  createMultiQuorumAccount("multi", {signer}, 2);
  auto cmd =
      iroha::model::generators::CommandGenerator().generateAddAssetQuantity(
          account,
          "coin#test",
          iroha::Amount().createFromString("20.00").value());
  auto tx =
      iroha::model::generators::TransactionGenerator().generateTransaction(
          account, 2, {cmd});
  tx.quorum = 2;

  sign(tx, account);
  sendBlocking(tx);
  auto subscriber =
      irohad->getPeerCommunicationService()->on_commit().subscribe([](auto) {
        FAIL() << "MST tx should be commited without required quorum";
      });
  std::this_thread::sleep_for(kProposalDelay + kVoteDelay);
  subscriber.unsubscribe();

  tx.signatures = {};
  sign(tx, signer);
  sendForCommit(tx);
}

/**
 * @given create peer with default genesis block
 * @when sent valid transaction with quorum 2 and one signature
 *   AND sent same transaction with another signature
 * @then check that block contains transaction
 */
// TEST(MST, OnePeerSendsTest) {
//   // TODO 21/12/2017 muratovv rework with auto erased crypto manager
//   auto user1 = "user1";
//   auto key1 = iroha::create_keypair(), key2 = iroha::create_keypair();
//   iroha::model::generators::CommandGenerator gen;

//   iroha::model::Transaction tx;
//   tx.commands.push_back(gen.generateAddAssetQuantity(
//       user1, "test", iroha::Amount().createFromString("0").value()));

//   integration_framework::IntegrationTestFramework()
//       .setInitialState()  // todo replace with valid genesis block
//       .addUser(user1, key1, {key2.pubkey})
//       .sendTx(addSignature(tx, key1))  // todo replace with tx with sign 1
//       .sendTx(addSignature(tx, key2))  // todo replace with tx with sign 2
//       .skipProposal()
//       .checkBlock([](const auto &block) {
//       })  // todo check that transactions with both signatures appears
//       .done();
// }
