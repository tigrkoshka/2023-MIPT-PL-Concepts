#include "logger.hpp"

#include <ostream>    // for ostream
#include <streambuf>  // for streambuf

namespace karma::detail::utils::logger {

class NoOpStreamBuff : public std::streambuf {
   public:
    int overflow(int num) override {
        return num;
    }
};

NoOpStreamBuff no_op_stream_buff;
std::ostream no_op(&no_op_stream_buff);

}  // namespace karma::detail::utils::logger
