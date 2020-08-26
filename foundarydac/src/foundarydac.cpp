#include <foundarydac.hpp>

ACTION foundarydac::reducecrvote(name from, extended_asset extended_voting_asset, int8_t cr_direction) {
  require_auth(from);
  check(cr_direction == INCREASE || cr_direction == DECREASE || cr_direction == SAME, "invalid option");

  crvote_table _crvote(get_self(), from.value);
  auto crvote_contract_index = _crvote.get_index<name("contractkey")>();
  auto crvote_contracts_itr = crvote_contract_index.find(extended_voting_asset.contract.value);

  while(crvote_contracts_itr !=crvote_contract_index.end() && crvote_contracts_itr->sym!=quantity.symbol) {
    contract_itr = contract_itr.next();
  }

  check(crvote_contracts_itr !=crvote_contract_index.end(), "no active votes using this asset");
  if(cr_direction == INCREASE) {
    check(crvote_contracts_itr->up >= extended_voting_asset.quantity.amount, "active amount voting for CR increse is " + std::to_string(crvote_contracts_itr->up));
    crvote_contract_index.modify(crvote_contracts_itr, from, [&](auto& row) {
      row.up = crvote_contracts_itr->up - extended_voting_asset.quantity.amount;
    });
  } else if(cr_direction == DECREASE) {
    check(crvote_contracts_itr->down >= extended_voting_asset.quantity.amount, "active amount voting for CR decrease is " + std::to_string(crvote_contracts_itr->down));
    crvote_contract_index.modify(crvote_contracts_itr, from, [&](auto& row) {
      row.down = crvote_contracts_itr->down - extended_voting_asset.quantity.amount;
    });
  } else if(cr_direction == SAME) {
    check(crvote_contracts_itr->same >= extended_voting_asset.quantity.amount, "active amount voting for CR to stay ame is " + std::to_string(crvote_contracts_itr->same));
    crvote_contract_index.modify(crvote_contracts_itr, from, [&](auto& row) {
      row.same = crvote_contracts_itr->same - extended_voting_asset.quantity.amount;
    });
  }
}

ACTION foundarydac::addcrvote(name from, extended_asset extended_voting_asset, int8_t cr_direction) {
  require_auth(from);
  check(cr_direction == INCREASE || cr_direction == DECREASE || cr_direction == SAME, "invalid option");
  uint64_t total_user_votes = user_staked_tokens(from, extended_voting_asset);

  crvote_table _crvote(get_self(), from.value);
  auto crvote_contract_index = _crvote.get_index<name("contractkey")>();
  auto crvote_contracts_itr = crvote_contract_index.find(extended_voting_asset.contract.value);

  while(crvote_contracts_itr !=crvote_contract_index.end() && crvote_contracts_itr->sym!=quantity.symbol) {
    contract_itr = contract_itr.next();
  }

  if(crvote_contracts_itr == crvote_contract_index.end()) {
    check(quantity<=total_votes, "can only vote with " + std::to_string(total_votes));
    uint64_t upvotes = 0;
    uint64_t downvotes = 0;
    uint64_t samevotes = 0;
    if(cr_direction == INCREASE) {
      upvotes = quantity;
    } else if (cr_direction == DECREASE) {
      downvotes = quantity;
    } else if (cr_direction == SAME){
      samevotes = quantity;
    }

    _crvote.emplace(from, [&](auto& row) {
      row.id = _crvote.get_available_key();
      row.contract = quantity.contract;
      row.sym = quantity.symbol;
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  } else {
    uint64_t upvotes = crvote_contracts_itr->up;
    uint64_t downvotes = crvote_contracts_itr->down;
    uint64_t samevotes = crvote_contracts_itr->same;

    uint64_t already_used_voting_power = upvotes + downvotes + samevotes;
    check(already_used_voting_power + quantity <=total_votes, "can only vote with " + std::to_string(total_votes - quantity));

    if(cr_direction == INCREASE) {
      upvotes = upvotes + quantity;
    } else if (cr_direction == DECREASE) {
      downvotes = downvotes + quantity;
    } else if (cr_direction == SAME){
      samevotes = samevotes + quantity;
    }
    crvote_contract_index.modify(crvote_contracts_itr, from, [&](auto& row) {
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  }
  uint64_t voting_cycle_id = cr_voting_cycle_id();
  cr_global_add_votes(voting_cycle_id, extended_voting_asset, cr_direction);
}

// called every X hours
ACTION foundarydac::countvotes() {
  require_auth(get_self());
  uint64_t curr_voting_cycle = cr_voting_cycle_id();
  crstats_table _crstats(get_self(), get_self().value);

  auto curr_cycle_index = _crstats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(curr_voting_cycle);

  double total_up = 0.0;
  double total_down = 0.0;
  double total_same = 0.0;

  while(curr_cycle_itr != curr_cycle_index.end() && curr_cycle_itr->curr_id == curr_voting_cycle) {
    total_up = curr_cycle_itr->up * asset_price(curr_cycle_itr->sym);
    total_down = curr_cycle_itr->down * asset_price(curr_cycle_itr->sym);
    total_same = curr_cycle_itr->same * asset_price(curr_cycle_itr->sym);
  }

  double total_votes = total_up + total_same + total_down;
  if(total_votes/max_votes > quorum) {
    if(total_up/total_votes > majority()) {
      double step = step();
      adjust_cr(step);
      update_votedata();
    } else if (total_down/total_votes > majority()) {
      double step = - step();
      adjust_cr(step);
      update_votedata();
    } else if (total_same/total_votes > majority()) {
    
    } 
  }
}


uint64_t foundarydac::user_staked_tokens(name from, extended_asset extended_voting_asset) {
  staked_table _staked(name("mudminer"), from.value);
  auto contract_index = _staked.get_index<name("contractkey")>();
  auto contracts_itr = contract_index.find(extended_voting_asset.contract.value);
  while(contracts_itr !=contract_index.end() && contract_itr->stake.symbol!=extended_voting_asset.quantity.symbol) {
    contract_itr = contract_itr.next();
  }
  check(contracts_itr != contract_index.end(), "contract not in stake table");

  asset asset_staked = contracts_itr->stake;
  uint64_t user_votes = asset_staked.amount;
  return user_votes;
}

uint64_t foundarydac::cr_voting_cycle_id() {
  crvotedata_table _crvotedata(get_self(), get_self().value);
  auto crvotedata_itr = _crvotedata.find(1);
  check(crvotedata_itr != _crvotedata.end(), "error, something gone wrong");
  return crvotedata_itr->cr_vote_cycle_id;
}

void foundarydac::cr_global_add_votes(uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction) {
  uint64_t upvotes = 0;
  uint64_t downvotes = 0;
  uint64_t samevotes = 0;
  
  crstats_table _crstats(get_self(), get_self().value);
  auto curr_cycle_index = _crstats.get_index<name("currcycle")>();
  auto curr_cycle_itr = curr_cycle_index.find(vote_cycle_id);
  while(curr_cycle_itr != curr_cycle_index.end() || (curr_cycle_itr->sym != extended_voting_asset.quantity.symbol) ) {
    curr_cycle_itr = curr_cycle_itr.next();
  }

  if(curr_cycle_itr == curr_cycle_index.end()) {
    if(cr_direction == UP) {
      upvotes = extended_voting_asset.quantity.amount;
    } else if (cr_direction == DOWN) {
      downvotes = extended_voting_asset.quantity.amount;
    } else if (cr_direction == SAME) {
      samevotes = extended_voting_asset.quantity.amount;
    }
    _crstats.emplace(get_self(), [&](auto& row) {
      row.id = _crstats.get_available_key();
      row.contract = extended_voting_asset.contract;
      row.sym = extended_voting_asset.quantity.symbol;
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  } else {
    upvotes = curr_cycle_itr->up;
    downvotes = curr_cycle_itr->down;
    samevotes = curr_cycle_itr->same;
    if(cr_direction == UP) {
      upvotes = upvotes + extended_voting_asset.quantity.amount;
    } else if (cr_direction == DOWN) {
      downvotes = downvotes + extended_voting_asset.quantity.amount;
    } else if (cr_direction == SAME) {
      samevotes = samevotes + extended_voting_asset.quantity.amount;
    }
    curr_cycle_index.modify(curr_cycle_itr, get_self(), [&](auto& row) {
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  }
}

void foundarydac::cr_global_reduce_votes(uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction) {
  crstats_table _crstats(get_self(), get_self().value);
  auto curr_cycle_index = _crstats.get_index<name("currcycle")>();
  auto curr_cycle_itr = curr_cycle_index.find(vote_cycle_id);
  while(curr_cycle_itr != curr_cycle_index.end() || (curr_cycle_itr->sym != extended_voting_asset.quantity.symbol) ) {
    curr_cycle_itr = curr_cycle_itr.next();
  }
  check(curr_cycle_itr != curr_cycle_index.end(), "no records found in global stats");

  upvotes = curr_cycle_itr->up;
  downvotes = curr_cycle_itr->down;
  samevotes = curr_cycle_itr->same;
  if(cr_direction == UP) {
    upvotes = upvotes - extended_voting_asset.quantity.amount;
  } else if (cr_direction == DOWN) {
    downvotes = downvotes - extended_voting_asset.quantity.amount;
  } else if (cr_direction == SAME) {
    samevotes = samevotes - extended_voting_asset.quantity.amount;
  }
  curr_cycle_index.modify(curr_cycle_itr, get_self(), [&](auto& row) {
    row.up = upvotes;
    row.down = downvotes;
    row.same = samevotes;
  });
}

double foundarydac::asset_price(symbol sym) {
  return 1.0;
}

uint64_t foundarydac::max_votes() {
  return 1;
}

double foundarydac::quorum() {
  return .66;
}

double foundarydac::majority() {
  return .5;
}

double foundarydac::step() {
  return 0.5;
}



void foundarydac::adjust_cr(double step) {
  crvalue_table _crvalue(get_self(), get_self().value);
  auto crvalue_itr = _crvalue.find(1);
  if (crvalue_itr == _crvalue.end()) {
    _crvalue.emplace(get_self(), [&](auto& row) {
      row.id = 1;
      row.key = "CR";
      row.value = 100.00;
    });
  } else {
    _crvalue.modify(crvalue_itr, get_self(), [&](auto& crvalue) {
      crvalue.value = crvalue.value + step;
    });
  }
}

void foundarydac::update_votedata() {
  crvotedata_table _crvotedata(get_self(), get_self().value);
  auto crvotedata_itr = _crvotedata.find(1);
  check(crvotedata_itr != _crvotedata.end(), "error, something gone wrong");
  uint64_t new_voting_cycle_id =  crvotedata_itr->cr_vote_cycle_id + 1;
  _crvotedata.modify(crvotedata_itr, get_self(), [&](auto& row) {
    row.cr_vote_cycle_id = new_voting_cycle_id ;
  });

  crstats_table _crstats(get_self(), get_self().value);

  auto curr_cycle_index = _crstats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(curr_voting_cycle);
  while(curr_cycle_itr != curr_cycle_index.end() && curr_cycle_itr->curr_id == curr_voting_cycle_id) {
    _crstats.emplace(get_self(), [&](auto& row) {
      row.id = _crstats.available_primary_key();
      row.curr_id = new_voting_cycle_id;
      row.contract = curr_cycle_itr->contract;
      row.sym = curr_cycle_itr->sym;
      row.up = curr_cycle_itr->up;
      row.down = curr_cycle_itr->down;
      row.same =curr_cycle_itr->same;
    });
  }
}



uint64_t foundarydac::get_brick_supply() {
  return 0;
}

uint64_t foundarydac::get_mud_supply() {
  return 0;
}

double foundarydac::get_mud_brick_price() {
  return 0.0;
}

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
