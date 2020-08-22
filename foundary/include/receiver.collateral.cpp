void foundary::receiveusdt(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  bt_table _bt(get_self(), from.value);
  auto bt_itr = _bt.begin();
  check(bt_itr != _bt.end(), "contact admin, no records");

  extended_asset ext_collateral_asset = bt_itr->ext_collateral_asset;
  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  name collateral_contract = ext_collateral_asset.get_extended_symbol().get_contract();

  check(ext_collateral_asset.quantity.amount == 0, "already received collateral");
  check(collateral_sym == quantity.symbol, "symbol mismatch from starttxn action");
  check(collateral_contract == get_first_receiver(), "contract must match starttxn action");
  
  extended_asset new_ext_collateral_asset = extended_asset(quantity, collateral_contract); 
  
  action{
      permission_level{get_self(), name("active")},
      name("foundarydac"),
      name("addcoll"),
      std::make_tuple(new_ext_collateral_asset)
  }.send();
  
  _bt.modify(bt_itr, name(from), [&](auto& row) {
    row.ext_collateral_asset = new_ext_collateral_asset;
  });

  int no_of_rows = 0;
  for ( auto itr = bt_itr; itr != _bt.end(); itr++ ) {
    no_of_rows ++ ;
  }
  check(no_of_rows==1, "contact admin, more than 1 record");

}

void foundary::receivevigor(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  bt_table _bt(get_self(), from.value);
  auto bt_itr = _bt.begin();
  check(bt_itr != _bt.end(), "contact admin, no records");

  extended_asset ext_collateral_asset = bt_itr->ext_collateral_asset;
  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  name collateral_contract = ext_collateral_asset.get_extended_symbol().get_contract();

  check(ext_collateral_asset.quantity.amount == 0, "already received collateral");
  check(collateral_sym == quantity.symbol, "symbol mismatch from previous");
  check(collateral_contract == get_first_receiver(), "contract must match previous action");

  extended_asset new_ext_collateral_asset = extended_asset(quantity, collateral_contract);
  
  action{
      permission_level{get_self(), name("active")},
      name("foundarydac"),
      name("addcoll"),
      std::make_tuple(new_ext_collateral_asset)
  }.send();
  

  _bt.modify(bt_itr, name(from), [&](auto& row) {
    row.ext_collateral_asset = new_ext_collateral_asset;
  });

  int no_of_rows = 0;
  for ( auto itr = bt_itr; itr != _bt.end(); itr++ ) {
    no_of_rows ++ ;
  }
  check(no_of_rows==1, "contact admin, more than 1 record");
}