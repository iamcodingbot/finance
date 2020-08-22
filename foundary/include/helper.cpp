void foundary::transfer_from_self(name to, extended_asset e_asset, string memo ) {
    name contract = e_asset.contract;
    asset quantity = e_asset.quantity;
    if(quantity.amount > 0) {
      action{
        permission_level{get_self(), name("active")},
        contract,
        name("transfer"),
        std::make_tuple(name("foundary"), to, quantity, memo)
      }.send();
    }
}

double foundary::get_mud_usdt_price() {
  return 1.00;
}

double foundary::get_collateral_usdt_price(symbol collateral_sym, name collateral_contract) {
  return 1.00;
}

double foundary::get_cr() {
  crvalue_table _crvalue(name("foundarydac"), name("foundarydac").value);
  auto crvalue_itr = _crvalue.find(1);
  double cr = crvalue_itr->value;
  return cr;
}

std::tuple<symbol, name> foundary::get_symbol_contract(uint64_t id) {
    collaterals_table _collaterals(name("foundarydac"), name("foundarydac").value);
    auto collaterals_itr = _collaterals.find(id);
    check(collaterals_itr != _collaterals.end(), "contract not supported");
    return std::make_tuple(collaterals_itr->sym, collaterals_itr->contract);
}

std::tuple<extended_asset, extended_asset> foundary::assets_to_return(asset quantity, symbol collateral_sym, name collateral_contract) {

  double cr = get_cr();

  uint64_t collateral_amount_in_usdt  = (uint64_t)(0.5 + quantity.amount * cr/100);
  uint64_t mud_amount_in_usdt = quantity.amount - collateral_amount_in_usdt;

  double collateral_usdt_price = get_collateral_usdt_price(collateral_sym, collateral_contract);
  double mud_usdt_price = get_mud_usdt_price();

  uint64_t mud_amount = (uint64_t)(0.5 + mud_amount_in_usdt/mud_usdt_price);
  uint64_t collateral_amount = (uint64_t)(0.5 + collateral_amount_in_usdt/collateral_usdt_price);

  const symbol mud_sym = symbol("MUD", 0);

  const asset collateral_asset = asset(collateral_amount, collateral_sym);
  const asset mud_asset = asset(mud_amount, mud_sym);

  const extended_asset ext_collateral_asset = extended_asset(collateral_asset, collateral_contract);
  const extended_asset ext_mud_asset = extended_asset(mud_asset, name("mudissuer"));

  return std::make_tuple(ext_collateral_asset, ext_mud_asset);
}

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> foundary::onlycollateral(extended_asset ext_collateral_asset, extended_asset ext_mud_asset) {
  uint64_t brick_return_amount = 0;
  uint64_t mud_retire_amount = 0;
  uint64_t mud_return_amount = 0;
  uint64_t collateral_return_amount = 0;
  
  
  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  name collateral_contract = ext_collateral_asset.get_extended_symbol().get_contract();
  double collateral_usdt_price = get_collateral_usdt_price(collateral_sym, collateral_contract);

  uint64_t collateral_deposited = ext_collateral_asset.quantity.amount;
  uint64_t deposited_collateral_in_usdt = collateral_deposited * collateral_usdt_price;
  
  brick_return_amount = deposited_collateral_in_usdt;
  mud_retire_amount = 0;
  mud_return_amount = ext_mud_asset.quantity.amount;
  collateral_return_amount = 0;
  
  return std::make_tuple(brick_return_amount, mud_retire_amount, mud_return_amount, collateral_return_amount);
}

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> foundary::partcollateral(extended_asset ext_collateral_asset, extended_asset ext_mud_asset, double cr) {

  uint64_t brick_return_amount = 0;
  uint64_t mud_retire_amount = 0;
  uint64_t mud_return_amount = 0;
  uint64_t collateral_return_amount = 0;

  double mud_usdt_price = get_mud_usdt_price();
  uint64_t mud_deposited = ext_mud_asset.quantity.amount;
  uint64_t deposited_mud_in_usdt = mud_deposited * mud_usdt_price;

  symbol collateral_sym = ext_collateral_asset.get_extended_symbol().get_symbol();
  name collateral_contract = ext_collateral_asset.get_extended_symbol().get_contract();
  double collateral_usdt_price = get_collateral_usdt_price(collateral_sym, collateral_contract);
  uint64_t collateral_deposited = ext_collateral_asset.quantity.amount;
  uint64_t deposited_collateral_in_usdt = collateral_deposited * collateral_usdt_price;
  
  double collateral_percent = 100 * (deposited_collateral_in_usdt)/(deposited_mud_in_usdt + deposited_collateral_in_usdt);
  double mud_percent = 100 - collateral_percent;
  
  double ar = 100 - cr;

  if(cr > collateral_percent) {
    uint64_t amount_of_mud_to_burn_in_usdt = (uint64_t)(0.5 + deposited_collateral_in_usdt * 100 * ar / cr);
    // collateral_deposited * collateral_usdt_price
    mud_retire_amount = (uint64_t)(0.5 + amount_of_mud_to_burn_in_usdt/mud_usdt_price);
    mud_return_amount = mud_deposited - mud_retire_amount;
    brick_return_amount = (deposited_collateral_in_usdt + amount_of_mud_to_burn_in_usdt); 
    collateral_return_amount = 0;
  } else if (cr < collateral_percent) {
    uint64_t usable_collateral_in_usdt = (uint64_t)(0.5 + deposited_mud_in_usdt * 100 * cr / ar);
    uint64_t usable_units_of_collateral = (uint64_t)(0.5 + usable_collateral_in_usdt/collateral_usdt_price);

    collateral_return_amount = collateral_deposited - usable_units_of_collateral;
    mud_retire_amount = mud_deposited;
    mud_return_amount = 0;
    brick_return_amount = deposited_mud_in_usdt + usable_collateral_in_usdt;

  } else if (cr == collateral_percent) {
    brick_return_amount = deposited_mud_in_usdt + deposited_collateral_in_usdt;
    mud_retire_amount = mud_deposited;
    mud_return_amount = 0;
    collateral_return_amount = 0;
  }
  return std::make_tuple(brick_return_amount, mud_retire_amount, mud_return_amount, collateral_return_amount);
}