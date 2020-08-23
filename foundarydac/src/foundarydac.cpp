#include <foundarydac.hpp>

ACTION foundarydac::updateCR() {

  // if total votes more than 66%, then update.
  // same, same -.5, same + .5
  // if no cr change then stays voted
  // option1, option2, option3
  // delete individual entries
  // update voting balance
}
// cr, voted timestamp
// 100 , 99.5
// 99.5(), 100(), 99()
// 


ACTION foundarydac::updatecr(double newcr) {
  require_auth(get_self());
  crvalue_table _crvalue(get_self(), get_self().value);
  auto crvalue_itr = _crvalue.find(1);
  if (crvalue_itr == _crvalue.end()) {
    _crvalue.emplace(get_self(), [&](auto& row) {
      row.id = 1;
      row.key = "CR";
      row.value = newcr;
    });
  } else {
    _crvalue.modify(crvalue_itr, get_self(), [&](auto& crvalue) {
      crvalue.value = newcr;
    });
  }
}
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


EOSIO_DISPATCH(foundarydac, (updatecr)(addnewcoin)(updatelimit)(addcoll)(subcoll))
