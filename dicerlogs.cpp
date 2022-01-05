#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/transaction.hpp>

#define TEAM_DICE N(dicer.e) // dicer.e

using namespace std;
using namespace eosio;

class dicerlogs : public contract {

  public:
    
    struct receipt {
        uint64_t id;                   // Id
        account_name player;           // 下注账号
        account_name referrer;         // 推荐人
        asset bet_amount;              // 下注金额
        asset bet_award;               // 中奖金额
        asset bet_mining;              // 挖矿金额
        time_point_sec bet_time;       // 下注时间
        uint64_t status;               // 订单状态
        uint64_t under_number;         // 小于点数
        uint64_t result_number;        // 结果点数
        checksum256 player_seed;       // 玩家种子
        checksum256 banker_seed;       // 庄家种子
        checksum256 banker_seed_hash;  // 庄家种子hash
    };

    dicerlogs(account_name self):contract(self){}

    //@abi action
    void order(receipt order) {
        require_auth(TEAM_DICE);
        require_recipient(order.player);
    }
};

EOSIO_ABI(dicerlogs, (order));