#include <vector>
#include <ctype.h>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/transaction.hpp>
#include <boost/functional/hash.hpp>

#define CORE_TOKEN S(4, EOS)         //EAST          EOS
#define CORE_ACCOUNT N(eosio.token) //shadowbanker  eosio.token

#define TEAM_TOKEN S(4, ESA)        //TEST          ESA
#define TEAM_ACCOUNT N(eosiodrizzle)
#define TEAM_SHADOWS N(eosioshadows)
#define TEAM_SERVICE N(shadowserver)
#define TEAM_BANKERS N(shadowbanker)
#define TEAM_BETLOGS  N(dicerlogs.e)
#define TEAM_BLACKHOLE N(blackhole.e)
#define WIN96_MINING N(winninetysix)


#define UTC_BASE_YEAR 1970
#define MONTH_PER_YEAR 12
#define DAY_PER_YEAR 365
#define SEC_PER_DAY 86400
#define SEC_PER_HOUR 3600
#define SEC_PER_MIN 60

using namespace std;
using namespace eosio;

class shadowsdicer : public contract {

  private:

    const unsigned char g_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const string SYSTEM_KEY = "EOS7eEvCovjsN6QzJfhhyu5K6Q5WCeVFfoxEJLFnvNi1ZbXWuFqZp";
    const string SYSTEM_REMARK = "--- 影骰是一款集合（骰子+接龙+竞拍+分红）的公平游戏，采用玩家可验证的算法生成随机数，庄家绝对无法操控骰子点数，官网：http://idice.app。";
    const int8_t BASE58_MAP[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,
    8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
    19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1,
    -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    typedef struct
    {
        unsigned short nYear;
        unsigned char nMonth;
        unsigned char nDay;
        unsigned char nHour;
        unsigned char nMin;
        unsigned char nSec;
        unsigned char DayIndex; /* 0 = Sunday */
    } mytime_struct;

    struct weight{
        uint64_t rank;
        account_name owner;
        EOSLIB_SERIALIZE(weight, (rank)(owner))
    };

    // @abi table settings i64
    struct setting{
        uint64_t id;                  // Id
        uint64_t min_bet;             // 最低下注
        uint64_t max_bet;             // 最高下注
        uint64_t dice_max;            // 骰子点数
        uint64_t dice_edge;           // 庄家优势
        uint64_t dice_water;          // 庄家抽水
        uint64_t is_staking;          // 允许质押
        uint64_t is_mining;           // 允许挖矿
        uint64_t is_betting;          // 允许下注
        uint64_t is_bidding;          // 允许竞拍
        uint64_t team_ratio;          // 平台比例
        uint64_t stake_ratio;         // 质押比例
        uint64_t system_ratio;        // 系统比例
        uint64_t dragon_ratio;        // 接龙比例
        uint64_t lucky_ratio;         // 幸运比例
        uint64_t shadows_ratio;       // 简影比例
        uint64_t referrer_ratio;      // 推荐比例
        uint64_t bidding_ratio;       // 竞拍比例
        uint64_t bidding_losing;      // 竞拍损失
        uint64_t bidding_offset;      // 竞拍时间间隔
        uint64_t bidding_minimum;     // 竞拍最小金额
        uint64_t bidding_increase;    // 竞拍增长比例
        uint64_t bidding_min_bonus;   // 竞拍最大奖励
        uint64_t bidding_max_bonus;   // 竞拍最大奖励
        uint64_t luck_time_offset;    // 接龙权重奖池时间间隔
        uint64_t luck_time_increase;  // 接龙权重时间增加数量
        uint64_t stake_time_offset;   // 质押时间间隔
        uint64_t stake_time_redeem;   // 质押赎回小时
        uint64_t stake_dividend_ratio;// 质押分红比例
        uint64_t hand_card_limit;     // 出牌限额
        uint64_t max_payout_amount;   // 最大赔付金额
        uint64_t mining_supply_amount;// 挖矿供应量


        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(setting, (id)(min_bet)(max_bet)(dice_max)(dice_edge)(dice_water)(is_staking)(is_mining)(is_betting)(is_bidding)(team_ratio)(stake_ratio)(system_ratio)(dragon_ratio)(lucky_ratio)(shadows_ratio)(referrer_ratio)(bidding_ratio)(bidding_losing)(bidding_offset)(bidding_minimum)(bidding_increase)(bidding_min_bonus)(bidding_max_bonus)(luck_time_offset)(luck_time_increase)(stake_time_offset)(stake_time_redeem)(stake_dividend_ratio)(hand_card_limit)(max_payout_amount)(mining_supply_amount))
    };
    typedef multi_index<N(settings), setting> _setting;
    _setting settings;

    // @abi table games i64
    struct game{
        uint64_t id;                  // Id
        uint64_t bid_jackpot;         // 竞拍奖池
        uint64_t locked_balance;      // 锁定金额

        uint64_t stake_time;          // 质押倒计时
        uint64_t stake_owner;         // 质押分红Id
        uint64_t stake_token;         // 质押ESA
        uint64_t stake_token_mirror;  // 质押ESA
        uint64_t stake_jackpot;       // 质押奖池
        uint64_t stake_jackpot_mirror;// 质押奖池镜像

        uint64_t dragon_jackpot;      // 接龙奖池
        vector<weight> dragon_ranks;  // 接龙排行
        uint64_t dragon_lucktime;     // 接龙幸运倒计时
        uint64_t dragon_luckround;    // 接龙幸运场次
        account_name dragon_luckowner;// 接龙幸运获得者
        uint64_t dragon_luckpot;      // 接龙幸运奖池
        uint64_t dragon_luckpot_mirror;// 质押奖池镜像

        uint64_t total_team_fee;      // 平台分红
        uint64_t total_stake_fee;     // 质押分红
        uint64_t total_system_fee;    // 系统分红
        uint64_t total_dragon_fee;    // 接龙分红
        uint64_t total_luck_fee;      // 幸运分红
        uint64_t total_shadows_fee;   // 简影分红
        uint64_t total_bidding_fee;   // 竞拍分红

        uint64_t total_order_count;   // 总注单数
        uint64_t total_player_count;  // 总玩家数
        uint64_t total_dragon_count;  // 接龙数量
        uint64_t total_bidding_count; // 竞拍数量
        uint64_t total_drawing_count; // 总结算数量
        uint64_t total_transfer_count;// 总交易数量

        uint64_t total_order_amount;  // 总下注额
        uint64_t total_mining_amount; // 总挖矿额
        uint64_t total_destroy_amount;// 总销毁矿产
        uint64_t total_supply_amount; // 赔付供应量
        uint64_t total_dividend_amount; // 系统总分红

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(game, (id)(bid_jackpot)(locked_balance)(stake_time)(stake_owner)(stake_token)(stake_token_mirror)(stake_jackpot)(stake_jackpot_mirror)(dragon_jackpot)(dragon_ranks)(dragon_lucktime)(dragon_luckround)(dragon_luckowner)(dragon_luckpot)(dragon_luckpot_mirror)(total_team_fee)(total_stake_fee)(total_system_fee)(total_dragon_fee)(total_luck_fee)(total_shadows_fee)(total_bidding_fee)(total_order_count)(total_player_count)(total_dragon_count)(total_bidding_count)(total_drawing_count)(total_transfer_count)(total_order_amount)(total_mining_amount)(total_destroy_amount)(total_supply_amount)(total_dividend_amount))
    };
    typedef multi_index<N(games), game> _game;
    _game games;
  
    // @abi table biddings i64
    struct bidding{
        uint64_t id;                  // Id
        uint64_t status;              // 竞拍完成
        asset bid_price;              // 竞拍价格
        uint64_t bid_jackpot;         // EOS奖池
        uint64_t open_time;           // 竞拍开始时间
        uint64_t stop_time;           // 竞拍结束时间
        uint64_t first_token;         // 第一名ESA
        account_name first_owner;     // 第一名获胜账号60
        uint64_t second_token;        // 第二名ESA
        account_name second_owner;    // 第二名获胜账号30
        uint64_t third_token;         // 第三名ESA
        account_name third_owner;     // 第三名获胜账号10

        uint64_t primary_key() const { return id; }
        uint64_t time_key() const { return stop_time; }

        EOSLIB_SERIALIZE(bidding, (id)(status)(bid_price)(bid_jackpot)(open_time)(stop_time)(first_token)(first_owner)(second_token)(second_owner)(third_token)(third_owner))
    };
    typedef multi_index<N(biddings), bidding,
        indexed_by<N(time), const_mem_fun<bidding, uint64_t, &bidding::time_key>>
    > _bidding;
    _bidding biddings;  

    // @abi table dragons i64
    struct dragon{
        uint64_t id;                  // Id
        asset price;                  // 骰子价值
        uint64_t number;              // 骰子点数
        account_name owner;           // 下注账号

        uint64_t primary_key() const { return id; }
        uint64_t number_key() const { return number; }
        account_name owner_key() const { return owner; }

        EOSLIB_SERIALIZE(dragon, (id)(price)(number)(owner))
    };
    typedef multi_index<N(dragons), dragon,
        indexed_by<N(number), const_mem_fun<dragon, uint64_t, &dragon::number_key>>,
        indexed_by<N(owner), const_mem_fun<dragon, account_name, &dragon::owner_key>>
    > _dragon;
    _dragon dragons;  

    // @abi table queues i64
    struct queue{
        uint64_t id;                  // Id
        uint64_t start;               // 接龙起始Id
        uint64_t stop;                // 接龙截止Id
        uint64_t count;               // 消牌数量
        uint64_t profit;              // 消牌利润
        uint64_t status;              // 接龙状态
        uint64_t number;              // 接龙点数
        account_name owner;           // 接龙中奖账号

        uint64_t primary_key() const { return id; }
        uint64_t number_key() const { return number; }

        EOSLIB_SERIALIZE(queue, (id)(start)(stop)(count)(profit)(status)(number)(owner))
    };
    typedef multi_index<N(queues), queue,
        indexed_by<N(number), const_mem_fun<queue, uint64_t, &queue::number_key>>
    > _queue;
    _queue queues;  

    // @abi table players i64
    struct player {
        uint64_t id;                  // Id
        account_name owner;           // 账号
        account_name referrer;        // 推荐人
        asset balance;                // 账户余额

        uint64_t bet_step;            // 投注步长  
        uint64_t bet_time;            // 最后下注时间
        uint64_t bet_profit;          // 投注获利
        uint64_t bet_amount;          // 下注金额

        uint64_t stake_time;          // 质押时间
        uint64_t stake_amount;        // 质押数量
        uint64_t stake_profit;        // 质押分红

        uint64_t unstake_id;          // 质押交易id
        uint64_t unstake_time;        // 质押锁定时间
        uint64_t unstake_amount;      // 质押锁定数量

        uint64_t dragon_round;        // 接龙场次
        uint64_t dragon_count;        // 出牌数量
        uint64_t dragon_profit;       // 接龙获利

        uint64_t mining_amount;       // 挖矿数量
        uint64_t referrer_amount;     // 推荐利润

        uint64_t primary_key() const { return id; }
        account_name owner_key() const { return owner; }
        account_name referrer_key() const { return referrer; }
        key256 weight_key() const { return get_weight_key(dragon_round,dragon_count); }

        static key256 get_weight_key(uint64_t dragon_round,uint64_t dragon_count) 
        {
            return key256::make_from_word_sequence<uint64_t>(dragon_round,dragon_count);
        }

        EOSLIB_SERIALIZE(player, (id)(owner)(referrer)(balance)(bet_step)(bet_time)(bet_profit)(bet_amount)(stake_time)(stake_amount)(stake_profit)(unstake_id)(unstake_time)(unstake_amount)(dragon_round)(dragon_count)(dragon_profit)(mining_amount)(referrer_amount))
    };
    typedef multi_index<N(players), player,
        indexed_by<N(owner), const_mem_fun<player, account_name, &player::owner_key>>,
        indexed_by<N(referrer), const_mem_fun<player, account_name, &player::referrer_key>>,
        indexed_by<N(weight), const_mem_fun<player, key256, &player::weight_key>>
    > _player;
    _player players;
    
    // @abi table orders i64
    struct order {
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

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(order, (id)(player)(referrer)(bet_amount)(bet_award)(bet_mining)(bet_time)(status)(under_number)(result_number)(player_seed)(banker_seed)(banker_seed_hash))
    };
    typedef multi_index<N(orders), order> _order;
    _order orders;

    // @abi table seeds i64
    struct seed {
        checksum256 hash;              // 庄家种子hash
        uint64_t expired;              // 过期时间     

        uint64_t primary_key() const { return hash_to_primary(hash); }
        uint64_t expired_key() const { return expired; }

        EOSLIB_SERIALIZE(seed, (hash)(expired))
    };
    typedef multi_index<N(seeds), seed,
    indexed_by<N(expired), const_mem_fun<seed, uint64_t, &seed::expired_key>>
    > _seed;
    _seed seeds;

    // @abi table accounts i64
    struct account {
        asset    balance;
        uint64_t primary_key()const { return balance.symbol.name(); }
    };
    typedef eosio::multi_index<N(accounts), account> accounts;

    
    unsigned char applib_dt_is_leap_year(unsigned short year)
    {
        if ((year % 400) == 0) {
            return 1;
        } else if ((year % 100) == 0) {
            return 0;
        } else if ((year % 4) == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    
    unsigned char applib_dt_last_day_of_mon(unsigned char month, unsigned short year)
    {
        if ((month == 0) || (month > 12)) {
            return g_day_per_mon[1] + applib_dt_is_leap_year(year);
        }
    
        if (month != 2) {
            return g_day_per_mon[month - 1];
        } else {
            return g_day_per_mon[1] + applib_dt_is_leap_year(year);
        }
    }
    
    unsigned char applib_dt_dayindex(unsigned short year, unsigned char month, unsigned char day)
    {
        char century_code, year_code, month_code, day_code;
        int week = 0;
    
        century_code = year_code = month_code = day_code = 0;
    
        if (month == 1 || month == 2) {
            century_code = (year - 1) / 100;
            year_code = (year - 1) % 100;
            month_code = month + 12;
            day_code = day;
        } else {
            century_code = year / 100;
            year_code = year % 100;
            month_code = month;
            day_code = day;
        }
    
        week = year_code + year_code / 4 + century_code / 4 - 2 * century_code + 26 * ( month_code + 1 ) / 10 + day_code - 1;
        week = week > 0 ? (week % 7) : ((week % 7) + 7);
    
        return week;
    }

    void utc_sec_2_mytime(unsigned int utc_sec, mytime_struct *result, bool daylightSaving)
    {
        int sec, day;
        unsigned short y;
        unsigned char m;
        unsigned short d;
        unsigned char dst;
    
    
        if (daylightSaving) {
            utc_sec += SEC_PER_HOUR;
        }
    
        /* hour, min, sec */
        /* hour */
        sec = utc_sec % SEC_PER_DAY;
        result->nHour = sec / SEC_PER_HOUR;
    
        /* min */
        sec %= SEC_PER_HOUR;
        result->nMin = sec / SEC_PER_MIN;
    
        /* sec */
        result->nSec = sec % SEC_PER_MIN;
    
        /* year, month, day */
        /* year */
        /* year */
        day = utc_sec / SEC_PER_DAY;
        for (y = UTC_BASE_YEAR; day > 0; y++) {
            d = (DAY_PER_YEAR + applib_dt_is_leap_year(y));
            if (day >= d)
            {
                day -= d;
            }
            else
            {
                break;
            }
        }
    
        result->nYear = y;
    
        for (m = 1; m < MONTH_PER_YEAR; m++) {
            d = applib_dt_last_day_of_mon(m, y);
            if (day >= d) {
                day -= d;
            } else {
                break;
            }
        }
    
        result->nMonth = m;
        result->nDay = (unsigned char) (day + 1);
        result->DayIndex = applib_dt_dayindex(result->nYear, result->nMonth, result->nDay);
    }

    asset get_payout(const uint64_t& under_number, const asset& quantity,const uint64_t dice_max,const double dice_water) {
        return min(get_max_payout(quantity,under_number,dice_max,dice_water), get_max_bonus());
    }

    asset get_max_payout(const asset& quantity,const uint64_t& under_number,const uint64_t dice_max,const double dice_water) {
        auto odds =(dice_max - dice_max * get_real_amount(dice_water)) / ((double)under_number - 1.0);
        return asset(odds * quantity.amount, c);
    }

    asset get_max_bonus() { 
        //auto amount = get_avaliable_balance(settingitr->locked_balance) / 500;
        //return amount.amount<=500000?asset(500000,CORE_TOKEN):amount;
        auto settingitr = settings.begin();
        return asset(settingitr->max_payout_amount,CORE_TOKEN);
     }

    // asset get_avaliable_balance(const uint64_t locked_balance) {
    //     auto locked = asset(locked_balance, CORE_TOKEN);
    //     auto balance = get_token_balance(CORE_TOKEN);
    //     eosio_assert(balance >= locked, "系统有效赔付余额不足");
    //     return balance - locked;
    // }

    asset get_eos_balance(symbol_type symbol){
        accounts accountstable(CORE_ACCOUNT,_self);
        const auto& ac = accountstable.get(symbol.name());
        return ac.balance;
    }

    asset get_esa_balance(symbol_type symbol){
        accounts accountstable(TEAM_BANKERS,_self);
        const auto& ac = accountstable.get(symbol.name());
        return ac.balance;
    }
    
    void assert_seed(const checksum256& data_seed, const checksum256& hash) {
        string data = sha256_to_hex(data_seed);
        char* chr = const_cast<char*>(data.c_str());
        assert_sha256(chr, strlen(chr), &hash);
    }

    uint64_t get_random(const checksum256& seed1, const checksum256& seed2,const uint64_t dice_max) {
        size_t hashcode = 0;
        hash_merge(hashcode, sha256_to_hex(seed1));
        hash_merge(hashcode, sha256_to_hex(seed2));
        return (hashcode % dice_max) + 1;
    }

    template <class T>inline void hash_merge(std::size_t& data_seed, const T& v) {
        //std::hash<T> hasher;
        boost::hash<std::string> hasher;
        data_seed ^= hasher(v) + 0x9e3779b9 + (data_seed << 6) + (data_seed >> 2);
    }

    string sha256_to_hex(const checksum256& sha256) {
        return hash_to_hex((char*)sha256.hash, sizeof(sha256.hash));
    }

    checksum256 hex_to_sha256(const string& hex_string) {
        eosio_assert(hex_string.length() == 64, "无效的sha256字符串");
        checksum256 checksum;
        from_hex(hex_string, (char*)checksum.hash, sizeof(checksum.hash));
        return checksum;
    }

    checksum256 string_to_sha256(const string data)
    {
        checksum256 hash;
        sha256(const_cast<char*>(data.c_str()), data.size() * sizeof(char), &hash);
        return hash;
    }
    
    size_t string_split(const string& input,string* output,const char& separator,const size_t& first_pos = 0,const bool& required = false) {
        eosio_assert(first_pos != string::npos, "解析MEMO信息失败");
        auto pos = input.find(separator, first_pos);
        if (pos == string::npos) {
            eosio_assert(!required, "解析MEMO信息错误");
            return string::npos;
        }
        *output = input.substr(first_pos, pos - first_pos);
        return pos;
    }

    double get_real_amount(const uint64_t amount){
        return double(amount)/double(10000); 
    }

    bool is_digits(const std::string &str){
        return std::all_of(str.begin(), str.end(), ::isdigit); 
    }

    string hash_to_hex(const char* d, uint32_t s) {
        std::string r;
        const char* to_hex = "0123456789abcdef";
        uint8_t* c = (uint8_t*)d;
        for (uint32_t i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
        return r;
    }

    uint8_t from_hex(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        eosio_assert(false, "无效的hex字符串");
        return 0;
    }

    size_t from_hex(const string& hex_str, char* out_data, size_t out_data_len) {
        auto i = hex_str.begin();
        uint8_t* out_pos = (uint8_t*)out_data;
        uint8_t* out_end = out_pos + out_data_len;
        while (i != hex_str.end() && out_end != out_pos) {
            *out_pos = from_hex((char)(*i)) << 4;
            ++i;
            if (i != hex_str.end()) {
                *out_pos |= from_hex((char)(*i));
                ++i;
            }
            ++out_pos;
        }
        return out_pos - (uint8_t*)out_data;
    }
    
    string uint64_to_string(uint64_t input) {
        string result;
        uint8_t base = 10;
        do {
            char c = input % base;
            input /= base;
            if (c < 10)
                c += '0';
            else
                c += 'A' - 10;
            result = c + result;
        } while (input);
        return result;
    }

    uint64_t hash_to_uint64(const string& hash) {
        return std::hash<string>{}(hash);
    }

    uint64_t hash_to_uint64(const checksum256& hash) {
        return hash_to_uint64(sha256_to_hex(hash));
    }

    static uint64_t hash_to_primary(const checksum256 sha256)
    {
        const char* d = (char*)sha256.hash;
        uint32_t s = sizeof(sha256.hash);
        std::string r;
        const char* to_hex = "0123456789abcdef";
        uint8_t* c = (uint8_t*)d;
        for (uint32_t i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
        return std::hash<string>{}(r);
    }

    bool decode_base58(const string& str, std::vector<unsigned char>& vch) {
        auto psz = str.c_str();
        while (*psz && isspace(*psz)) psz++;
        int zeroes = 0;
        int length = 0;
        while (*psz == '1') {
            zeroes++;
            psz++;
        }
        int size = strlen(psz) * 733 / 1000 + 1;
        std::vector<unsigned char> b256(size);
        static_assert(
            sizeof(BASE58_MAP) / sizeof(BASE58_MAP[0]) == 256,"BASE58_MAP 必须为256位");
        while (*psz && !isspace(*psz)) {
            int carry = BASE58_MAP[(uint8_t)*psz];
            if (carry == -1)
                return false;
            int i = 0;
            for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin();
                (carry != 0 || i < length) && (it != b256.rend());
                ++it, ++i) {
                carry += 58 * (*it);
                *it = carry % 256;
                carry /= 256;
            }
            assert(carry == 0);
            length = i;
            psz++;
        }
        while (isspace(*psz)) psz++;
        if (*psz != 0) return false;
        std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
        while (it != b256.end() && *it == 0) it++;
        vch.reserve(zeroes + (b256.end() - it));
        vch.assign(zeroes, 0x00);
        while (it != b256.end()) vch.push_back(*(it++));
        return true;
    }

    signature string_to_signature(const string& sig, const bool& checksumming = true) {
        const auto pivot = sig.find('_');
        eosio_assert(pivot != string::npos, "签名缺少分隔符");
        const auto prefix_str = sig.substr(0, pivot);
        eosio_assert(prefix_str == "SIG", "签名前缀不正确");
        const auto next_pivot = sig.find('_', pivot + 1);
        eosio_assert(next_pivot != string::npos, "签名缺少曲线");
        const auto curve = sig.substr(pivot + 1, next_pivot - pivot - 1);
        eosio_assert(curve == "K1" || curve == "R1", "签名曲线不正确");
        const bool k1 = curve == "K1";
        auto data_str = sig.substr(next_pivot + 1);
        eosio_assert(!data_str.empty(), "签名数据不正确");
        vector<unsigned char> vch;

        eosio_assert(decode_base58(data_str, vch), "签名解码失败");

        eosio_assert(vch.size() == 69, "签名数据无效");

        if (checksumming) {
            array<unsigned char, 67> check_data;
            copy_n(vch.begin(), 65, check_data.begin());
            check_data[65] = k1 ? 'K' : 'R';
            check_data[66] = '1';

            checksum160 check_sig;
            ripemd160(reinterpret_cast<char*>(check_data.data()), 67, &check_sig);

            eosio_assert(memcmp(&check_sig.hash, &vch.end()[-4], 4) == 0, "签名校验失败");
        }

        signature _sig;
        unsigned int type = k1 ? 0 : 1;
        _sig.data[0] = (uint8_t)type;
        for (int i = 1; i < sizeof(_sig.data); i++) {
            _sig.data[i] = vch[i - 1];
        }
        return _sig;
    }

    public_key string_to_public_key(const string& pubkey, const bool& checksumming = true) {
        string pubkey_prefix("EOS");
        auto base58substr = pubkey.substr(pubkey_prefix.length());
        vector<unsigned char> vch;
        eosio_assert(decode_base58(base58substr, vch), "公钥解码失败");
        eosio_assert(vch.size() == 37, "公钥不正确");
        if (checksumming) {

            array<unsigned char, 33> pubkey_data;
            copy_n(vch.begin(), 33, pubkey_data.begin());

            checksum160 check_pubkey;
            ripemd160(reinterpret_cast<char*>(pubkey_data.data()), 33, &check_pubkey);

            eosio_assert(memcmp(&check_pubkey.hash, &vch.end()[-4], 4) == 0, "公钥校验失败");
        }
        public_key _pub_key;
        unsigned int type = 0;
        _pub_key.data[0] = (char)type;
        for (int i = 1; i < sizeof(_pub_key.data); i++) {
            _pub_key.data[i] = vch[i - 1];
        }
        return _pub_key;
    }

    uint64_t get_next_transfer_id()
    {
        auto gameitr = games.begin();
        games.modify(gameitr, 0, [&](auto &s) {
            s.total_transfer_count++;
        });
        return gameitr->total_transfer_count;
    }

    uint64_t get_next_dragon_id()
    {
        auto gameitr = games.begin();
        games.modify(gameitr, 0, [&](auto &s) {
            s.total_dragon_count++;
        });
        return gameitr->total_dragon_count;
    }
  
    uint64_t get_next_bidding_id()
    {
        auto gameitr = games.begin();
        games.modify(gameitr, 0, [&](auto &s) {
            s.total_bidding_count++;
        });
        return gameitr->total_bidding_count;
    }

    void delay_transfer(account_name contract,account_name to,asset quantity,string memo)
    {
        memo.append(SYSTEM_REMARK);
        transaction tran; 
        tran.actions.emplace_back(permission_level{_self, N(active)}, contract, N(transfer), std::make_tuple(_self,to, quantity, memo)); 
        tran.delay_sec = 0; 
        tran.send(get_next_transfer_id(), _self, false); 
    }
  
    uint64_t get_next_drawing_id()
    {
        auto gameitr = games.begin();
        games.modify(gameitr, 0, [&](auto &s) {
            s.total_drawing_count++;
        });
        return gameitr->total_drawing_count;
    }    

    void check_signature(uint64_t under_number,uint64_t expiration,account_name referrer,checksum256 banker_seed_hash,signature banker_seed_signature)
    {
        string data = to_string(under_number);
        data += "+";
        data += to_string(expiration);
        data += "+";
        data += name{referrer}.to_string();
        data += "+";
        data += sha256_to_hex(banker_seed_hash);
        checksum256 hashcode;
        sha256(const_cast<char*>(data.c_str()), data.size() * sizeof(char), &hashcode);
        public_key key = string_to_public_key(SYSTEM_KEY);
        assert_recover_key(&hashcode,(char*)&banker_seed_signature.data,sizeof(banker_seed_signature.data),(char*)&key.data,sizeof(key.data));
    }

    void parse_data(asset quantity,string memo,uint64_t* under_number,uint64_t* expiration,account_name* referrer,checksum256* player_seed_hash,checksum256* banker_seed_hash,signature* banker_seed_signature,uint64_t* partner)
    {
        eosio_assert(quantity.amount<=100000, "超出系统最大限额10EOS" );

        size_t postion;
        string data;
        auto pluscount = count(memo.begin(),memo.end(),'+');
        eosio_assert(pluscount==6, "缺少加号分隔符");
        memo.erase(std::remove_if(memo.begin(),memo.end(),[](unsigned char x) { return std::isspace(x); }),memo.end());

        // 获取骰子点数
        postion = string_split(memo, &data, '+', 0, true);
        eosio_assert(data.size()>0, "骰子点数不能为空");
        eosio_assert(is_digits(data), "骰子点数必须是数字");
        *under_number = stoull(data);

        // 获取超时时间
        postion = string_split(memo, &data, '+', ++postion, true);
        eosio_assert(data.size()>0, "超时时间不能为空");
        eosio_assert(is_digits(data), "超时时间必须是数字");
        *expiration = stoull(data);

        // 获取推荐人
        postion = string_split(memo, &data, '+', ++postion, true);
        eosio_assert(data.size()>0, "推荐人不能为空");
        *referrer = string_to_name(data.c_str());

        // 获取玩家种子
        postion = string_split(memo, &data, '+', ++postion, true);
        eosio_assert(data.size()>0, "玩家种子不能为空");
        *player_seed_hash = hex_to_sha256(data);

        // 获取庄家种子
        postion = string_split(memo, &data, '+', ++postion, true);
        eosio_assert(data.size()>0, "庄家种子不能为空");
        *banker_seed_hash = hex_to_sha256(data);

        // 获取庄家种子签名
        postion = string_split(memo, &data, '+', ++postion, true);
        eosio_assert(data.size()>0, "庄家种子签名不能为空");
        *banker_seed_signature = string_to_signature("SIG_K1_"+data);

        // 合作商参数
        data = memo.substr(++postion);
        eosio_assert(data.size()>0, "合作商参数不能为空");
        *partner = stoull(data);
    }

    void verify_data(account_name from,asset quantity, uint64_t under_number,uint64_t expiration,account_name referrer,checksum256 banker_seed_hash,signature banker_seed_signature)
    {
        // 验证下注金额
        auto gameitr = games.begin();
        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_betting==1, "下注功能未启用" ); 
        eosio_assert(gameitr!=games.end(), "游戏尚未初始化" );       
        eosio_assert(quantity.amount>=settingitr->min_bet, "金额至少为0.1EOS" );
        eosio_assert(quantity.amount<=settingitr->max_bet, "金额超出最大限额" );
        eosio_assert(now()<expiration, "随机种子已超时");

        // 验证最大赔付
        auto max_bonus = get_max_bonus();
        auto max_payout = get_max_payout(quantity,under_number,settingitr->dice_max,settingitr->dice_water);
        eosio_assert(max_payout<=max_bonus,"下注金额超出系统最大限额");
        eosio_assert(gameitr->total_supply_amount>=gameitr->locked_balance && ((gameitr->total_supply_amount-gameitr->locked_balance)  >= max_payout.amount),"下注金额超出系统最大赔付额");        

        // 验证庄家种子
        auto seed_key = hash_to_primary(banker_seed_hash);
        auto seeditr = seeds.find(seed_key);
        eosio_assert(seeditr == seeds.end(), "庄家随机种子已被使用");

        // 验证推荐人、骰子点数、种子超时、种子签名
        eosio_assert(referrer!=from,"推荐人不能是自己");
        eosio_assert(is_account(referrer),"推荐人账号不存在");
        auto max_dice_number = settingitr->dice_max * get_real_amount(settingitr->dice_edge) + 1;
        eosio_assert(under_number>=2 && under_number<=max_dice_number, "骰子点数超出范围");
        check_signature(under_number,expiration,referrer,banker_seed_hash,banker_seed_signature);
    }

    void write_bet(account_name from,asset quantity, uint64_t under_number,uint64_t expiration,account_name referrer,checksum256 player_seed_hash,checksum256 banker_seed_hash,uint64_t partner)
    {
        // 计算挖矿数量
        auto gameitr = games.begin();        
        auto settingitr = settings.begin();
        auto esa_balance = get_esa_balance(TEAM_TOKEN);
        auto bet_amount = double(quantity.amount);
        auto bet_mining = settingitr->is_mining==1?((double(esa_balance.amount) / double(settingitr->mining_supply_amount)) * bet_amount * 20):0;  //auto mining_count = esa_balance.amount * bet_amount / (10000000000UL + gameitr->total_order_amount + bet_amount);

        // 计算分红比例
        auto system_fee = bet_amount * get_real_amount(settingitr->system_ratio);
        auto team_fee = system_fee * get_real_amount(settingitr->team_ratio);
        auto stake_fee = system_fee * get_real_amount(settingitr->stake_ratio);
        auto dragon_fee = system_fee * get_real_amount(settingitr->dragon_ratio);
        auto luck_fee = system_fee * get_real_amount(settingitr->lucky_ratio);
        auto shadows_fee = system_fee * get_real_amount(settingitr->shadows_ratio);
        auto bidding_fee = system_fee * get_real_amount(settingitr->bidding_ratio);
        auto referrer_fee = system_fee * get_real_amount(settingitr->referrer_ratio);
        auto left_fee = (quantity.amount>=system_fee? quantity.amount - system_fee:0);

        // 写入账号数据
        auto is_new_player = false;
        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        if(playeritr == playeridx.end())
        {
           players.emplace(_self,[&](auto & s){
                s.id = players.available_primary_key();
                s.owner = from;           
                s.balance = asset(0,CORE_TOKEN);
                s.mining_amount = bet_mining;
                s.bet_amount = bet_amount;
                s.bet_time = now();
                if(s.dragon_round != gameitr->dragon_luckround)
                {
                    s.dragon_count = 0;
                }
                s.dragon_round = gameitr->dragon_luckround;
            });
            is_new_player = true;
        }else{
            playeridx.modify(playeritr, 0, [&](auto &s) {
                s.mining_amount += bet_mining;
                s.bet_amount += bet_amount;
                s.bet_time = now();
                if(s.dragon_round != gameitr->dragon_luckround)
                {
                    s.dragon_count = 0;
                }
                s.dragon_round = gameitr->dragon_luckround;
            });
        }

        //写入种子
        seeds.emplace(_self,[&](auto & s){
            s.hash = banker_seed_hash;
            s.expired = expiration;
        });

        // 推荐人分红
        auto referrer_esa_fee = bet_mining * get_real_amount(settingitr->referrer_ratio);
        if (referrer>0 && referrer != _self && referrer_esa_fee>0) {     
            auto referreritr = playeridx.find(referrer);
            if(referreritr != playeridx.end())
            {
                playeridx.modify(referreritr, 0, [&](auto &s) {
                    s.referrer_amount += referrer_esa_fee;
                    s.mining_amount += referrer_esa_fee;
                });
            }else{
                players.emplace(_self,[&](auto & s){
                    s.id = players.available_primary_key();
                    s.owner = referrer;           
                    s.balance = asset(0,CORE_TOKEN);
                    s.mining_amount = referrer_esa_fee;
                    s.referrer_amount += referrer_esa_fee;
                });
            }
        }

        // 写入统计数据
        games.modify(gameitr, 0, [&](auto &s) {
            s.bid_jackpot += bidding_fee;
            s.stake_jackpot += stake_fee;      
            s.dragon_jackpot += dragon_fee;    
            s.dragon_luckpot += luck_fee;
            s.locked_balance += bet_amount;      
            s.total_team_fee += team_fee;
            s.total_stake_fee += stake_fee;
            s.total_system_fee += system_fee;
            s.total_dragon_fee += dragon_fee;
            s.total_luck_fee += luck_fee;
            s.total_shadows_fee += shadows_fee;
            s.total_bidding_fee += bidding_fee;
            s.total_order_count += 1;
            s.total_player_count += (is_new_player?1:0);
            s.total_order_amount += bet_amount;
            s.total_mining_amount += bet_mining + referrer_esa_fee;
            s.total_supply_amount += left_fee + referrer_fee;
        });

        // 写入订单数据
        orders.emplace(_self,[&](auto & s){
            s.id = gameitr->total_order_count;
            s.player = from;
            s.referrer = referrer;//(is_new_player == true?referrer:playeritr->referrer);
            s.bet_amount = quantity;
            s.bet_award = asset(0,CORE_TOKEN);
            s.bet_mining = asset(bet_mining,TEAM_TOKEN);
            s.status = 0;
            s.bet_time = time_point_sec(now());
            s.under_number = under_number;
            s.result_number = 0;
            s.player_seed = player_seed_hash;
            s.banker_seed_hash = banker_seed_hash;
        });
        
        // 获得下注挖矿
        if(bet_mining>0)
        {      
            transaction transfer;
            string remark = "获得投注矿产";
            remark.append(SYSTEM_REMARK);
            asset miningfee(bet_mining,TEAM_TOKEN);
            if(partner==1)
            {
                transfer.actions.emplace_back(permission_level {_self, N(active) }, TEAM_BANKERS, N(transfer), std::make_tuple(_self, WIN96_MINING, referrer_esa_fee, remark));
            }
            transfer.actions.emplace_back(permission_level {_self, N(active) }, TEAM_BANKERS, N(transfer), std::make_tuple(_self, from, miningfee, remark));
            transfer.send(get_next_transfer_id(), _self, false); 
        }

    }

    void bet(account_name from, account_name to, asset quantity, string memo)
    {
        uint64_t under_number;
        uint64_t expiration;
        account_name referrer;
        checksum256 player_seed_hash;
        checksum256 banker_seed_hash;
        signature banker_seed_signature;
        uint64_t partner;

        // 提取注单数据
        parse_data(quantity,memo,&under_number,&expiration,&referrer,&player_seed_hash,&banker_seed_hash,&banker_seed_signature,&partner);

        // 检查金额、随机种子、种子签名
        verify_data(from,quantity,under_number,expiration,referrer,banker_seed_hash,banker_seed_signature);

        // 保存注单信息、锁定金额、发送矿产
        write_bet(from,quantity,under_number,expiration,referrer,player_seed_hash,banker_seed_hash,partner);
    }

    void stake(account_name from, asset quantity) {
        require_auth(from);

        auto gameitr = games.begin();
        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_staking==1, "质押功能未启用" ); 
        eosio_assert(quantity.amount>=settingitr->min_bet, "金额至少为0.1" );
        eosio_assert(quantity.amount<=settingitr->max_bet, "金额超出最大限额" );
       
        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end() , "下注后才能质押" );

        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.stake_amount += quantity.amount;
            s.stake_time = now();
        });

        games.modify(gameitr,0,[&](auto &s){
            s.stake_token += quantity.amount;
        }); 
    }

    void bid(account_name from, asset quantity) {
        require_auth(from);

        mytime_struct now_time;
        utc_sec_2_mytime(now() + 8 * SEC_PER_HOUR, &now_time, false);
        if(now_time.nHour<9 || now_time.nHour>24)
        {
            eosio_assert(false, "竞拍功能只在9:00-24:00开放" );
        }

        auto gameitr = games.begin();
        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_bidding==1, "竞拍功能未启用" ); 
        eosio_assert(quantity.amount>=settingitr->min_bet, "金额至少为0.1" );
        eosio_assert(quantity.amount<=settingitr->max_bet, "金额超出最大限额" );   

        auto bid_amount = gameitr->bid_jackpot * get_real_amount(settingitr->bidding_min_bonus);
        eosio_assert(bid_amount>0, "奖池中资金不足" ); 

        auto rbiditr = biddings.rbegin();
        if(rbiditr==biddings.rend() || (now()>rbiditr->stop_time && rbiditr->stop_time>0))
        {
            eosio_assert(quantity.amount>=settingitr->bidding_minimum, "不能低于最低竞拍限额");
            biddings.emplace(_self,[&](auto & s){
                s.id = get_next_bidding_id();
                s.first_owner = from;
                s.first_token = quantity.amount;
                s.status=0;
                s.bid_price = quantity;
                s.open_time = now();
                s.stop_time = now() + settingitr->bidding_offset;
            });

        }else
        {
            auto is_remove_third = false;
            auto biditr = biddings.find(rbiditr->id);
            eosio_assert(biditr!=biddings.end(), "竞拍信息不存在" );
            auto return_destroy = biditr->third_token * get_real_amount(settingitr->bidding_losing);
            auto return_token = biditr->third_token - return_destroy;
            auto return_account = biditr->third_owner;

            auto bid_price = biditr->bid_price.amount+biditr->bid_price.amount*get_real_amount(settingitr->bidding_increase);
            biddings.modify(biditr, 0, [&](auto &s) 
            {
                s.stop_time = now() + settingitr->bidding_offset;
            
                if(from == s.first_owner)
                {
                    auto total_price = s.first_token + quantity.amount;
                    eosio_assert(total_price>=bid_price, "不能低于竞拍金额的1.1倍"); 
                    s.first_token = total_price;
                    s.bid_price = asset(total_price,TEAM_TOKEN);
                }
                else if(from == s.second_owner)
                {
                    auto total_price = s.second_token + quantity.amount;
                    eosio_assert(total_price>=bid_price, "不能低于竞拍金额的1.1倍"); 
                    s.second_token = total_price;
                    s.bid_price = asset(total_price,TEAM_TOKEN);

                    s.second_owner = s.first_owner;
                    s.second_token = s.first_token;

                    s.first_owner = from;
                    s.first_token = total_price;
                }
                else if(from == s.third_owner)
                {
                    auto total_price = s.third_token + quantity.amount;
                    eosio_assert(total_price>=bid_price, "不能低于竞拍金额的1.1倍"); 

                    s.third_token = total_price;
                    s.bid_price = asset(total_price,TEAM_TOKEN);

                    s.third_owner = s.second_owner;
                    s.third_token = s.second_token;

                    s.second_owner = s.first_owner;
                    s.second_token = s.first_token;

                    s.first_owner = from;
                    s.first_token = total_price;
                }
                else
                {
                    eosio_assert(quantity.amount>=settingitr->bidding_minimum, "不能低于最低竞拍限额");
                    eosio_assert(quantity.amount>=bid_price, "不能低于竞拍金额的1.1倍");
                    
                    is_remove_third=true;

                    s.bid_price = asset(quantity.amount,TEAM_TOKEN);

                    s.third_owner = s.second_owner;
                    s.third_token = s.second_token;

                    s.second_owner = s.first_owner;
                    s.second_token = s.first_token;

                    s.first_owner = from;
                    s.first_token = quantity.amount;
                }

            });

            //退换部分竞拍资金、销毁ESA
            if(is_remove_third==true)
            {
                if(return_account>0 && return_token>0)
                {
                    string return_remark = "退还竞拍质押金";
                    return_remark.append(SYSTEM_REMARK);
                    asset returnfee(return_token,TEAM_TOKEN);
                    action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, return_account, returnfee, return_remark)).send();
                }
                
                if(return_destroy>0)
                {
                    games.modify(gameitr, 0, [&](auto &s) {
                        s.total_destroy_amount+=return_destroy;
                    });
                    string destroy_remark = "销毁竞拍质押金";
                    destroy_remark.append(SYSTEM_REMARK);
                    asset destroyfee(return_destroy,TEAM_TOKEN);
                    action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, TEAM_BLACKHOLE, destroyfee, destroy_remark)).send();
                }
            }
        }  
    }


    void initialize()
    {
      auto settingitr = settings.begin();
      if(settingitr==settings.end())
      {
        settingitr = settings.emplace(_self,[&](auto & s){
            s.id = 0;
            s.min_bet = 1000;
            s.max_bet = 1000000000000;

            s.dice_max = 500;
            s.dice_edge = 9500;
            s.dice_water = 200;

            s.is_staking = 0;
            s.is_mining = 0;
            s.is_betting = 0;
            s.is_bidding = 0;

            s.team_ratio = 1000;
            s.stake_ratio = 4000;
            s.system_ratio = 200;
            s.dragon_ratio = 1800;
            s.lucky_ratio = 700;
            s.shadows_ratio = 1000;
            s.bidding_ratio = 1000;
            s.referrer_ratio = 500;
            
            s.bidding_losing = 2000;
            s.bidding_offset = 300;
            s.bidding_minimum = 1000000;
            s.bidding_increase = 1000;
            s.bidding_min_bonus = 500;
            s.bidding_max_bonus = 200000;

            s.luck_time_offset = 86400;
            s.luck_time_increase = 600;

            s.stake_time_offset = 21600;
            s.stake_time_redeem = 86400;
            s.stake_dividend_ratio = 250;

            s.hand_card_limit = 5000000;
            s.max_payout_amount = 500000;
            s.mining_supply_amount = 8000000000000;
        });
      }

      auto gameitr = games.begin();
      if(gameitr==games.end())
      {
        games.emplace(_self,[&](auto & s){
            s.id = 0;
            s.dragon_lucktime = now()+settingitr->luck_time_offset;
            s.stake_time = now()+settingitr->stake_time_offset;
            s.total_drawing_count = 100000000000000;
            s.total_transfer_count = 300000000000000;
            s.total_dragon_count = 500000000000000;
            s.total_bidding_count = 700000000000000;
            s.total_mining_amount = 0;
            s.total_supply_amount = 50000000;
        });
      }
    }

  public:

    shadowsdicer(account_name self):contract(self),
    games(_self, _self),
    queues(_self, _self),
    orders(_self, _self),
    seeds(_self, _self),
    players(_self, _self),
    dragons(_self, _self),
    biddings(_self, _self),
    settings(_self, _self)
    {
      initialize();
    }

    //@abi action
    void transfer(account_name from, account_name to, asset quantity, string memo ) 
    {
        require_auth( from ); 

        if(quantity.is_valid() && quantity.amount > 0 && from != _self && to == _self && from!=N(eosio.ram) && from!=N(eosio.stake) && from!=TEAM_SHADOWS)
        {
            if(quantity.symbol == TEAM_TOKEN)
            {
                if(memo=="BID")
                {
                    bid(from,quantity);
                }
                else if(memo=="STAKE")
                {
                    stake(from,quantity);
                }
            }
            else if(quantity.symbol == CORE_TOKEN)
            {
                bet(from,to,quantity,memo);
            }
        }
    }

    //@abi action
    void stakeclaim(account_name from) {
        require_auth(from);

        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_staking==1, "质押功能未启用" ); 

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end(), "质押账号不存在" );
        eosio_assert(playeritr->stake_profit>0, "没有足够的收益" );

        string remark = "获得质押分红";
        remark.append(SYSTEM_REMARK);
        asset dividendfee(playeritr->stake_profit,CORE_TOKEN);
        action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, from, dividendfee, remark)).send();

        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.stake_profit = 0;
        });
    }
    
    //@abi action
    void unstake(account_name from,asset quantity) {
        require_auth(from);

        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_staking==1, "质押功能未启用" ); 
        eosio_assert(quantity.amount>=settingitr->min_bet, "金额至少为0.1" );
        eosio_assert(quantity.amount<=settingitr->max_bet, "金额超出最大限额" ); 

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end() , "账户信息不存在" );
        eosio_assert(playeritr->stake_amount>=quantity.amount, "没有足够可赎回金额" ); 

        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.stake_amount -= quantity.amount;
        });

        auto gameitr = games.begin();
        auto balance = quantity.amount * double(0.5);
        if(gameitr->stake_token>=quantity.amount)
        {
            games.modify(gameitr,0,[&](auto &s){
                s.stake_token -= quantity.amount;
                s.total_destroy_amount += balance;
            }); 
        }else
        {
            games.modify(gameitr,0,[&](auto &s){
                s.stake_token = 0;
                s.total_destroy_amount += balance;
            }); 
        }

        if(balance>0)
        {
            string stake_remark = "获得立即赎回质押金";
            stake_remark.append(SYSTEM_REMARK);
            asset stakefee(balance,TEAM_TOKEN);
            action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, from, stakefee, stake_remark)).send();
            
            string destroy_remark = "销毁立即赎回质押金";
            destroy_remark.append(SYSTEM_REMARK);
            asset destroyfee(balance,TEAM_TOKEN);
            action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, TEAM_BLACKHOLE, destroyfee,destroy_remark )).send();
        }
    }

    //@abi action
    void unstakedelay(account_name from,asset quantity) {
        require_auth(from);

        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_staking==1, "质押功能未启用" ); 
        eosio_assert(quantity.amount>=settingitr->min_bet, "金额至少为0.1" );
        eosio_assert(quantity.amount<=settingitr->max_bet, "金额超出最大限额" ); 

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end() , "账户信息不存在" );
        eosio_assert(playeritr->stake_amount>=quantity.amount, "没有足够可赎回金额" );    

        cancel_deferred(from);

        auto unstake_time = now() + settingitr->stake_time_redeem;
        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.stake_amount -= quantity.amount;
            if(now()>playeritr->unstake_time)
            {
                s.unstake_amount = quantity.amount;
            }else
            {
                s.unstake_amount += quantity.amount;
            }
            s.unstake_time = unstake_time;
        });

        auto gameitr = games.begin();
        games.modify(gameitr,0,[&](auto &s){
            s.stake_token -= quantity.amount;
        }); 

        transaction transfer;    
        string remark ="赎回质押代币";
        remark.append(SYSTEM_REMARK);
        asset stakefee(quantity.amount,TEAM_TOKEN);
        transfer.actions.emplace_back(permission_level {_self, N(active) }, TEAM_BANKERS, N(transfer), std::make_tuple(_self, from, stakefee, remark));
        transfer.delay_sec = settingitr->stake_time_redeem+30;
        transfer.send(from, _self, false); 
    }

    //@abi action
    void unstakeoff(account_name from) {
        require_auth(from);

        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );
        eosio_assert(settingitr->is_staking==1, "质押功能未启用" ); 

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end() , "账户信息不存在" );  
        eosio_assert(playeritr->unstake_amount>0, "没有可撤销的质押金" );  
        eosio_assert(playeritr->unstake_time>now(), "已超出可撤销时间" );

        cancel_deferred(from);

        auto gameitr = games.begin();
        games.modify(gameitr,0,[&](auto &s){
            s.stake_token += playeritr->unstake_amount;
        }); 

        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.stake_amount += playeritr->unstake_amount;
            s.unstake_time = 0;
            s.unstake_amount = 0;
        });

    } 

    //@abi action
    void stakebonus() {
        require_auth(TEAM_SERVICE);  
        
        auto counter = 0UL;
        auto gameitr = games.begin();
        auto playeritr = players.begin();
        auto settingitr = settings.begin();
        eosio_assert(now()>=gameitr->stake_time, "尚未到达质押分红时间");

        if(gameitr->stake_jackpot==0 && gameitr->stake_jackpot_mirror==0)
        {
            games.modify(gameitr,0,[&](auto &s){
                s.stake_time = now()+settingitr->stake_time_offset;
            }); 
        }else
        {
            if(gameitr->stake_owner>0)
            {
                playeritr = players.lower_bound(gameitr->stake_owner);
            }

            if(gameitr->stake_jackpot_mirror==0)
            {
                auto today_amount = gameitr->stake_jackpot * get_real_amount(settingitr->stake_dividend_ratio);
                games.modify(gameitr,0,[&](auto &s){
                    s.stake_token_mirror = gameitr->stake_token;
                    s.stake_jackpot_mirror =today_amount ;
                    if(gameitr->stake_jackpot>=today_amount)
                    {
                        s.stake_jackpot -= today_amount;
                    }else
                    {
                        s.stake_jackpot = 0;
                    }
                    s.total_dividend_amount += today_amount;
                    s.stake_time = now()+settingitr->stake_time_offset;
                }); 
            }

            for(;playeritr!=players.end();++playeritr)
            {
                if((playeritr->id>0 && playeritr->id == gameitr->stake_owner) || playeritr->stake_amount==0) continue;

                auto ratio = double(playeritr->stake_amount)/double(gameitr->stake_token_mirror);
                auto bonus = gameitr->stake_jackpot_mirror * ratio;
                players.modify( playeritr,0, [&]( auto& s ) {
                    s.stake_profit += bonus;
                });
                if(counter>150)
                {
                    games.modify( gameitr,0, [&]( auto& s ) {
                        s.stake_owner = playeritr->id;
                    });
                    break;
                }
                counter++;   
            }
            
            if(playeritr==players.end())
            {
                games.modify( gameitr,0, [&]( auto& s ) {
                    s.stake_owner = 0;
                    s.stake_token_mirror = 0;
                    s.stake_jackpot_mirror = 0;
                });
            }
        }

    }

    //@abi action
    void bidbonus(uint64_t bid_id) {
        require_auth(TEAM_SERVICE);  

        auto gameitr = games.begin();
        auto settingitr = settings.begin();
        auto biditr = biddings.find(bid_id);
        eosio_assert(biditr!=biddings.end(), "竞拍ID不存在");  
        eosio_assert(now()>=biditr->stop_time && biditr->stop_time>0, "尚未到达结算时间");
        eosio_assert(biditr->status==0, "本场竞拍已经结算");

        auto first_bonus = 0UL; 
        auto second_bonus = 0UL;
        auto third_bonus = 0UL;
        auto is_first_owner = is_account(biditr->first_owner);
        auto is_second_owner = is_account(biditr->second_owner);
        auto is_third_owner = is_account(biditr->third_owner);
        auto bid_amount = gameitr->bid_jackpot * get_real_amount(settingitr->bidding_min_bonus);
        if(is_third_owner)
        {
            first_bonus = bid_amount * double(0.6);
            second_bonus = bid_amount * double(0.3);
            third_bonus = bid_amount * double(0.1);
        }else if(is_second_owner)
        {
            first_bonus = bid_amount * double(0.7);
            second_bonus = bid_amount * double(0.3);
        }else if(is_first_owner)
        {
            first_bonus = bid_amount;
        }

        biddings.modify(biditr, 0, [&](auto &s) {
            s.status =2;
            s.bid_jackpot = bid_amount;
        });

        games.modify(gameitr, 0, [&](auto &s) {
            if(gameitr->bid_jackpot >= bid_amount)
            {
                s.bid_jackpot-=bid_amount;
            }else
            {
                s.bid_jackpot=0;
            }
            s.total_dividend_amount += bid_amount;
        });

        string remark = "获得竞拍奖金";
        remark.append(SYSTEM_REMARK);
        if(is_first_owner && first_bonus>0)
        {
            asset first_fee(first_bonus,CORE_TOKEN);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, biditr->first_owner, first_fee, remark)).send();
        }

        if(is_second_owner && second_bonus>0)
        {    
            asset second_fee(second_bonus,CORE_TOKEN);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, biditr->second_owner, second_fee, remark)).send();
        }
        
        if(is_third_owner && third_bonus>0)
        {     
            asset third_fee(third_bonus,CORE_TOKEN);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, biditr->third_owner, third_fee, remark)).send();
        }
        
        auto destroy_token = biditr->first_token+biditr->second_token+biditr->third_token;
        if(destroy_token>0)
        { 
            games.modify(gameitr, 0, [&](auto &s) {
                s.total_destroy_amount+=destroy_token;
            });

            string destroy_remark = "销毁竞拍资金";
            destroy_remark.append(SYSTEM_REMARK);
            asset destroy_fee(destroy_token,TEAM_TOKEN);
            action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, TEAM_BLACKHOLE, destroy_fee, destroy_remark)).send(); 
        }

        // 移除过期竞拍
        auto counter = 0;
        auto now_time = now()-600;
        auto cleanitr = biddings.begin();
        while (cleanitr!=biddings.end() && now_time>cleanitr->stop_time && cleanitr->status==2) {
            if(counter >= 3)break;
            cleanitr = biddings.erase(cleanitr);
            counter++;
        }

    }

    //@abi action
    void luckbonus()
    {
        require_auth(TEAM_SERVICE);
        
        auto gameitr = games.begin();
        auto settingitr = settings.begin();
        eosio_assert(now()>=gameitr->dragon_lucktime, "尚未到达幸运分红时间");

        if(gameitr->dragon_luckpot==0 && gameitr->dragon_luckpot_mirror==0)
        {
            games.modify(gameitr,0,[&](auto &s){
                s.dragon_luckround +=1;
                s.dragon_ranks.clear();
                s.dragon_lucktime = now()+settingitr->luck_time_offset;
            }); 
        }else
        {
            if(gameitr->dragon_luckpot_mirror==0)
            {
                games.modify(gameitr,0,[&](auto &s){
                    s.dragon_luckpot_mirror = gameitr->dragon_luckpot; 
                    s.dragon_luckpot = 0;               
                    s.dragon_lucktime = now()+settingitr->luck_time_offset;
                    s.total_dividend_amount += gameitr->dragon_luckpot;
                }); 
            }

            transaction transfer;
            string remark ="接龙幸运奖池分红";
            remark.append(SYSTEM_REMARK);
            auto playeridx = players.template get_index<N(weight)>();
            auto playeritr = playeridx.rbegin();
            if(playeritr != playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto first_owner = playeritr->owner;
                auto first_fee = asset((gameitr->dragon_luckpot_mirror * 0.24),CORE_TOKEN);
                if(first_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, first_owner, first_fee, remark));
                }
                playeritr++;
            }

            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto second_owner = playeritr->owner;
                auto second_fee = asset((gameitr->dragon_luckpot_mirror * 0.12),CORE_TOKEN);
                if(second_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, second_owner, second_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto third_owner = playeritr->owner;
                auto third_fee = asset((gameitr->dragon_luckpot_mirror * 0.06),CORE_TOKEN);
                if(third_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, third_owner, third_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto fourth_owner = playeritr->owner;
                auto fourth_fee = asset((gameitr->dragon_luckpot_mirror * 0.03),CORE_TOKEN);
                if(fourth_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, fourth_owner, fourth_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto fifth_owner = playeritr->owner;
                auto fifth_fee = asset((gameitr->dragon_luckpot_mirror * 0.02),CORE_TOKEN);
                if(fifth_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, fifth_owner, fifth_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto sixth_owner = playeritr->owner;
                auto sixth_fee = asset((gameitr->dragon_luckpot_mirror * 0.01),CORE_TOKEN);
                if(sixth_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, sixth_owner, sixth_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto seventh_owner = playeritr->owner;
                auto seventh_fee = asset((gameitr->dragon_luckpot_mirror * 0.01),CORE_TOKEN);
                if(seventh_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, seventh_owner, seventh_fee, remark));
                }
                playeritr++;
            }
            
            if(playeritr!=playeridx.rend() && playeritr->dragon_round==gameitr->dragon_luckround)
            {
                auto eighth_owner = playeritr->owner;
                auto eighth_fee = asset((gameitr->dragon_luckpot_mirror * 0.01),CORE_TOKEN);
                if(eighth_fee.amount>0)
                {
                    transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, eighth_owner, eighth_fee, remark));
                }
            }

            auto last_owner_fee = asset((gameitr->dragon_luckpot_mirror * 0.5),CORE_TOKEN);
            if(last_owner_fee.amount>0)
            {
                transfer.actions.emplace_back(permission_level {_self, N(active) }, CORE_ACCOUNT, N(transfer), std::make_tuple(_self, gameitr->dragon_luckowner, last_owner_fee, remark));
            }

            if(transfer.actions.size()>0)
            {
                transfer.send(get_next_transfer_id(), _self, false);
            }

            games.modify(gameitr,0,[&](auto &s){
                s.dragon_luckround +=1;
                s.dragon_ranks.clear(); 
                s.dragon_luckpot_mirror = 0;
            }); 

        }
        
    }

    //@abi action
    void dragonbonus(uint64_t id){
        require_auth(TEAM_SERVICE);  
        
        auto gameitr = games.begin();
        auto queueitr = queues.find(id);
        auto settingitr = settings.begin();
        eosio_assert( queueitr->status==0, "此牌已经被消灭" );
        eosio_assert( queueitr!=queues.end(), "消牌Id不存在" );
        eosio_assert( gameitr->dragon_jackpot>0, "接龙奖池不能为0" );

        auto counter = 0UL;
        auto is_clean = true;
        auto dragonitr = dragons.lower_bound(queueitr->start);
        while( dragonitr != dragons.end() && dragonitr->id>=queueitr->start && dragonitr->id<=queueitr->stop) 
        {
            if(counter>=100)
            {
                queues.modify( queueitr,0, [&]( auto& s ) {
                    s.start = dragonitr->id;
                });
                is_clean = false;
                break;
            }
            counter++;
            dragonitr = dragons.erase(dragonitr);
        }

        if(counter>0)
        {
            auto profit = counter * (double(gameitr->dragon_jackpot) / double(settingitr->dice_max));
            queues.modify( queueitr,0, [&]( auto& s ) {
                s.count += counter;
                s.profit += profit;
                s.status = (is_clean==true?2:0);
            });

            games.modify( gameitr,0, [&]( auto& s ) {
                if(s.dragon_jackpot>=profit)
                {
                    s.dragon_jackpot -= profit;
                }else
                {
                    s.dragon_jackpot = 0;
                }
                s.total_dividend_amount += profit;
            });

            if(profit>0)
            {
                asset dragonfee(profit,CORE_TOKEN);
                auto owneritr = queues.find(id);
                auto playeridx = players.template get_index<N(owner)>();
                auto playeritr = playeridx.find(owneritr->owner);
                if(playeritr != playeridx.end())
                {
                    playeridx.modify(playeritr, 0, [&](auto &s) {
                        s.dragon_profit += dragonfee.amount;
                    });
                }

                string remark = "获得接龙奖金";
                remark.append(SYSTEM_REMARK);
                action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, owneritr->owner, dragonfee, remark)).send();
            }
        }
    }
    
    //@abi action
    void drawing(const uint64_t& id, const checksum256& banker_seed){
        require_auth(TEAM_SERVICE); 

        auto betitr = orders.find(id);
        eosio_assert(betitr!=orders.end(), "注单ID不存在");
        eosio_assert(betitr->status==0, "注单已结算完成");
        //assert_seed(banker_seed,betitr->banker_seed_hash);

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(betitr->player);
        eosio_assert(playeritr != playeridx.end(), "下注账号不存在"); 

        auto gameitr = games.begin();
        auto settingitr = settings.begin();

        // 下注中奖转账
        asset payout = asset(0, CORE_TOKEN);;
        uint64_t random = get_random(betitr->player_seed,banker_seed,settingitr->dice_max);
        if (random < betitr->under_number && random>=1 && random<=settingitr->dice_max ) 
        {
            string remark = "获取中奖金额";
            remark.append(SYSTEM_REMARK);
            payout = get_payout(betitr->under_number, betitr->bet_amount,settingitr->dice_max,settingitr->dice_water);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, betitr->player, payout, remark)).send();
        }

        //订单完成
        orders.modify(betitr, 0, [&](auto &s) {
            s.status=2;
            s.bet_award=payout;
            s.result_number = random;
            s.banker_seed = banker_seed;
        });

        // 累加玩家盈亏
        auto card_limit =  gameitr->dragon_luckpot * 0.05;
        if(card_limit<100000)
        {
            card_limit=100000;
        }

        auto is_get_card = (playeritr->bet_step+betitr->bet_amount.amount)>=card_limit;
        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.bet_profit += payout.amount;
            if(is_get_card)
            {
                s.dragon_count += 1;
                s.bet_step = (playeritr->bet_step+betitr->bet_amount.amount) - card_limit;
            }else
            {
                s.bet_step += betitr->bet_amount.amount;
            }
        });

        // 修改统计数据
        games.modify(gameitr, 0, [&](auto &s) {
            if(gameitr->locked_balance>=betitr->bet_amount.amount)
            {
                s.locked_balance -= betitr->bet_amount.amount;
            }
            if(payout.amount>0)
            {
                s.total_supply_amount -= (gameitr->total_supply_amount>=payout.amount?payout.amount:0);
            }
        });

        // 写入接龙点数
        if(is_get_card)
        {
            auto dragonidx = dragons.template get_index<N(number)>();
            auto dragonitr = dragonidx.find(random);
            if(dragonitr != dragonidx.end())
            {
                //uint64_t dragon_fee = betitr->bet_amount.amount * get_real_amount(settingitr->system_ratio) * get_real_amount(settingitr->dragon_ratio);
                asset dragonfee(0,CORE_TOKEN);
                auto dragonstopitr = dragons.emplace(_self,[&](auto & s){
                    s.id = get_next_dragon_id();
                    s.price = dragonfee;
                    s.number = random;
                    s.owner = betitr->player;
                });

                auto queueitr = queues.begin();
                auto is_exist = false;
                while(queueitr != queues.end())
                {
                    if(random >= queueitr->start && random<=queueitr->stop)
                    {
                        is_exist=true;
                        break;
                    }
                    queueitr++;
                }

                if(is_exist==false){
                    queues.emplace(_self,[&](auto & s){
                        s.id = get_next_dragon_id();
                        s.start = dragonitr->id;
                        s.stop = dragonstopitr->id;
                        s.owner = betitr->player;
                        s.number = random;
                    });
                }

            }else
            {
                asset dragonfee(0,CORE_TOKEN);
                dragons.emplace(_self,[&](auto & s){
                    s.id = get_next_dragon_id();
                    s.price = dragonfee;
                    s.number = random;
                    s.owner = betitr->player;
                });
            }

            auto weight_count = 0UL;
            auto weightidx = players.template get_index<N(weight)>();
            auto weightitr = weightidx.rbegin();
            games.modify(gameitr, 0, [&](auto &s) {
                s.dragon_ranks.clear();
                s.dragon_luckowner = betitr->player;
                if((s.dragon_lucktime-now()+settingitr->luck_time_increase)>settingitr->luck_time_offset)
                {
                    s.dragon_lucktime = now()+settingitr->luck_time_offset;
                }else
                {
                    s.dragon_lucktime =s.dragon_lucktime+settingitr->luck_time_increase;
                }
            });

            vector<weight> dragon_ranks;
            while(weightitr != weightidx.rend() && weight_count<8 && weightitr->dragon_count>0 && weightitr->dragon_round==gameitr->dragon_luckround)
            {
                weight weight{
                    .rank = weightitr->dragon_count,
                    .owner =weightitr->owner
                };
                dragon_ranks.push_back(weight);
                weight_count++;
                weightitr++;
            }
            games.modify(gameitr, 0, [&](auto &s) {
                s.dragon_ranks = dragon_ranks;
            });
        }

        // 发送注单回执
        order order{
            .id = betitr->id,
            .player = betitr->player,
            .referrer = betitr->referrer,
            .status = 2,
            .bet_amount = betitr->bet_amount,
            .bet_award = payout,
            .bet_mining = betitr->bet_mining,
            .bet_time = betitr->bet_time,
            .under_number = betitr->under_number,
            .result_number = random,
            .player_seed = betitr->player_seed,
            .banker_seed = banker_seed,
            .banker_seed_hash = betitr->banker_seed_hash
        };

        // 生成中奖回执
        transaction transfer; 
        transfer.actions.emplace_back(permission_level{_self, N(active)}, TEAM_BETLOGS, N(order), order); 
        transfer.send(get_next_drawing_id(), _self, false); 
    }

    //@abi action
    void teambonus(){
        require_auth(TEAM_SERVICE); 

        auto gameitr = games.begin();
        auto team_fee = gameitr->total_team_fee;
        if (team_fee>0) {

            string remark = "定时提取研发费用";
            remark.append(SYSTEM_REMARK);
            asset teamfee(team_fee,CORE_TOKEN);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, TEAM_ACCOUNT, teamfee, remark)).send();

            games.modify(gameitr, 0, [&](auto &s) {
                s.total_team_fee = 0;
                s.total_dividend_amount += team_fee;
            });
        }

        auto shadows_fee = gameitr->total_shadows_fee;
        if (shadows_fee>0) {

            string remark = "定时提取简影分红";
            remark.append(SYSTEM_REMARK);
            asset shadowsfee(shadows_fee,CORE_TOKEN);
            action(permission_level{_self, N(active)},CORE_ACCOUNT,N(transfer),make_tuple(_self, TEAM_SHADOWS, shadowsfee, remark)).send();

            games.modify(gameitr, 0, [&](auto &s) {
                s.total_shadows_fee = 0;
                s.total_dividend_amount += shadows_fee;
            });
        }
    }

    //@abi action
    void refbonus(account_name from) {
        require_auth(from);

        auto settingitr = settings.begin();
        eosio_assert(settingitr!=settings.end(), "系统尚未初始化" );

        auto playeridx = players.template get_index<N(owner)>();
        auto playeritr = playeridx.find(from);
        eosio_assert(playeritr != playeridx.end(), "推荐账号不存在" );
        eosio_assert(playeritr->referrer_amount>0, "没有足够的分红" );

        string remark = "获得推荐分红";
        remark.append(SYSTEM_REMARK);
        asset claimfee(playeritr->referrer_amount,TEAM_TOKEN);
        action(permission_level{_self, N(active)},TEAM_BANKERS,N(transfer),make_tuple(_self, from, claimfee, remark)).send();

        playeridx.modify(playeritr, 0, [&](auto &s) {
            s.referrer_amount = 0;
        });
    }

    //@abi action
    void ordererase(std::vector<uint64_t>& list){
        require_auth(TEAM_SERVICE); 

        for(const auto&item:list)
        {
            auto orderitr = orders.find(item);
            if(orderitr!=orders.end() && orderitr->status==2)
            {
                orders.erase(orderitr);
            }
        }
    }

    //@abi action
    void dragonerase(std::vector<uint64_t>& list){
        require_auth(TEAM_SERVICE); 

        for(const auto&item:list)
        {
            auto queueitr = queues.find(item);
            if(queueitr!=queues.end() && queueitr->status==2)
            {
                queues.erase(queueitr);
            }
        }
    }

    //@abi action
    void config(string field,uint64_t value){
        require_auth(TEAM_SERVICE); 

        auto settingitr = settings.begin();
        settings.modify(settingitr, 0, [&](auto &s) {
            if(field == "min_bet")
            {
                s.min_bet = value;// 最低下注
            }
            if(field == "max_bet")
            {
                s.max_bet = value;// 最高下注
            }
            if(field == "dice_max")
            {
                s.dice_max = value;// 骰子点数
            }
            if(field == "dice_edge")
            {
                s.dice_edge = value;// 庄家优势
            }
            if(field == "dice_water")
            {
                s.dice_water = value;// 庄家抽水
            }
            if(field == "is_staking")
            {
                s.is_staking = value;// 允许质押
            }
            if(field == "is_mining")
            {
                s.is_mining = value;// 允许挖矿
            }
            if(field == "is_betting")
            {
                s.is_betting = value;// 允许下注
            }
            if(field == "is_bidding")
            {
                s.is_bidding = value;// 允许竞拍
            }
            if(field == "team_ratio")
            {
                s.team_ratio = value;// 平台比例
            }
            if(field == "stake_ratio")
            {
                s.stake_ratio = value;// 质押比例
            }
            if(field == "system_ratio")
            {
                s.system_ratio = value;// 系统比例
            }
            if(field == "dragon_ratio")
            {
                s.dragon_ratio = value;// 接龙比例
            }
            if(field == "lucky_ratio")
            {
                s.lucky_ratio = value;// 幸运比例
            }
            if(field == "shadows_ratio")
            {
                s.shadows_ratio = value;// 简影比例
            }
            if(field == "referrer_ratio")
            {
                s.referrer_ratio = value;// 推荐比例
            }
            if(field == "bidding_ratio")
            {
                s.bidding_ratio = value;// 竞拍比例
            }
            if(field == "bidding_losing")
            {
                s.bidding_losing = value;// 竞拍损失
            }
            if(field == "bidding_offset")
            {
                s.bidding_offset = value;// 竞拍时间间隔
            }
            if(field == "bidding_minimum")
            {
                s.bidding_minimum = value;// 竞拍最小金额
            }
            if(field == "bidding_increase")
            {
                s.bidding_increase = value;// 竞拍增长比例
            }
            if(field == "bidding_min_bonus")
            {
                s.bidding_min_bonus = value;// 竞拍最大奖励
            }
            if(field == "bidding_max_bonus")
            {
                s.bidding_max_bonus = value;// 竞拍最大奖励
            }
            if(field == "luck_time_offset")
            {
                s.luck_time_offset = value;// 接龙权重奖池时间间隔
            }
            if(field == "luck_time_increase")
            {
                s.luck_time_increase = value;// 接龙权重时间增加数量
            }
            if(field == "stake_time_offset")
            {
                s.stake_time_offset = value;// 质押时间间隔
            }
            if(field == "stake_time_redeem")
            {
                s.stake_time_redeem = value;// 质押赎回小时
            }
            if(field == "stake_dividend_ratio")
            {
                s.stake_dividend_ratio = value;// 质押分红比例
            }            
            if(field == "hand_card_limit")
            {
                s.hand_card_limit = value;// 出牌限额
            }
            if(field == "max_payout_amount")
            {
                if(value>s.max_payout_amount)
                {
                    s.max_payout_amount = value;// 最大赔付金额
                }else
                {
                    eosio_assert(false,"新值必须大于最大赔付金额");
                }
                
            }
            if(field == "mining_supply_amount")
            {
                s.mining_supply_amount = value;// 挖矿供应量
            }
            
        });
        
        auto gameitr = games.begin();
        games.modify(gameitr, 0, [&](auto &s) {
            if(field == "total_supply_amount") // 总供应额度
            {
              s.total_supply_amount = value;
            }
            if(field == "total_mining_amount") // 已挖矿额度
            {
              s.total_mining_amount = value;
            }
            if(field == "dragon_lucktime") // 幸运接龙时间
            {
              s.dragon_lucktime = now()+settingitr->luck_time_offset;
            }
            if(field == "stake_time") // 质押挖矿时间
            {
              s.stake_time = now()+settingitr->stake_time_offset;
            }
        });
                    

    }

    //@abi action
    void clean(uint64_t clean_type){
        require_auth(TEAM_SERVICE); 

        if(clean_type==1)//系统设置
        {
            auto settingitr = settings.begin();
            while( settingitr != settings.end() ) 
            {
                settingitr = settings.erase(settingitr);
            } 

            auto gameitr = games.begin();
            while( gameitr != games.end() ) 
            {
                gameitr = games.erase(gameitr);
            }
        }else if(clean_type==2)//竞拍数据
        {
            auto biddingitr = biddings.begin();
            auto biddingcounter = 0;
            while( biddingitr != biddings.end()) 
            {
                if(biddingcounter>=150)break;
                biddingitr = biddings.erase(biddingitr);
                biddingcounter++;
            }
        }else if(clean_type==3)//玩家数据
        {
            auto playeritr = players.begin();
            auto playercounter = 0;
            while( playeritr != players.end()) 
            {
                if(playercounter>=150)break;
                playeritr = players.erase(playeritr);
                playercounter++;
            }
        }else if(clean_type==4)//订单数据
        {
            auto orderitr = orders.begin();
            auto ordercounter = 0;
            while( orderitr != orders.end()) 
            {
                if(ordercounter>=150)break;
                orderitr = orders.erase(orderitr);
                ordercounter++;
            }
        }else if(clean_type==5)//队列数据
        {
            auto queueitr = queues.begin();
            auto queuecounter = 0;
            while( queueitr != queues.end()) 
            {
                if(queuecounter>=150)break;
                queueitr = queues.erase(queueitr);
                queuecounter++;
            }
        }else if(clean_type==6)//接龙数据
        {
            auto dragonitr = dragons.begin();
            auto dragoncounter = 0;
            while( dragonitr != dragons.end()) 
            {
                if(dragoncounter>=150)break;
                dragonitr = dragons.erase(dragonitr);
                dragoncounter++;
            }
        }else if(clean_type==7)//hash数据
        {
            auto seeditr = seeds.begin();
            auto seedcounter = 0;
            while( seeditr != seeds.end()) 
            {
                if(seedcounter>=50)break;
                seeditr = seeds.erase(seeditr);
                seedcounter++;
            }
        }
    }
        
    //@abi action
    void receipt(order& receipt){
        require_auth(_self);
        //require_recipient(receipt.player);
    }

    //@abi action  
    void test()
    {
        checksum256 h;
        auto size = transaction_size();
        char buf[size];
        uint32_t read = read_transaction( buf, size );
        eosio_assert( size == read, "read_transaction failed");
        sha256(buf, read, &h);
        printhex( &h, sizeof(h) );
    }
};

 #define EOSIO_ABI_EX( TYPE, MEMBERS ) \
 extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
       if( action == N(onerror)) { \
          eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
       } \
       auto self = receiver; \
       if((code == CORE_ACCOUNT && action == N(transfer)) || (code == TEAM_BANKERS && action == N(transfer)) || (code == self && (action == N(unstake) || action == N(bidbonus) || action == N(stakebonus) || action == N(luckbonus) || action == N(dragonbonus) || action == N(teambonus) || action == N(refbonus) || action == N(stakeclaim) || action == N(unstakedelay) || action == N(unstakeoff) || action == N(drawing) || action == N(ordererase) || action == N(dragonerase) || action == N(config)|| action == N(clean) || action == N(test) || action == N(onerror))) ) { \
          TYPE thiscontract( self ); \
          switch( action ) { \
             EOSIO_API( TYPE, MEMBERS ) \
          } \
       } \
    } \
 }

EOSIO_ABI_EX(shadowsdicer, (transfer)(unstake)(bidbonus)(stakebonus)(luckbonus)(dragonbonus)(teambonus)(refbonus)(stakeclaim)(unstakedelay)(unstakeoff)(drawing)(ordererase)(dragonerase)(config)(clean)(test))