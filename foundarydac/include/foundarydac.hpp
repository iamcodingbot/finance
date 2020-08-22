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

    TABLE crvalue {
      uint64_t id;
      string key;
      double value;
      auto primary_key() const { return id; }
    };
    typedef multi_index<name("crvalue"), crvalue> crvalue_table;
};
