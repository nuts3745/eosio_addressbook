#include <eosio/eosio.hpp>
#include <eosio/print.hpp>

using namespace eosio;

class [[eosio::contract("addressbook")]] addressbook : public eosio::contract
{
public:
    addressbook(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

    [[eosio::action]] void upsert(name user, std::string first_name, std::string last_name, uint64_t age, std::string street, std::string city, std::string state)
    {
        require_auth(user);
        address_index addresses(get_first_receiver(), get_first_receiver().value);
        auto iterator = addresses.find(user.value);
        if (iterator == addresses.end())
        {
            addresses.emplace(user, [&](auto &row) {
                row.key = user;
                row.first_name = first_name;
                row.last_name = last_name;
                row.age = age;
                row.street = street;
                row.city = city;
                row.state = state;
            });
            send_summary(user, " レコードの登録に成功しました");
        }
        else
        {
            addresses.modify(iterator, user, [&](auto &row) {
                row.key = user;
                row.first_name = first_name;
                row.last_name = last_name;
                row.age = age;
                row.street = street;
                row.city = city;
                row.state = state;
            });
            send_summary(user, " レコードの変更に成功しました");
        }
    }

    [[eosio::action]] void erase(name user)
    {
        require_auth(user);

        address_index addresses(get_self(), get_first_receiver().value);
        auto iterator = addresses.find(user.value);
        check(iterator != addresses.end(), "レコードが存在しません");
        addresses.erase(iterator);
        send_summary(user, " レコードの削除に成功しました");
    }
    [[eosio::action]] void notify(name user, std::string msg)
    {
        require_auth(get_self());
        require_recipient(user);
    }

private:
    struct [[eosio::table]] person
    {
        name key;
        std::string first_name;
        std::string last_name;
        uint64_t age;
        std::string street;
        std::string city;
        std::string state;

        uint64_t primary_key() const { return key.value; }
        uint64_t get_secondary_1() const { return age; }
    };

    void send_summary(name user, std::string message)
    {
        action(
            permission_level{get_self(), "active"_n},
            get_self(),
            "notify"_n,
            std::make_tuple(user, name{user}.to_string() + message))
            .send();
    };

    typedef eosio::multi_index<"people"_n, person, indexed_by<"byage"_n, const_mem_fun<person, uint64_t, &person::get_secondary_1>>> address_index;
};