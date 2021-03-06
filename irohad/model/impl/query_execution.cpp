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

#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/query_execution.hpp"
#include "model/execution/common_executor.hpp"
#include "model/permissions.hpp"
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_detail_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"

#include <boost/algorithm/string.hpp>
#include <utility>

using namespace iroha::model;
using namespace iroha::ametsuchi;

QueryProcessingFactory::QueryProcessingFactory(
    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery)
    : _wsvQuery(std::move(wsvQuery)), _blockQuery(std::move(blockQuery)) {}

std::string getDomainFromName(const std::string &account_id) {
  std::vector<std::string> res;
  boost::split(res, account_id, boost::is_any_of("@"));
  return res.size() > 1 ? res.at(1) : "";
}

bool hasQueryPermission(const std::string &creator,
                        const std::string &target_account,
                        WsvQuery &wsv_query,
                        const std::string &indiv_permission_id,
                        const std::string &all_permission_id,
                        const std::string &domain_permission_id) {
  auto perms_set = getAccountPermissions(creator, wsv_query);
  return
      // 1. Creator has grant permission from other user
      (creator != target_account
       and wsv_query.hasAccountGrantablePermission(
               creator, target_account, indiv_permission_id))
      or  // ----- Creator has role permission ---------
      (perms_set.has_value()
       and (
               // 2. Creator want to query his account, must have role
               // permission
               (creator == target_account
                and accountHasPermission(perms_set.value(),
                                         indiv_permission_id))
               or  // 3. Creator has global permission to get any account
               (accountHasPermission(perms_set.value(),
                                     all_permission_id))
               or  // 4. Creator has domain permission
               (getDomainFromName(creator) == getDomainFromName(target_account)
                and accountHasPermission(perms_set.value(),
                                         domain_permission_id))));
}

bool QueryProcessingFactory::validate(
    const model::GetAssetInfo &query) {
  // TODO: check signatures
  return checkAccountRolePermission(
      query.creator_account_id, *_wsvQuery, can_read_assets);
}

bool QueryProcessingFactory::validate(
    const model::GetRoles &query) {
  // TODO: check signatures
  return checkAccountRolePermission(
      query.creator_account_id, *_wsvQuery, can_get_roles);
}

bool QueryProcessingFactory::validate(const model::GetRolePermissions &query) {
  // TODO: check signatures
  return checkAccountRolePermission(
      query.creator_account_id, *_wsvQuery, can_get_roles);
}

bool QueryProcessingFactory::validate(
    const model::GetAccount &query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_account,
                            can_get_all_accounts,
                            can_get_domain_accounts);
}

bool QueryProcessingFactory::validate(
    const model::GetSignatories &query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_signatories,
                            can_get_all_signatories,
                            can_get_domain_signatories);
}

bool QueryProcessingFactory::validate(
    const model::GetAccountAssets &query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_acc_ast,
                            can_get_all_acc_ast,
                            can_get_domain_acc_ast);
}

bool QueryProcessingFactory::validate(
  const model::GetAccountDetail& query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_acc_detail,
                            can_get_all_acc_detail,
                            can_get_domain_acc_detail
                            );
}

bool QueryProcessingFactory::validate(
    const model::GetAccountTransactions &query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_acc_txs,
                            can_get_all_acc_txs,
                            can_get_domain_acc_txs);
}

bool QueryProcessingFactory::validate(
    const model::GetAccountAssetTransactions &query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id,
                            query.account_id,
                            *_wsvQuery,
                            can_get_my_acc_ast_txs,
                            can_get_all_acc_ast_txs,
                            can_get_domain_acc_ast_txs);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetAssetInfo(const model::GetAssetInfo &query) {
  auto ast = _wsvQuery->getAsset(query.asset_id);
  if (!ast.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ASSET;
    return std::make_shared<ErrorResponse>(response);
  }
  AssetResponse response;
  response.asset = std::move(ast.value());
  response.query_hash = iroha::hash(query);
  return std::make_shared<AssetResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetRoles(const model::GetRoles &query) {
  auto roles = _wsvQuery->getRoles();
  if (not roles.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ROLES;
    return std::make_shared<ErrorResponse>(response);
  }
  RolesResponse response;
  response.query_hash = iroha::hash(query);
  response.roles = std::move(roles.value());
  return std::make_shared<RolesResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetRolePermissions(
    const model::GetRolePermissions &query) {
  auto perm = _wsvQuery->getRolePermissions(query.role_id);
  if (not perm.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ROLES;
    return std::make_shared<ErrorResponse>(response);
  }
  RolePermissionsResponse response;
  response.query_hash = iroha::hash(query);
  response.role_permissions = std::move(perm.value());
  return std::make_shared<RolePermissionsResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetAccount(
    const model::GetAccount &query) {
  auto acc = _wsvQuery->getAccount(query.account_id);
  auto roles = _wsvQuery->getAccountRoles(query.account_id);
  if (not acc.has_value() or not roles.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ACCOUNT;
    return std::make_shared<ErrorResponse>(response);
  }

  AccountResponse response;
  response.account = acc.value();
  response.roles = roles.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<AccountResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetAccountAssets(
    const model::GetAccountAssets &query) {
  auto acct_asset =
      _wsvQuery->getAccountAsset(query.account_id, query.asset_id);
  if (!acct_asset.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ACCOUNT_ASSETS;
    return std::make_shared<ErrorResponse>(response);
  }
  AccountAssetResponse response;
  response.acct_asset = acct_asset.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<AccountAssetResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountDetail(
  const model::GetAccountDetail& query) {
  auto acct_detail =
    _wsvQuery->getAccountDetail(query.account_id, query.creator_account_id, query.detail);
  if (!acct_detail.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT_DETAIL;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  iroha::model::AccountDetailResponse response;
  response.detail = acct_detail.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<iroha::model::AccountDetailResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssetTransactions(
    const model::GetAccountAssetTransactions& query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetTransactions(query.account_id,
                                                               query.asset_id);
  TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_asset_tx;
  return std::make_shared<TransactionsResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetAccountTransactions(
    const model::GetAccountTransactions &query) {
  auto acc_tx = _blockQuery->getAccountTransactions(query.account_id);
  TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_tx;
  return std::make_shared<TransactionsResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::executeGetSignatories(
    const model::GetSignatories &query) {
  auto signs = _wsvQuery->getSignatories(query.account_id);
  if (!signs.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = model::ErrorResponse::NO_SIGNATORIES;
    return std::make_shared<ErrorResponse>(response);
  }
  SignatoriesResponse response;
  response.query_hash = iroha::hash(query);
  response.keys = signs.value();
  return std::make_shared<SignatoriesResponse>(response);
}

std::shared_ptr<QueryResponse>
QueryProcessingFactory::execute(
    std::shared_ptr<const model::Query> query) {
  // TODO 26/09/17 Nasrulin: change to handler map or/with templates #VARIANT
  if (instanceof <GetAccount>(query.get())) {
    auto qry = std::static_pointer_cast<const GetAccount>(query);

    if (!validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAccount(*qry);
  }
  if (instanceof <GetAccountAssets>(query.get())) {
    auto qry =
        std::static_pointer_cast<const GetAccountAssets>(query);
    if (!validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAccountAssets(*qry);
  }
  if (instanceof <iroha::model::GetAccountDetail>(query.get())) {
    auto qry =
      std::static_pointer_cast<const iroha::model::GetAccountDetail>(query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountDetail(*qry);
  }
  if (instanceof <iroha::model::GetSignatories>(query.get())) {
    auto qry =
        std::static_pointer_cast<const GetSignatories>(query);
    if (!validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetSignatories(*qry);
  }
  if (instanceof <GetAccountTransactions>(query.get())) {
    auto qry =
        std::static_pointer_cast<const GetAccountTransactions>(
            query);
    if (!validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAccountTransactions(*qry);
  }
  if (instanceof <GetAccountAssetTransactions>(query.get())) {
    auto qry = std::
        static_pointer_cast<const GetAccountAssetTransactions>(
            query);
    if (!validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAccountAssetTransactions(*qry);
  }
  if (instanceof <GetRoles>(query.get())) {
    auto qry = std::static_pointer_cast<const GetRoles>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetRoles(*qry);
  }
  if (instanceof <GetRolePermissions>(query.get())) {
    auto qry = std::static_pointer_cast<const GetRolePermissions>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetRolePermissions(*qry);
  }
  if (instanceof <GetAssetInfo>(query.get())) {
    auto qry = std::static_pointer_cast<const GetAssetInfo>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAssetInfo(*qry);
  }
  ErrorResponse response;
  response.query_hash = iroha::hash(*query);
  response.reason = model::ErrorResponse::NOT_SUPPORTED;
  return std::make_shared<ErrorResponse>(response);
}
