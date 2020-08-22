#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT foundary : public contract {
  public:
    using contract::contract;

    ACTION starttxn(name from,
      uint64_t id);

    ACTION settle(name from);

    [[eosio::on_notify("brickissuer::transfer")]]
    void receivebrick(name from, name to, asset quantity, string memo);

    [[eosio::on_notify("tethertether::transfer")]]
    void receiveusdt(name from, name to, asset quantity, string memo);

    [[eosio::on_notify("mudissuer::transfer")]]
    void receivemud(name from, name to, asset quantity, string memo);

    [[eosio::on_notify("vigortoken11::transfer")]]
    void receivevigor(name from, name to, asset quantity, string memo);

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


    TABLE bricktxn {
      uint64_t id;
      extended_asset ext_mud_asset;
      extended_asset ext_collateral_asset;
      auto primary_key() const {return id;}
    };
    typedef eosio::multi_index<name("bricktxn"), bricktxn> bt_table;

    void transfer_from_self(name to, extended_asset e_asset, string memo);

    double get_mud_usdt_price();

    double get_collateral_usdt_price(symbol collateral_sym, name collateral_contract);

    double get_cr();

    std::tuple<symbol, name> get_symbol_contract(uint64_t id);

    std::tuple<extended_asset, extended_asset> assets_to_return(asset quantity, symbol collateral_sym, name collateral_contract);

    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> onlycollateral(extended_asset ext_collateral_asset, extended_asset ext_mud_asset);

    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> partcollateral(extended_asset ext_collateral_asset, extended_asset ext_mud_asset, double cr);
};
