#ifndef BPN_ROTATION_COMMON_HPP_
#define BPN_ROTATION_COMMON_HPP_

#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

namespace bpn_rotation {

// -------------------------------------
//   Functions
// -------------------------------------
struct timespec jst2utc(struct timespec);
std::string gen_time_str(struct timespec);

}  // namespace bpn_rotation

#endif

