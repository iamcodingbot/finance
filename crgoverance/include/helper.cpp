uint64_t crgovernance::user_staked_tokens(name from, extended_asset extended_voting_asset) {
  staked_table _staked(name("mudminer"), from.value);
  auto contract_index = _staked.get_index<name("contractkey")>();
  auto contract_itr = contract_index.find(extended_voting_asset.contract.value);
  while(contract_itr != contract_index.end() && 
    contract_itr->stake.symbol!=extended_voting_asset.quantity.symbol) {
    contract_itr++;
  }
  check(contract_itr != contract_index.end(), "contract not in stake table");

  asset asset_staked = contract_itr->stake;
  uint64_t user_votes = asset_staked.amount;
  return user_votes;
}

uint64_t crgovernance::cr_voting_cycle_id() {
  vote_cycle_table _vote_cycle(get_self(), get_self().value);
  auto vote_cycle_itr = _vote_cycle.find(1);
  check(vote_cycle_itr != _vote_cycle.end(), "error, something gone wrong");
  return vote_cycle_itr->cr_vote_cycle_id;
}

void crgovernance::add_global_votes(name user, uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction) {
  uint64_t upvotes = 0;
  uint64_t downvotes = 0;
  uint64_t samevotes = 0;
  
  vote_stats_table _vote_stats(get_self(), get_self().value);
  auto curr_cycle_index = _vote_stats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(vote_cycle_id);
  while(curr_cycle_itr != curr_cycle_index.end() &&
   (curr_cycle_itr->sym != extended_voting_asset.quantity.symbol)) {
    curr_cycle_itr++;
  }

  uint64_t user_vote_amount = extended_voting_asset.quantity.amount;

  if(curr_cycle_itr == curr_cycle_index.end()) {
    if(cr_direction == INCREASE) {
      upvotes = user_vote_amount;
    } else if (cr_direction == DECREASE) {
      downvotes = user_vote_amount;
    } else if (cr_direction == SAME) {
      samevotes = user_vote_amount;
    }
    _vote_stats.emplace(user, [&](auto& row) {
      row.id = _vote_stats.available_primary_key();
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
    if(cr_direction == INCREASE) {
      upvotes = upvotes + user_vote_amount;
    } else if (cr_direction == DECREASE) {
      downvotes = downvotes + user_vote_amount;
    } else if (cr_direction == SAME) {
      samevotes = samevotes + user_vote_amount;
    }
    curr_cycle_index.modify(curr_cycle_itr, user, [&](auto& row) {
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  }
}

void crgovernance::reduce_global_votes(name user, uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction) {
  vote_stats_table _vote_stats(get_self(), get_self().value);
  auto curr_cycle_index = _vote_stats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(vote_cycle_id);
  while(curr_cycle_itr != curr_cycle_index.end() && 
    (curr_cycle_itr->sym != extended_voting_asset.quantity.symbol)) {
    curr_cycle_itr ++;
  }
  check(curr_cycle_itr != curr_cycle_index.end(), "no records found in global stats");

  uint64_t upvotes = curr_cycle_itr->up;
  uint64_t downvotes = curr_cycle_itr->down;
  uint64_t samevotes = curr_cycle_itr->same;
  uint64_t user_vote_amount = extended_voting_asset.quantity.amount;

  if(cr_direction == INCREASE) {
    upvotes = upvotes - user_vote_amount;
  } else if (cr_direction == DECREASE) {
    downvotes = downvotes - user_vote_amount;
  } else if (cr_direction == SAME) {
    samevotes = samevotes - user_vote_amount;
  }
  curr_cycle_index.modify(curr_cycle_itr, user, [&](auto& row) {
    row.up = upvotes;
    row.down = downvotes;
    row.same = samevotes;
  });
}

double crgovernance::asset_price(symbol sym) {
  return 1.0;
}

uint64_t crgovernance::max_votes() {
  return 10000000;
}

double crgovernance::quorum() {
  return .66;
}

double crgovernance::majority() {
  return .5;
}

double crgovernance::step() {
  return 0.5;
}

void crgovernance::adjust_cr(double step) {
  crvalue_table _crvalue(get_self(), get_self().value);
  auto crvalue_itr = _crvalue.find(1);
  check(crvalue_itr != _crvalue.end(), "something's wrong; no cr value in table"); 
  _crvalue.modify(crvalue_itr, get_self(), [&](auto& crvalue) {
    crvalue.value = crvalue.value + step;
  }); 
}

void crgovernance::update_votedata() {
  vote_cycle_table _vote_cycle(get_self(), get_self().value);
  auto vote_cycle_itr = _vote_cycle.find(1);
  check(vote_cycle_itr != _vote_cycle.end(), "error, something gone wrong");
  uint64_t curr_voting_cycle_id = vote_cycle_itr->cr_vote_cycle_id;
  uint64_t new_voting_cycle_id = curr_voting_cycle_id + 1;
  _vote_cycle.modify(vote_cycle_itr, get_self(), [&](auto& row) {
    row.cr_vote_cycle_id = new_voting_cycle_id ;
  });

  vote_stats_table _vote_stats(get_self(), get_self().value);

  auto curr_cycle_index = _vote_stats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(curr_voting_cycle_id);
  while(curr_cycle_itr != curr_cycle_index.end() && 
    curr_cycle_itr->curr_id == curr_voting_cycle_id) {
    _vote_stats.emplace(get_self(), [&](auto& row) {
      row.id = _vote_stats.available_primary_key();
      row.curr_id = new_voting_cycle_id;
      row.contract = curr_cycle_itr->contract;
      row.sym = curr_cycle_itr->sym;
      row.up = curr_cycle_itr->up;
      row.down = curr_cycle_itr->down;
      row.same =curr_cycle_itr->same;
    });
  }
}