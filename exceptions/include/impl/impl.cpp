#include "impl.hpp"

#include <csetjmp>          // for longjmp
#include <exception>        // for terminate
#include <iostream>         // for cerr, endl
#include <source_location>  // for source_location
#include <utility>          // for move

#include "impl/data.hpp"
#include "impl/node.hpp"
#include "objects/manager.hpp"

/*                                   // stack                : caught
 *
 * try {                             // 1 (0)                :
 *     try {                         // 1 (0), 2 (1)         :
 *
 *                                   // caught is empty, so just longjmp
 *        throw(type::A);            // 1 (0), 2* (1)        :
 *
 *     } catch (type::A) {           // 1 (0)                : 2 (1)
 *        try {                      // 1 (0), 3 (1)         : 2 (1)
 *           try {                   // 1 (0), 3 (1), 4 (2)  : 2 (1)
 *
 *                                   // stack.top().depth >= caught.top().depth
 *                                   // so do not pop from caught, just longjmp
 *              rethrow;             // 1 (0), 3 (1), 4* (2) : 2 (1)
 *
 *           } catch (type::A) {     // 1 (0), 3 (1)         : 2 (1), 4 (2)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth
 *                                   // pop from caught
 *              rethrow;             // 1 (0), 3* (1)        : 2 (1)
 *           }
 *        } catch(type::A) {         // 1 (0)                : 2 (1), 3 (1)
 *           // OK
 *                                   // stack.top() is not raised,
 *                                   // stack.top().depth < caught.top().depth
 *                                   // so pop from caught
 *        }                          // 1 (0)                : 2 (1)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth
 *                                   // pop from caught
 *        rethrow;                   // 1* (0)               :
 *     }
 * } catch (type::A) {               //                      : 1 (0)
 *     // OK
 *                                   // stack is empty (aka not raised),
 *                                   // so pop from caught
 * }                                 //                      :
 *
 */

/*
 * try {                                 // 1 (0)
 *     try {                             // 1 (0), 2 (1)         :
 *
 *                                       // caught is empty, so just longjmp
 *         throw(type::A)                // 1 (0), 2* (1)        :
 *
 *     } catch (type::A) {               // 1 (0)                : 2 (1)
 *         try {                         // 1 (0), 3 (1)         : 2 (1)
 *
 *                                       // stack.top().depth >=
 * caught.top().depth,
 *                                       // so just longjmp
 *             throw(type::B)            // 1 (0), 3* (1)        : 2 (1)
 *
 *         } catch (type::B) {           // 1 (0)                : 2 (1), 3 (1)
 *
 *                                       // while stack.top().depth <
 * caught.top().depth
 *                                       // pop from caught
 *             rethrow;                  // 1* (0)               :
 *         }
 *     }
 * } catch (type::B) {                   //                      : 1 (0)
 *     // OK
 *                                       // stack is empty (aka not raised),
 *                                       // so pop from caught
 * }                                     //                      :
 *
 */

/*
 * try {                             // 1 (0)                :
 *     try {                         // 1 (0), 2 (1)         :
 *
 *                                   // caught is empty, so just longjmp
 *         throw(type::A);           // 1 (0), 2* (1)        :
 *
 *     } catch (type::A) {           // 1 (0)                : 2 (1)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth,
 *                                   // pop from caught
 *         throw(type::B);           // 1* (0)               :
 *     }
 * } catch (type::B) {               //                      : 1 (0)
 *     // OK
 *                                   // stack is empty (aka not raised),
 *                                   // so pop from caught
 * }                                 //                      :
 *
 */

/*
 * try {                             // 1 (0)                         :
 *     try {                         // 1 (0), 2 (1)                  :
 *         try {                     // 1 (0), 2 (1), 3 (2)           :
 *             try {                 // 1 (0), 2 (1), 3 (2), 4 (3)    :
 *
 *                                   // caught is empty, so just longjmp
 *                 throw(type::A);   // 1 (0), 2 (1), 3 (2), 4* (3)   :
 *
 *             } catch (type::A) {   // 1 (0), 2 (1), 3 (2)           : 4 (3)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth,
 *                                   // pop from caught
 *                 throw(type::B);   // 1 (0), 2 (1), 3* (2)          :
 *             }
 *         } catch (type::B) {       // 1 (0), 2 (1)                  : 3 (2)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth,
 *                                   // pop from caught
 *             rethrow;              // 1 (0), 2* (1)                 :
 *         }
 *     } catch (type::B) {           // 1 (0)                         : 2 (1)
 *         try {                     // 1 (0), 5 (1)                  : 2 (1)
 *
 *                                   // stack.top().depth >= caught.top().depth,
 *                                   // so just longjmp
 *             throw(type::A);       // 1 (0), 5* (1)                 : 2 (1)
 *
 *         } catch (type::A) {       // 1 (0)                         : 2 (1), 5
 * (1)
 *             // OK
 *                                   // stack is empty (aka not raised),
 *                                   // so pop from caught
 *         }                         // 1 (0)                         : 2 (1)
 *
 *                                   // while stack.top().depth <
 * caught.top().depth,
 *                                   // pop from caught
 *         rethrow;                  // 1* (0)                        :
 *     }
 * } catch (type::B) {               //                               : 1 (0)
 *     // OK
 *                                   // stack is empty (aka not raised),
 *                                   // so pop from caught
 * }                                 //                               :
 *
 */

/* try {                                 // 1 (0)                       :
 *     try {                             // 1 (0), 2 (0)                :
 *         throw(type::A);               // 1 (0), 2* (1)               :
 *     } catch (type::A) {               // 1 (0)                       : 2 (1)
 *         try {                         // 1 (0), 3 (1)                : 2 (1)
 *             try {                     // 1 (0), 3 (1), 4 (2)         : 2 (1)
 *                 try {                 // 1 (0), 3 (1), 4 (2), 5 (3)  : 2 (1)
 *
 *                                       // stack.top().depth >=
 * caught.top().depth
 *                                       // so do not pop from caught
 *                     throw(type::B);   // 1 (0), 3 (1), 4 (2), 5* (3) : 2 (1)
 *
 *                 } catch (type::B) {   // 1 (0), 3 (1), 4 (2)         : 2 (1),
 * 5 (3)
 *
 *                                       // while stack.top().depth <
 * caught.top().depth,
 *                                       // pop from caught
 *                     rethrow;          // 1 (0), 3 (1), 4* (2)        : 2 (1)
 *                 }
 *             } catch (type::A) {       // fails
 *                 // smth
 *
 *                                       // stack.top() is raised,
 *                                       // so pop from stack;
 *                                       // stack.top().depth >=
 * caught.top().depth,
 *                                       // so do not pop from caught
 *             }                         // 1 (0), 3* (1)               : 2 (1)
 *
 *         } catch (type::A) {           // fails
 *             // smth
 *                                       // stack.top() is raised,
 *                                       // so pop from stack;
 *                                       // while stack.top().depth <
 * caught.top().depth
 *                                       // pop from caught
 *         }                             // 1* (0)                      :
 *     }
 * } catch (type::B) {                   //                             : 1 (0)
 *     // OK
 *                                       // stack is empty (aka not raised),
 *                                       // so pop from caught
 * }                                     //                             :
 *
 */

/*
 * try {                   // 1 (0)  :
 *    // OK
 * } catch (type::A) {
 *    // smth
 *                         // stack top is not raised
 *                         // and caught is empty,
 *                         // so pop from stack
 * }                       //        :
 *
 */

/*
 * try {                      // 1 (0)  :
 *
 *                            // caught is empty, so just longjmp
 *    throw(type::A);         // 1* (0) :
 * } catch (type::A) {        //        : 1 (0)
 *    try {                   // 2 (0)  : 1 (0)
 *        // OK
 *    } catch (type::B) {
 *        // smth
 *                            // stack.top() is not raised
 *                            // stack.top().depth >= caught.top().depth,
 *                            // so pop from stack
 *    }                       //        : 1 (0)
 *
 *                            // stack is empty (aka not raised),
 *                            // so pop from caught
 * }                          //        :
 *
 */

namespace except::detail {

int* Impl::StartTry() {
    stack.emplace(stack.size());
    ObjectManager::RecordCheckpoint();
    return stack.top().Buff();
}

void Impl::FinishTry() {
    if (stack.empty()) {
        caught.pop();
        return;
    }

    if (stack.top().raised) {
        // same as Catch(); Rethrow();

        Node thrown = std::move(stack.top());
        stack.pop();

        Throw(std::move(thrown.data), thrown.source_location);
    }

    if (!caught.empty() && stack.top().depth < caught.top().depth) {
        caught.pop();
    } else {
        ObjectManager::Pop();
        stack.pop();
    }
}

void Impl::DoCatch() {
    caught.push(std::move(stack.top()));
    stack.pop();
}

bool Impl::CatchAll() {
    DoCatch();
    return true;
}

void Impl::Throw(Data data, std::source_location source_location) {
    if (stack.empty()) {
        std::cerr << "uncaught exception at line " << source_location.line()
                  << " of file " << source_location.file_name() << std::endl;
        std::terminate();
    }

    while (!caught.empty() && stack.top().depth < caught.top().depth) {
        caught.pop();
    }

    ObjectManager::UnwindToCheckpoint();
    stack.top().Raise(std::move(data), source_location);
    std::longjmp(stack.top().Buff(), 1);  // NOLINT(cert-err52-cpp)
}

void Impl::Rethrow() {
    Throw(std::move(caught.top().data), caught.top().source_location);
}

thread_local std::stack<Node> Impl::stack;
thread_local std::stack<Node> Impl::caught;

}  // namespace except::detail
