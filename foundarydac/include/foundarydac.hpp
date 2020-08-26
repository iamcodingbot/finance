#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT foundarydac : public contract {
  public:
    using contract::contract;

    ACTION updatecr(double newcr);
    ACTION addnewcoin(name contract, string symstr, uint8_t precision);
    ACTION updatelimit(uint64_t id, uint64_t newlimit);
    ACTION addcoll(extended_asset ext_collateral_asset);
    ACTION subcoll(extended_asset ext_collateral_asset);

  private:
    TABLE collaterals {
      uint64_t id;
      name contract;
      symbol sym;
      uint64_t upperlimit;
      uint64_t current;
      auto primary_key() const {return id;}
      uint64_t contract_key() const {return contract.value;}
    };
    typedef multi_index<name("collaterals"), collaterals,
    indexed_by<name("contractkey"), const_mem_fun<collaterals, uint64_t, &collaterals::contract_key>>> collaterals_table;

    TABLE staked {
      uint64_t id;
      name contract;
      asset stake;
      auto primary_key() const {return id;}
      uint64_t contract_key() const {return contract.value;}
    }
    typedef multi_index<name("staked"), staked,
    indexed_by<name("contract"), const_mem_fun<staked, uint64_t, &staked::contract_key>>> staked_table;

    TABLE crvote {
      uint64_t id;
      name contract;
      symbol sym;
      uint64_t up;
      uint64_t down;
      uint64_t same;
      auto primary_key() const {return id;}
      uint64_t contract_key() const {return contract.value;}
    }
    typedef multi_index<name("crvote"), crvote,
    indexed_by<name("contract"), const_mem_fun<staked, uint64_t, &staked::contract_key>>> crvote_table;

    TABLE crvalue {
      uint64_t id;
      string key;
      double value;
      auto primary_key() const { return id; }
    };
    typedef multi_index<name("crvalue"), crvalue> crvalue_table;

    enum cr_direction: int8_t {
      INCREASE= 1,
      DECREASE = -1,
      SAME=0 
    };

    TABLE crstats {
      uint64_t id;
      uint64_t curr_id;
      name contract;
      symbol sym;
      uint64_t up;
      uint64_t down;
      uint64_t same;
      auto primary_key() const { return id; }
      uint64_t curr_cycle_key() const {return curr_id;}
    }
    typedef multi_index<name("crstats"), crstats
    indexed_by<name("currcyclekey"), const_mem_fun<staked, uint64_t, &staked::curr_cycle_key>>> crstats_table;

    TABLE crvotedata {
      uint64_t id;
      uint64_t cr_vote_cycle_id;
    }
    typedef multi_index<name("crvotedata"), crvotedata> crvotedata_table;

};
