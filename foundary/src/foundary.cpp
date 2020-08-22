#include <foundary.hpp>
#include <helper.cpp>
#include <receiver.collateral.cpp>
#include <receiver.mud.cpp>
#include <receiver.brick.cpp>

ACTION foundary::starttxn(name from,
    uint64_t id) {
  require_auth(from);
  collaterals_table _collaterals(name("foundarydac"), name("foundarydac").value);
  auto collaterals_itr = _collaterals.find(id);
  check(collaterals_itr != _collaterals.end(), "unknown id");

  bt_table _bt(get_self(), from.value);
  auto bt_itr = _bt.begin();
  check(bt_itr == _bt.end(), "record already present");
  
  const symbol mud_sym = symbol("MUD", 0);
  asset mud_asset = asset(0, mud_sym);
  extended_asset extended_mud_asset = extended_asset(mud_asset, name("mudissuer"));

  symbol collateral_sym = collaterals_itr->sym;
  name collateral_contract = collaterals_itr->contract;
  asset collateral_asset = asset(0, collateral_sym);
  extended_asset extended_collateral_asset = extended_asset(collateral_asset, name(collateral_contract));
  
  _bt.emplace(name(from), [&](auto& row) {
    row.id = _bt.available_primary_key();
    row.ext_mud_asset = extended_mud_asset;
    row.ext_collateral_asset = extended_collateral_asset;
  });
}

ACTION foundary::settle(name from) {
  require_auth(from);

  bt_table _bt(get_self(), from.value);
  auto bt_itr = _bt.begin();
  check(bt_itr != _bt.end(), "invalid txn");

  extended_asset ext_mud_asset = bt_itr->ext_mud_asset;
  extended_asset ext_collateral_asset = bt_itr->ext_collateral_asset;

  
  double cr = get_cr();

  std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> settlement_assets;
  if(cr == 100) {
    settlement_assets = onlycollateral(ext_collateral_asset, ext_mud_asset);
  } else {
    settlement_assets = partcollateral(ext_collateral_asset, ext_mud_asset, cr);
  }


  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  const symbol brick_sym = symbol("BRICK", 0);
  const symbol mud_sym = symbol("MUD", 0);
  
  uint64_t brick_return_amount = std::get<0>(settlement_assets);
  uint64_t mud_retire_amount = std::get<1>(settlement_assets);
  uint64_t mud_return_amount = std::get<2>(settlement_assets);
  uint64_t collateral_return_amount = std::get<3>(settlement_assets);

  if(mud_retire_amount>0) {
    asset mud_retire_asset = asset(mud_retire_amount, mud_sym); 
    action{
      permission_level{get_self(), name("active")},
      name("mudissuer"),
      name("retire"),
      std::make_tuple(mud_retire_asset, std::string(""))
    }.send();
  }
  if(mud_return_amount>0) {
    asset mud_return_asset = asset(mud_return_amount, mud_sym); 
    action{
      permission_level{get_self(), name("active")},
      name("mudissuer"),
      name("transfer"),
      std::make_tuple(name("foundary"), name(from), mud_return_asset, std::string("refund"))
    }.send(); 
  }
  if(collateral_return_amount>0) {
    asset collateral_return_asset = asset(collateral_return_amount, collateral_sym);
    name collateral_contract = ext_collateral_asset.get_extended_symbol().get_contract();
    action{
      permission_level{get_self(), name("active")},
      name(collateral_contract),
      name("transfer"),
      std::make_tuple(name("foundary"), name(from), collateral_return_asset, std::string("refund"))
    }.send();
  }
  if(brick_return_amount>0) {
    asset brick_asset = asset(brick_return_amount, brick_sym);
    action{
      permission_level{get_self(), name("active")},
      name("brickissuer"),
      name("issue"),
      std::make_tuple(name("foundary"), brick_asset, std::string(""))
    }.send();

    action{
      permission_level{get_self(), name("active")},
      name("brickissuer"),
      name("transfer"),
      std::make_tuple(name("foundary"), name(from), brick_asset, std::string("new brick"))
    }.send();
  }
  _bt.erase(bt_itr);
}