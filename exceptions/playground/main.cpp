#include <iostream>
#include <vector>

#include "../include/except"

struct NonTrivial final {
    explicit NonTrivial()
        : index_(index++) {
        std::cout << "NonTrivial: default " << index_ << std::endl;
    }
    ~NonTrivial() {
        std::cout << "NonTrivial: destructor " << index_ << std::endl;
    }

    NonTrivial(std::string s)
        : index_(s.size()) {
        std::cout << "NonTrivial: single-argument implicit" << std::endl;
    }
    explicit NonTrivial(size_t s)
        : index_(s) {
        std::cout << "NonTrivial: single-argument explicit" << std::endl;
    }
    NonTrivial(size_t idx, std::string s)
        : index_(s.size() + idx) {
        std::cout << "NonTrivial: multi-argument implicit" << std::endl;
    }
    explicit NonTrivial(std::string s, size_t idx)
        : index_(s.size() + idx) {
        std::cout << "NonTrivial: multi-argument explicit" << std::endl;
    }

    NonTrivial(const NonTrivial& other)
        : index_(other.index_) {
        std::cout << "NonTrivial: copy-construction" << std::endl;
    }

    NonTrivial& operator=(const NonTrivial& other) {
        std::cout << "NonTrivial: copy-assignment" << std::endl;

        if (this == &other) {
            return *this;
        }

        this->index_ = other.index_;
        return *this;
    }

    NonTrivial(NonTrivial&& other) noexcept
        : index_(std::exchange(other.index_, 0)) {
        std::cout << "NonTrivial: move-construction" << std::endl;
    }

    NonTrivial& operator=(NonTrivial&& other) {
        std::cout << "NonTrivial: move-assignment" << std::endl;

        this->index_ = other.index_;
        return *this;
    };

    operator bool() const {
        return index_ > 0;
    }

    explicit operator size_t() const {
        return index_;
    }

    explicit operator std::string() const {
        return std::to_string(index_);
    }

   private:
    size_t index_;
    static size_t index;
};

size_t NonTrivial::index = 0;

void InitializerLists() {
    AutoObject<std::vector<int>> o = {
        {1, 2, 3},
        std::allocator<int>()
    };

    std::vector<int> v = {
        {1, 2, 3},
        std::allocator<int>()
    };

    AutoObject<std::vector<int>> o2{1, 2, 3};
    std::vector<int> v2{1, 2, 3};

    AutoObject<std::vector<int>> o3 = {1, 2, 3};
    std::vector<int> v3 = {1, 2, 3};

    const AutoObject<std::vector<int>>& o4 = std::move(v3);
}

void Constructors() {
    // check constructors

    //    {
    //        AutoObject<const NonTrivial> oo0;
    //        AutoObject<NonTrivial[10]> oo1;
    //        AutoObject<NonTrivial&> oo2;
    //        AutoObject<NonTrivial*> oo3;
    //        AutoObject<NonTrivial****> oo4;
    //    }

    // default

    {
        NonTrivial nt;
        AutoObject<NonTrivial> oo;
        std::cout << std::endl;
    }

    // implicit single-argument

    {
        NonTrivial nt{std::string("15")};
        AutoObject<NonTrivial> oo{std::string("15")};
        std::cout << std::endl;
    }

    {
        NonTrivial nt             = std::string("15");
        AutoObject<NonTrivial> oo = std::string("15");
        std::cout << std::endl;
    }

    // implicit single-argument with argument conversion

    {
        NonTrivial nt{"15"};
        AutoObject<NonTrivial> oo{"15"};
        std::cout << std::endl;
    }

    //    {
    //        NonTrivial nt             = "15";
    //        AutoObject<NonTrivial> oo = "15";
    //    }

    // explicit single-argument

    {
        NonTrivial nt{15ull};
        AutoObject<NonTrivial> oo{15ull};
        std::cout << std::endl;
    }

    //    {
    //        NonTrivial nt = 15ull;
    //        AutoObject<NonTrivial> oo = 15ull;
    //    }

    // implicit multi-argument

    {
        NonTrivial nt{15ull, std::string("15")};
        AutoObject<NonTrivial> oo{15ull, std::string("15")};
        std::cout << std::endl;
    }

    {
        NonTrivial nt             = {15ull, std::string("15")};
        AutoObject<NonTrivial> oo = {15ull, std::string("15")};
        std::cout << std::endl;
    }

    // implicit multi-argument with argument conversion

    {
        NonTrivial nt{15ull, "15"};
        AutoObject<NonTrivial> oo{15ull, "15"};
        std::cout << std::endl;
    }

    {
        NonTrivial nt             = {15ull, "15"};
        AutoObject<NonTrivial> oo = {15ull, "15"};
        std::cout << std::endl;
    }

    // explicit multi-argument

    //    {
    //        NonTrivial nt = {std::string("15"), 15ull};
    //        AutoObject<NonTrivial> oo = {std::string("15"), 15ull};
    //    }

    {
        NonTrivial nt{std::string("15"), 15ull};
        AutoObject<NonTrivial> oo{std::string("15"), 15ull};
        std::cout << std::endl;
    }

    // explicit multi-argument with argument conversion

    //    {
    //        NonTrivial nt = {"15", 15ull};
    //        AutoObject<NonTrivial> oo = {"15", 15ull};
    //    }

    {
        NonTrivial nt{"15", 15ull};
        AutoObject<NonTrivial> oo{"15", 15ull};
        std::cout << std::endl;
    }
}

void Conversions() {
    // check conversions

    {
        NonTrivial nt;
        AutoObject<NonTrivial> oo;

        {
            bool ntb = nt;
            bool oob = oo.Get();

            std::cout << ntb && oob;
        }

        {
            auto ntb = static_cast<bool>(nt);
            auto oob = static_cast<bool>(oo.Get());

            std::cout << ntb && oob;
        }
    }

    {
        NonTrivial nt;
        AutoObject<NonTrivial> oo;

        //        {
        //            std::string nts = nt;
        //            std::string oos = oo;
        //        }

        {
            auto nts = static_cast<std::string>(nt);
            auto oos = static_cast<std::string>(oo.Get());

            std::cout << nts + oos;
        }
    }
}

void Assignments() {
    // check assignments

    {
        AutoObject<NonTrivial> oo{1ull};
        AutoObject<NonTrivial> oo2{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(oo.Get()) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo2.Get()) << std::endl;

        oo = oo2;

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(oo.Get()) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo2.Get()) << std::endl;

        std::cout << std::endl;
    }

    {
        AutoObject<NonTrivial> oo{1ull};
        AutoObject<NonTrivial> oo2{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(oo.Get()) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo2.Get()) << std::endl;

        oo = std::move(oo2);

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(oo.Get()) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo2.Get()) << std::endl;

        std::cout << std::endl;
    }

    {
        NonTrivial nt{1ull};
        AutoObject<NonTrivial> oo{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        oo = nt;

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        std::cout << std::endl;
    }

    {
        NonTrivial nt{1ull};
        AutoObject<NonTrivial> oo{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        oo = std::move(nt);

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        std::cout << std::endl;
    }

    {
        NonTrivial nt{1ull};
        AutoObject<NonTrivial> oo{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        nt = oo.Get();

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        std::cout << std::endl;
    }

    {
        NonTrivial nt{1ull};
        AutoObject<NonTrivial> oo{2ull};

        std::cout << "Start:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        nt = std::move(oo.Get());

        std::cout << "End:" << std::endl;
        std::cout << "    first:  " << static_cast<size_t>(nt) << std::endl;
        std::cout << "    second: " << static_cast<size_t>(oo.Get()) << std::endl;

        std::cout << std::endl;
    }
}

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

/*

void Test1() {
    TRY {
        TRY {
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught 1" << std::endl;

            TRY {
                TRY {
                    Rethrow();
                }
                CATCH(except::Type::A) {
                    std::cout << "caught 2" << std::endl;
                    Rethrow();
                }
            }
            CATCH(except::Type::A) {
                std::cout << "caught 3" << std::endl;
            }

            Rethrow();
        }
    }
    CATCH(except::Type::A) {
        std::cout << "caught 4" << std::endl;
    }
}

void Test2() {
    TRY {
        TRY {
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught 1" << std::endl;

            TRY {
                Throw(except::Type::B);
            }
            CATCH(except::Type::B) {
                std::cout << "caught 2" << std::endl;

                Rethrow();
            }
        }
    }
    CATCH(except::Type::B) {
        std::cout << "caught 3" << std::endl;
    }
}

void Test3() {
    TRY {
        TRY {
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught 1" << std::endl;
            Throw(except::Type::B);
        }
    }
    CATCH(except::Type::B) {
        std::cout << "caught 2" << std::endl;
    }
}

void Test4() {
    TRY {
        TRY {
            TRY {
                TRY {
                    Throw(except::Type::A);
                }
                CATCH(except::Type::A) {
                    std::cout << "caught 1" << std::endl;
                    Throw(except::Type::B);
                }
            }
            CATCH(except::Type::B) {
                std::cout << "caught 2" << std::endl;
                Rethrow();
            }
        }
        CATCH(except::Type::B) {
            TRY {
                Throw(except::Type::A);
            }
            CATCH(except::Type::A) {
                std::cout << "caught 3" << std::endl;
            }

            Rethrow();
        }
    }
    CATCH(except::Type::B) {
        std::cout << "caught 4" << std::endl;
    }
}

void Test5() {
    TRY {
        TRY {
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            TRY {
                TRY {
                    TRY {
                        Throw(except::Type::B);
                    }
                    CATCH(except::Type::B) {
                        std::cout << "caught 1" << std::endl;
                        Rethrow();
                    }
                }
                CATCH(except::Type::A) {
                    // should not enter
                    std::cout << "unexpected 1" << std::endl;
                }
            }
            CATCH(except::Type::A) {
                // should not enter
                std::cout << "unexpected 2" << std::endl;
            }
        }
    }
    CATCH(except::Type::B) {
        std::cout << "caught 2" << std::endl;
    }
}

void Test6() {
    TRY {
        // OK
    }
    CATCH(except::Type::A) {
        std::cout << "unexpected 1" << std::endl;
    }
    CATCH() {
        std::cout << "unexpected 2" << std::endl;
    }
}

void Test7() {
    TRY {
        Throw(except::Type::A);
    }
    CATCH(except::Type::A) {
        std::cout << "caught 1" << std::endl;

        TRY {
            // OK
        }
        CATCH(except::Type::B) {
            std::cout << "unexpected 1" << std::endl;
        }
    }
}

*/

struct A : except::Exception {};

struct C {};

struct B : A, C {};

void Test() {
    TRY {
        TRY {
            Throw(size_t(5));
        } CATCH (int&, aaa) {
            std::cout << "(1): caught int: " << aaa << std::endl;
            aaa = 10;
            Throw();
        } CATCH(size_t&, aaa) {
            std::cout << "(1): caught size_t: " << aaa << std::endl;
            aaa = 10;
            Throw();
        }
    } CATCH (int&, aaa) {
        std::cout << "(2): caught int: " << aaa << std::endl;
    } CATCH(size_t&, aaa) {
        std::cout << "(2): caught size_t: " << aaa << std::endl;
    }
}

void TestCPP() {
    try {
        try {
            throw size_t(5);
        } catch (int& aaa) {
            std::cout << "(1): caught int: " << aaa << std::endl;
            aaa = 10;
            throw;
        } catch (size_t& aaa) {
            std::cout << "(1): caught size_t: " << aaa << std::endl;
            aaa = 10;
            throw;
        }
    } catch (int& aaa) {
        std::cout << "(2): caught int: " << aaa << std::endl;
    } catch (size_t& aaa) {
        std::cout << "(2): caught size_t: " << aaa << std::endl;
    }
}

int main() {
//    Test1();
//    Test2();
//    Test3();
//    Test4();
//    Test5();
//    Test6();
    Test();
    TestCPP();
    return 0;
}
