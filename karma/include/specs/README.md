## Specs

This directory provides type definitions and constants defined
in the Karma assembler standard, which are used throughout
the [karma library](../../include).

### Dependencies

The declarations of the symbols from this directory are not dependent
on any other parts of the [karma library](../../include).

The definitions of some symbols from this directory are dependent
on the symbols provided by the [utils directory](../utils).

### Symbols

#### Internal

```c++
karma::
        detail::
                specs::
                        arch::                                 // architecture.hpp
                        |       // Architecture-defined types
                        |       Word
                        |       TwoWords
                        |       Double
                        |       kWordSize
                        |       kMaxWord
                        |
                        |       // Registers
                        |       Register
                        |       RegisterEnum
                        |       kNRegisters
                        |       kCallFrameRegister
                        |       kStackRegister
                        |       kInstructionRegister
                        |       kRegisterNameToNum
                        |       kRegisterNumToName
                        |
                        |       // Memory
                        |       Address
                        |       kMemorySize
                        cmd::                                  // commands.hpp
                        |       // Code and format
                        |       Bin
                        |       Format
                        |       Code
                        |       CodeFormat
                        |       GetCode
                        |       kFormatToString
                        |       kCodeToFormat
                        |       kNameToCode
                        |       kCodeToName
                        |
                        |       args::
                        |       |       Register
                        |       |       Receiver
                        |       |       Source
                        |       |       Address
                        |       |       Modifier
                        |       |       Immediate
                        |       |       kModSize
                        |       |       kImmSize
                        |       |       RMArgs
                        |       |       RRArgs
                        |       |       RIArgs
                        |       |       JArgs
                        |       |
                        |       syscall::
                        |       |       Char
                        |       |       kMaxChar
                        |       |       Code
                        |       |
                        |       parse::
                        |       |       RM
                        |       |       RR
                        |       |       RI
                        |       |       J
                        |       |
                        |       build::
                        |               RM
                        |               RR
                        |               RI
                        |               J
                        consts::                               // constants.hpp
                        |       // Types
                        |       Type
                        |       UInt32
                        |       UInt64
                        |       Double
                        |       Char
                        |       String
                        |       kTypeToName
                        |       kNameToType
                        |   
                        |       // Type specs
                        |       kDoublePrecision
                        |       kCharQuote
                        |       kStringQuote
                        |       kStringEnd
                        exec::                                 // exec.hpp
                        |       kDefaultExtension
                        |       kIntroString
                        |       kIntroSize
                        |       kMetaInfoEndPos
                        |       kHeaderSize
                        |       kCodeSegmentPos
                        |       kProcessorID
                        flags::                                // flags.hpp
                        |       Flag
                        |       kEqual
                        |       kGreater
                        |       kLess
                        syntax::                               // syntax.hpp
                        |       kCommentSep
                        |       kDisableCommentSep
                        |       kLabelEnd
                        |       IsAllowedLabelChar
                        |       kIncludeDirective
                        |       kEntrypointDirective
```
