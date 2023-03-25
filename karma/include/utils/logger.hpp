#pragma once

#include <ostream>    // for ostream
#include <streambuf>  // for streambuf

namespace karma {

struct Logger {
   private:
    struct NoOpStreamBuff : std::streambuf {
        int overflow(int num) override;
    };

    static NoOpStreamBuff no_op_stream_buff;
    static std::ostream no_op_stream;

   public:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    Logger(std::ostream& log)
        : log(log) {}

    static Logger NoOp();

   public:
    std::ostream& log;
};

}  // namespace karma