#include "logger.hpp"

#include <ostream>  // for ostream

namespace karma {

int Logger::NoOpStreamBuff::overflow(int num) {
    return num;
}

Logger Logger::NoOp() {
    return {no_op_stream};
}

Logger::NoOpStreamBuff Logger::no_op_stream_buff = NoOpStreamBuff();
std::ostream Logger::no_op_stream = std::ostream(&no_op_stream_buff);

}  // namespace karma
