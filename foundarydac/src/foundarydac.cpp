#include <foundarydac.hpp>

// add new coin
ACTION foundarydac::addnewcoin(name contract, string symstr, uint8_t precision) {
  require_auth(get_self());
  collaterals_table _collaterals(get_self(), get_self().value);

  auto contract_index = _collaterals.get_index<name("contractkey")>();
  auto contracts_itr = contract_index.find(contract.value);
  check(contracts_itr == contract_index.end(), "contract already registered; one token per contract");

  symbol sym = symbol(symstr, precision);
    _collaterals.emplace(get_self(), [&](auto& row) {
      row.id = _collaterals.available_primary_key();
      row.contract = contract;
      row.sym = sym;
      row.upperlimit = 0;
      row.current = 0;
  });

}

ACTION foundarydac::updatelimit(uint64_t id, uint64_t newlimit) {
  
  require_auth(get_self());
  collaterals_table _collaterals(get_self(), get_self().value);
  auto collaterals_itr = _collaterals.find(id);
  
  check(collaterals_itr != _collaterals.end(), "invalid id");
  check(collaterals_itr->current <= newlimit, "newlimit cant be <= current collateral");
  
  _collaterals.modify(collaterals_itr, get_self(), [&](auto& row) {
    row.upperlimit = newlimit;
  });

}

ACTION foundarydac::addcoll(extended_asset ext_collateral_asset) {
  require_auth(name("foundary"));
 
  name contract = ext_collateral_asset.get_extended_symbol().get_contract();
  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  uint64_t amount = ext_collateral_asset.quantity.amount;

  collaterals_table _collaterals(get_self(), get_self().value);
  auto contract_index = _collaterals.get_index<name("contractkey")>();
  auto contracts_itr = contract_index.find(contract.value);

  check(contracts_itr != contract_index.end(), "unrecognized contract");
  check(contracts_itr->sym == collateral_sym, "unrecognized symbol"); 
  check(contracts_itr->current + amount <= contracts_itr->upperlimit, "limit cant be less than current collateral");
  contract_index.modify(contracts_itr, get_self(), [&](auto& row) {
      row.current = contracts_itr->current + amount;
  });
}

ACTION foundarydac::subcoll(extended_asset ext_collateral_asset) {
  require_auth(name("foundary"));
 
  name contract = ext_collateral_asset.get_extended_symbol().get_contract();
  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  uint64_t amount = ext_collateral_asset.quantity.amount;

  collaterals_table _collaterals(get_self(), get_self().value);
  auto contract_index = _collaterals.get_index<name("contractkey")>();
  auto contracts_itr = contract_index.find(contract.value);

  check(contracts_itr != contract_index.end(), "unrecognized contract");
  check(contracts_itr->sym == collateral_sym, "unrecognized symbol"); 
  check(contracts_itr->current - amount > 0, "negative collateral value");
  contract_index.modify(contracts_itr, get_self(), [&](auto& row) {
      row.current = contracts_itr->current - amount;
  });
}


EOSIO_DISPATCH(foundarydac, (addnewcoin)(updatelimit)(addcoll)(subcoll))
