#include <mudminer.hpp>

ACTION mudminer::blp(name from, name to, asset quantity, string memo) {
  require_auth(from);
  action{
    permission_level{get_self(), name("active")},
    name("mudminer"),
    name("issue"),
    std::make_tuple(quantity, std::string(""))
  }.send();

  action{
    permission_level{get_self(), name("active")},
    name("mudminer"),
    name("transfer"),
    std::make_tuple(get_self(), from, quantity, std::string(""))
  }.send();
}

ACTION mudminer::claimblp(name from, name to, asset quantity, string memo) {
  require_auth(from);
  action{
    permission_level{get_self(), name("active")},
    name("eidoslp"),
    name("transfer"),
    std::make_tuple(get_self(), from, quantity, std::string(""))
  }.send();

  action{
    permission_level{get_self(), name("active")},
    name("mudminer"),
    name("retire"),
    std::make_tuple(get_self(), from, quantity, std::string(""))
  }.send();

  // no active vote
  // return eidoslp token
  // if calc on going, then block claiming
}

ACTION mudminer::mlp(name from, name to, asset quantity, string memo) {
  require_auth(from);
  // issue vote1 token
  // transfer to other apart from mudminer
}

ACTION mudminer::claimmlp(name from, name to, asset quantity, string memo) {
  // no active vote
  // return eidoslp token
  // if calc on going, then block claiming
}

ACTION mudminer::eosbri(name from, name to, asset quantity, string memo) {
  require_auth(from);
  // issue vote1 token
  // transfer to other apart from mudminer

}

ACTION mudminer::claimeosbri(name from, name to, asset quantity, string memo) {
  // no active vote
  // return eidoslp token
}

ACTION mudminer::startdiv() {

}

ACTION mudminer::creatediv() {

  // reward can be claimed every hour.
  // only if tokens are staked at beginning of hour.
  // update claim table
}

ACTION mudminer::enddiv() {

}

ACTION mudminer::claim(name from) {
  // transfer from self to from
}


EOSIO_DISPATCH(mudminer, (hi)(clear))
