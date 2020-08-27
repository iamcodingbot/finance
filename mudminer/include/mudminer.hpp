#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT mudminer : public contract {
  public:
    using contract::contract;

    [[eosio::on_notify("ammtokens.eo::transfer")]]
    void receiveeidoslp(name from, name to, asset quantity, string memo);

    [[eosio::on_notify("evolutiondex::transfer")]]
    void receivealcorlp(name from, name to, asset quantity, string memo);

    ACTION claim(name from, extended_asset ext_asset);

    ACTION startdiv();

    ACTION creatediv();

    ACTION enddiv();

  private:
    TABLE staked {
      uint64_t id;
      name contract;
      asset stake;
      auto primary_key() const {return id;}
      uint64_t contract_key() const {return contract.value;}
    };
    typedef multi_index<name("staked"), staked,
    indexed_by<name("contractkey"), const_mem_fun<staked, uint64_t, &staked::contract_key>>> staked_table;

    TABLE uservote {
      uint64_t id;
      name contract;
      symbol sym;
      uint64_t up;
      uint64_t down;
      uint64_t same;
      auto primary_key() const {return id;}
      uint64_t contract_key() const {return contract.value;}
    };
    typedef multi_index<name("uservote"), uservote,
    indexed_by<name("contractkey"), const_mem_fun<uservote, uint64_t, &uservote::contract_key>>> user_vote_table;

};
