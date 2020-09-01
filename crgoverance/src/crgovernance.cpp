#include <crgovernance.hpp>
#include <helper.cpp>

ACTION crgovernance::reducevote(name from, extended_asset extended_voting_asset, int8_t cr_direction) {
  require_auth(from);
  check(cr_direction == INCREASE || cr_direction == DECREASE || cr_direction == SAME, "invalid option");

  user_vote_table _user_vote(get_self(), from.value);
  auto user_vote_contract_index = _user_vote.get_index<name("contractkey")>();
  auto user_vote_contract_itr = user_vote_contract_index.find(extended_voting_asset.contract.value);

  // iterate since LP tokens can be issued by same contract.
  while(user_vote_contract_itr != user_vote_contract_index.end() && 
    user_vote_contract_itr->sym != extended_voting_asset.quantity.symbol) {
    user_vote_contract_itr++;
  }

  check(user_vote_contract_itr != user_vote_contract_index.end(), "no active votes using this asset");
  uint64_t user_up_amount = user_vote_contract_itr->up;
  uint64_t user_down_amount = user_vote_contract_itr->down;
  uint64_t user_same_amount = user_vote_contract_itr->same;

  if(cr_direction == INCREASE) {
    user_up_amount = user_up_amount - extended_voting_asset.quantity.amount;
    check(user_up_amount >= 0, "post voting total amount can not be negative");
  } else if(cr_direction == DECREASE) {
    user_down_amount = user_down_amount - extended_voting_asset.quantity.amount;
    check(user_down_amount >= 0, "post voting total amount can not be negative");
  } else if(cr_direction == SAME) {
    user_same_amount = user_same_amount - extended_voting_asset.quantity.amount;
    check(user_same_amount >= 0, "post voting total amount can not be negative");
  }
  user_vote_contract_index.modify(user_vote_contract_itr, from, [&](auto& row) {
    row.up = user_up_amount;
    row.down = user_down_amount;
    row.same = user_same_amount;
  });
  uint64_t voting_cycle_id = cr_voting_cycle_id();
  reduce_global_votes(from, voting_cycle_id, extended_voting_asset, cr_direction);
}

ACTION crgovernance::addvote(name from, extended_asset extended_voting_asset, int8_t cr_direction) {
  require_auth(from);
  check(cr_direction == INCREASE || cr_direction == DECREASE || cr_direction == SAME, "invalid option");
  uint64_t total_user_votes = user_staked_tokens(from, extended_voting_asset);

  user_vote_table _user_vote(get_self(), from.value);
  auto user_vote_contract_index = _user_vote.get_index<name("contractkey")>();
  auto user_vote_contract_itr = user_vote_contract_index.find(extended_voting_asset.contract.value);

  while(user_vote_contract_itr != user_vote_contract_index.end() 
    && user_vote_contract_itr->sym != extended_voting_asset.quantity.symbol) {
    user_vote_contract_itr++;
  }

  uint64_t upvotes = 0;
  uint64_t downvotes = 0;
  uint64_t samevotes = 0;
  uint64_t user_vote_amount = extended_voting_asset.quantity.amount;

  if(user_vote_contract_itr == user_vote_contract_index.end()) {
    check(user_vote_amount <= total_user_votes, "can only vote with " + std::to_string(total_user_votes));

    if(cr_direction == INCREASE) {
      upvotes = user_vote_amount;
    } else if (cr_direction == DECREASE) {
      downvotes = user_vote_amount;
    } else if (cr_direction == SAME){
      samevotes = user_vote_amount;
    }

    _user_vote.emplace(from, [&](auto& row) {
      row.id = _user_vote.available_primary_key();
      row.contract = extended_voting_asset.contract;
      row.sym = extended_voting_asset.quantity.symbol;
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  } else {
    upvotes = user_vote_contract_itr->up;
    downvotes = user_vote_contract_itr->down;
    samevotes = user_vote_contract_itr->same;

    uint64_t already_used_voting_power = upvotes + downvotes + samevotes;
    check(already_used_voting_power + user_vote_amount <= total_user_votes, 
      "can only vote with " + std::to_string(total_user_votes - user_vote_amount));

    if(cr_direction == INCREASE) {
      upvotes = upvotes + user_vote_amount;
    } else if (cr_direction == DECREASE) {
      downvotes = downvotes + user_vote_amount;
    } else if (cr_direction == SAME){
      samevotes = samevotes + user_vote_amount;
    }
    user_vote_contract_index.modify(user_vote_contract_itr, from, [&](auto& row) {
      row.up = upvotes;
      row.down = downvotes;
      row.same = samevotes;
    });
  }
  uint64_t voting_cycle_id = cr_voting_cycle_id();
  add_global_votes(from, voting_cycle_id, extended_voting_asset, cr_direction);
}

// called every X hours
ACTION crgovernance::countvotes() {
  require_auth(get_self());
  uint64_t curr_voting_cycle = cr_voting_cycle_id();
  vote_stats_table _vote_stats(get_self(), get_self().value);

  auto curr_cycle_index = _vote_stats.get_index<name("currcyclekey")>();
  auto curr_cycle_itr = curr_cycle_index.find(curr_voting_cycle);

  double total_up = 0.0;
  double total_down = 0.0;
  double total_same = 0.0;

  while(curr_cycle_itr != curr_cycle_index.end() 
    && curr_cycle_itr->curr_id == curr_voting_cycle) {
    total_up = curr_cycle_itr->up * asset_price(curr_cycle_itr->sym);
    total_down = curr_cycle_itr->down * asset_price(curr_cycle_itr->sym);
    total_same = curr_cycle_itr->same * asset_price(curr_cycle_itr->sym);
    curr_cycle_itr ++;
  }

  double total_votes = total_up + total_same + total_down;
  if(total_votes/max_votes() > quorum()) {
    if(total_up/total_votes > majority()) {
      double stepvalue = step();
      adjust_cr(stepvalue);
      update_votedata();
    } else if (total_down/total_votes > majority()) {
      double stepvalue = - step();
      adjust_cr(stepvalue);
      update_votedata();
    } 
  }
}

ACTION crgovernance::initialcr() {
  require_auth(get_self());
  crvalue_table _crvalue(get_self(), get_self().value);
  auto crvalue_itr = _crvalue.find(1);
  check(crvalue_itr == _crvalue.end(), "CR already intialized");
  _crvalue.emplace(get_self(), [&](auto& row) {
    row.id = 1;
    row.key = "CR";
    row.value = 100.0;
  });
}

EOSIO_DISPATCH(crgovernance, (initialcr)(countvotes)(reducevote)(addvote))
