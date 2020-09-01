#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT crgovernance : public contract {
  public:
    using contract::contract;

    ACTION initialcr();
    ACTION addvote(name from, extended_asset extended_voting_asset, int8_t cr_direction);
    ACTION reducevote(name from, extended_asset extended_voting_asset, int8_t cr_direction);
    ACTION countvotes();
    
  private:

    enum cr_direction: int8_t {
      INCREASE= 1,
      DECREASE = -1,
      SAME=0 
    };
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

    TABLE crvalue {
      uint64_t id;
      string key;
      double value;
      auto primary_key() const { return id; }
    };
    typedef multi_index<name("crvalue"), crvalue> crvalue_table;

   TABLE votestats {
      uint64_t id;
      uint64_t curr_id;
      name contract;
      symbol sym;
      uint64_t up;
      uint64_t down;
      uint64_t same;
      auto primary_key() const { return id; }
      uint64_t curr_cycle_key() const {return curr_id;}
    };
    typedef multi_index<name("votestats"), votestats,
    indexed_by<name("currcyclekey"), const_mem_fun<votestats, uint64_t, &votestats::curr_cycle_key>>> vote_stats_table;

    TABLE votecycle {
      uint64_t id;
      uint64_t cr_vote_cycle_id;
      auto primary_key() const { return id; }
    };
    typedef multi_index<name("votecycle"), votecycle> vote_cycle_table;


    uint64_t user_staked_tokens(name from, extended_asset extended_voting_asset);
    uint64_t cr_voting_cycle_id();
    void add_global_votes(name user, uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction);
    void reduce_global_votes(name user, uint64_t vote_cycle_id, extended_asset extended_voting_asset, int8_t cr_direction);
    double asset_price(symbol sym);
    uint64_t max_votes();
    double quorum();
    double majority();
    double step();
    void adjust_cr(double step) ;
    void update_votedata();
};
