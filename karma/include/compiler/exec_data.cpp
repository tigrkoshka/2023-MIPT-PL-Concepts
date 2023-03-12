#include "exec_data.hpp"

namespace karma::compiler::detail {

ExecData::Segment& ExecData::Code() {
    return code_;
}

const ExecData::Segment& ExecData::Code() const {
    return code_;
}

ExecData::Segment& ExecData::Constants() {
    return constants_;
}

const ExecData::Segment& ExecData::Constants() const {
    return constants_;
}

}  // namespace karma::compiler::detail
