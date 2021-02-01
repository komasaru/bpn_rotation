/***********************************************************
  バイアス・歳差・章動適用

    DATE          AUTHOR          VERSION
    2020.11.16    mk-mode.com     1.00 新規作成

  Copyright(C) 2020 mk-mode.com All Rights Reserved.

  引数 : JST（日本標準時） X Y Z
         書式 * JST: 最大23桁の数字
                （先頭から、西暦年(4), 月(2), 日(2), 時(2), 分(2), 秒(2),
                            1秒未満(9)（小数点以下9桁（ナノ秒）まで））
              * X, Y, Z: 元の赤道直交座標
***********************************************************/
#include "bpn.hpp"
#include "common.hpp"
#include "coord.hpp"
#include "file.hpp"
#include "time.hpp"

#include <cstdlib>   // for EXIT_XXXX
#include <ctime>
#include <iomanip>   // for setprecision
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  namespace ns = bpn_rotation;
  std::string tm_str;      // time string
  unsigned int s_tm;       // size of time string
  int s_nsec;              // size of nsec string
  struct timespec ts;      // UTC
  struct timespec ts_jst;  // JST
  struct timespec ts_tdb;  // TDB
  struct tm t = {};        // for work
  double jcn;              // Julian Century Number
  ns::Coord pos_src;       // 直交座標（元の）
  ns::Coord pos_res;       // 座標（適用後）
  std::vector<std::vector<std::string>> l_ls;    // List of Leap Second
  std::vector<std::vector<std::string>> l_dut;   // List of DUT1
  std::vector<std::vector<double>>      dat_ls;  // data of lunisolar parameters
  std::vector<std::vector<double>>      dat_pl;  // data of planetary parameters

  try {
    // コマンドライン引数の個数チェック
    if (argc < 5) {
      std::cout << "Usage: ./bpn_rotation JST X Y Z" << std::endl;
      return EXIT_SUCCESS;
    }

    // 日本標準時取得
    tm_str = argv[1];
    s_tm = tm_str.size();
    if (s_tm > 23) {
      std::cout << "[ERROR] Over 23-digits!" << std::endl;
      return EXIT_FAILURE;
    }
    s_nsec = s_tm - 14;
    std::istringstream is(tm_str);
    is >> std::get_time(&t, "%Y%m%d%H%M%S");
    ts_jst.tv_sec  = mktime(&t);
    ts_jst.tv_nsec = 0;
    if (s_tm > 14) {
      ts_jst.tv_nsec = std::stod(
          tm_str.substr(14, s_nsec) + std::string(9 - s_nsec, '0'));
    }

    // 元の赤道直交座標取得
    pos_src.x = std::stod(argv[2]);
    pos_src.y = std::stod(argv[3]);
    pos_src.z = std::stod(argv[4]);

    // うるう秒, DUT1 一覧、
    // lunisolra, planetary パラメータ一覧取得
    l_ls.reserve(50);     // 予めメモリ確保
    l_dut.reserve(250);   // 予めメモリ確保
    dat_ls.reserve(700);  // 予めメモリ確保
    dat_pl.reserve(700);  // 予めメモリ確保
    ns::File o_f;
    if (!o_f.get_leap_sec_list(l_ls)) throw;
    if (!o_f.get_dut1_list(l_dut))    throw;
    if (!o_f.get_param_ls(dat_ls))    throw;
    if (!o_f.get_param_pl(dat_pl))    throw;

    // JST -> UTC
    ts = ns::jst2utc(ts_jst);

    // TDB（太陽系力学時）, T（JCN; ユリウス世紀数）計算
    // （TDB（太陽系力学時）の代わりに、実質的に同じとみなしてもよいと
    //   されている TT（地球時）を計算してもよい）
    ns::Time o_tm(ts, l_ls, l_dut);  // Object of UTC
    ts_tdb = o_tm.calc_tdb();
    ns::Time o_tm_tdb(ts_tdb, l_ls, l_dut);  // Object of TDB
    jcn = o_tm_tdb.calc_t();

    // Calculation & Display
    ns::Bpn o_bpn(jcn, dat_ls, dat_pl);
    std::cout << "      JST: "
              << ns::gen_time_str(ts_jst) << std::endl
              << "      TDB: "
              << ns::gen_time_str(ts_tdb) << std::endl
              << "T(of TDB): "
              << std::fixed << std::setprecision(10) << jcn
              << " century (= Julian Century Number)" << std::endl;
    pos_res = o_bpn.apply_bias(pos_src);           // バイアス適用
    std::cout << "---" << std::endl
              << "元の GCRS 座標: ["
              << pos_src.x << ", " << pos_src.y << ", " << pos_src.z
              << "]" << std::endl;
    std::cout << "+ バイアス適用: ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_prec(pos_res);           // 歳差適用
    std::cout << "+     歳差適用: ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_nut(pos_res);            // 章動適用
    std::cout << "+     章動適用: ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_bias(pos_src);           // バイアス適用
    std::cout << "---" << std::endl
              << "元の GCRS 座標: ["
              << pos_src.x << ", " << pos_src.y << ", " << pos_src.z
              << "]" << std::endl;
    std::cout << "+ バイアス適用: ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_prec_nut(pos_src);      // 歳差＆章動適用
    std::cout << "+ 歳差＆章動同時適用: " << std::endl
              << "                ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_bias_prec(pos_src);      // バイアス＆歳差適用
    std::cout << "---" << std::endl
              << "元の GCRS 座標: ["
              << pos_src.x << ", " << pos_src.y << ", " << pos_src.z
              << "]" << std::endl;
    std::cout << "+ バイアス＆歳差同時適用: " << std::endl
              << "                ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_nut(pos_res);            // 章動適用
    std::cout << "+     章動適用: ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;
    pos_res = o_bpn.apply_bias_prec_nut(pos_src);  // バイアス＆歳差＆章動適用
    std::cout << "---" << std::endl
              << "元の GCRS 座標: ["
              << pos_src.x << ", " << pos_src.y << ", " << pos_src.z
              << "]" << std::endl
              << "+ バイアス＆歳差＆章動同時適用: " << std::endl
              << "                ["
              << pos_res.x << ", " << pos_res.y << ", " << pos_res.z
              << "]" << std::endl;

  } catch (...) {
      std::cerr << "EXCEPTION!" << std::endl;
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

