void foundary::receivemud(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  bt_table _bt(get_self(), from.value);
  auto bt_itr = _bt.begin();
  check(bt_itr != _bt.end(), "not a valid transaction");

  extended_asset ext_mud_asset = bt_itr->ext_mud_asset;
  symbol mud_sym = ext_mud_asset.get_extended_symbol().get_symbol();
  name mud_contract = ext_mud_asset.get_extended_symbol().get_contract();

  check(mud_sym == quantity.symbol, "symbol mismatch");
  check(mud_contract == get_first_receiver(), "contract must match");
 
  extended_asset new_ext_mud_asset = extended_asset(quantity, mud_contract);

  _bt.modify(bt_itr, name(from), [&](auto& row) {
    row.ext_mud_asset = new_ext_mud_asset;
  });

  int no_of_rows = 0;
  for ( auto itr = bt_itr; itr != _bt.end(); itr++ ) {
    no_of_rows ++ ;
  }
  check(no_of_rows==1, "contact admin, more than 1 record");
}