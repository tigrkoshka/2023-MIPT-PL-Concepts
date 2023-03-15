#include "exec_data.hpp"

namespace karma {

Compiler::ExecData::Segment& Compiler::ExecData::Code() {
    return code_;
}

const Compiler::ExecData::Segment& Compiler::ExecData::Code() const {
    return code_;
}

Compiler::ExecData::Segment& Compiler::ExecData::Constants() {
    return constants_;
}

const Compiler::ExecData::Segment& Compiler::ExecData::Constants() const {
    return constants_;
}

}  // namespace karma
