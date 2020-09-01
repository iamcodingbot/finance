#include <mudminer.hpp>

void mudminer::receiveeidoslp(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  check(quantity.symbol == symbol("BRICKLP",0) || quantity.symbol == symbol("MUDLP",0),
     "not a supported EIDOS token");
  
  staked_table _staked(name("mudminer"), from.value);
  auto contract_index = _staked.get_index<name("contractkey")>();
  auto contract_itr = contract_index.find(get_first_receiver().value);
  while(contract_itr != contract_index.end() && 
    contract_itr->stake.symbol!=quantity.symbol) {
    contract_itr++;
  }
  if(contract_itr == contract_index.end()) {
    _staked.emplace(from, [&](auto& row) {
      row.id = _staked.available_primary_key();
      row.contract = get_first_receiver();;
      row.stake = quantity;
    });
  } else {
    asset curr_balance = contract_itr->stake;
    uint64_t curr_amount = curr_balance.amount;
    uint64_t new_amomunt = curr_balance.amount + quantity.amount;
    asset new_balance = asset(new_amomunt, curr_balance.symbol);
    contract_index.modify(contract_itr, from, [&](auto& row) {
      row.stake = new_balance;
    });
  }
}

void mudminer::receivealcorlp(name from, name to, asset quantity, string memo) {
  if (to != get_self() || from == get_self()){
      eosio::print("Ignoring token transfer of contract to itself.");
      return;
  }
  check(quantity.symbol == symbol("EOSBRI",0) || quantity.symbol == symbol("EOSMUD",0),
     "not a supported ALCOR token");
  
  staked_table _staked(name("mudminer"), from.value);
  auto contract_index = _staked.get_index<name("contractkey")>();
  auto contract_itr = contract_index.find(get_first_receiver().value);
  while(contract_itr != contract_index.end() && 
    contract_itr->stake.symbol!=quantity.symbol) {
    contract_itr++;
  }
  if(contract_itr == contract_index.end()) {
    _staked.emplace(from, [&](auto& row) {
      row.id = _staked.available_primary_key();
      row.contract = get_first_receiver();;
      row.stake = quantity;
    });
  } else {
    asset curr_balance = contract_itr->stake;
    uint64_t curr_amount = curr_balance.amount;
    uint64_t new_amomunt = curr_balance.amount + quantity.amount;
    asset new_balance = asset(new_amomunt, curr_balance.symbol);
    contract_index.modify(contract_itr, from, [&](auto& row) {
      row.stake = new_balance;
    });
  }
}

ACTION mudminer::claimlp(name from, extended_asset ext_asset) {
  require_auth(from);

  uint64_t deposited_amount = 0;
  uint64_t voted_amount = 0;
  // find max withdrawable balance, see if the quantity is less then that
  staked_table _staked(name("mudminer"), from.value);
  auto staked_contract_index = _staked.get_index<name("contractkey")>();
  auto staked_contract_itr = staked_contract_index.find(ext_asset.contract.value);
  while(staked_contract_itr != staked_contract_index.end() && 
    staked_contract_itr->stake.symbol!=ext_asset.quantity.symbol) {
    staked_contract_itr++;
  }
  check(staked_contract_itr == staked_contract_index.end(), "no deposits made of this asset");
  asset deposited_asset = staked_contract_itr->stake;
  deposited_amount = deposited_asset.amount;

  user_vote_table _user_vote(name("crgovernance"), from.value);
  auto contract_index = _user_vote.get_index<name("contractkey")>();
  auto contract_itr = contract_index.find(ext_asset.contract.value);

  while(contract_itr != contract_index.end() && 
    contract_itr->sym!=ext_asset.quantity.symbol) {
    contract_itr++;
  }
  if(contract_itr == contract_index.end()) {
    voted_amount = 0;
  } else {
    voted_amount = contract_itr->up + contract_itr->down + contract_itr->same;
  }

  check(deposited_amount - voted_amount >= ext_asset.quantity.amount, "unvote before withdrawal");

  action{
    permission_level{get_self(), name("active")},
    ext_asset.contract,
    name("transfer"),
    std::make_tuple(get_self(), from, ext_asset.quantity, std::string("stake refund"))
  }.send();

  asset curr_balance = staked_contract_itr->stake;
  uint64_t curr_amount = curr_balance.amount;
  uint64_t new_amomunt = curr_balance.amount - ext_asset.quantity.amount;
  asset new_balance = asset(new_amomunt, curr_balance.symbol);
  staked_contract_index.modify(staked_contract_itr, from, [&](auto& row) {
    row.stake = new_balance;
  });
  if(new_balance == 0) {
    _staked.erase(staked_contract_itr);
  }
}


ACTION mudminer::startdiv() {
  // mark start of div distribution
  // issue MUD tokens
  // getpooldistribution
  // update cycle number
}

ACTION mudminer::creatediv() {
  //
  // modify 
  // 
}

ACTION mudminer::enddiv() {
  //
}


EOSIO_DISPATCH(mudminer, (claimlp)(startdiv)(creatediv)(enddiv))
