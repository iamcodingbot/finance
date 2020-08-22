// @RAM_PAYER - 
// @AUTHORITY -
// @subtract coll
void foundary::receivebrick(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  auto coll_symbol_contract = get_symbol_contract(std::stoi(memo));
  symbol coll_sym = std::get<0>(coll_symbol_contract);
  name coll_contract = std::get<1>(coll_symbol_contract);

  auto assets = assets_to_return(quantity, coll_sym, coll_contract);
  extended_asset coll_ext_asset = std::get<0>(assets);
  extended_asset mud_ext_asset = std::get<1>(assets);

  action{
    permission_level{get_self(), name("active")},
    name("brickissuer"),
    name("retire"),
    std::make_tuple(quantity, memo)
  }.send();

  action{
      permission_level{get_self(), name("active")},
      name("foundarydac"),
      name("subcoll"),
      std::make_tuple(coll_ext_asset)
  }.send();

  transfer_from_self(to, coll_ext_asset, memo);

  if(mud_ext_asset.quantity.amount > 0) {
    action{
      permission_level{get_self(), name("active")},
      name("mudissuer"),
      name("issue"),
      std::make_tuple(name("foundary"), mud_ext_asset.quantity, memo)
    }.send();

    transfer_from_self(to, mud_ext_asset, memo);
  }

}