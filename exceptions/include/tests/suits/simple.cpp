#include <gtest/gtest.h>

#include <cstddef>  // for size_t

//
#include <except>

namespace except::test::impl {

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(Simple, Case1) {
    size_t caught = 0;

    TRY {
        TRY {
            Throw(RuntimeError{});
        }
        CATCH(RuntimeError) {
            ++caught;

            TRY {
                TRY {
                    Throw();
                }
                CATCH(RuntimeError) {
                    ++caught;
                    Throw();
                }
            }
            CATCH(RuntimeError) {
                ++caught;
            }

            Throw();
        }
    }
    CATCH(RuntimeError) {
        ++caught;
    }

    ASSERT_EQ(caught, 4);
}

TEST(Simple, Case2) {
    size_t caught = 0;

    TRY {
        TRY {
            Throw(RuntimeError{});
        }
        CATCH(RuntimeError) {
            ++caught;

            TRY {
                Throw(LogicError{});
            }
            CATCH(LogicError) {
                ++caught;
                Throw();
            }
        }
    }
    CATCH(LogicError) {
        ++caught;
    }

    ASSERT_EQ(caught, 3);
}

TEST(Simple, Case3) {
    size_t caught = 0;

    TRY {
        TRY {
            Throw(RuntimeError{});
        }
        CATCH(RuntimeError) {
            ++caught;
            Throw(LogicError{});
        }
    }
    CATCH(LogicError) {
        ++caught;
    }

    ASSERT_EQ(caught, 2);
}

TEST(Simple, Case4) {
    size_t caught = 0;

    TRY {
        TRY {
            TRY {
                TRY {
                    Throw(RuntimeError{});
                }
                CATCH(RuntimeError) {
                    ++caught;
                    Throw(LogicError{});
                }
            }
            CATCH(LogicError) {
                ++caught;
                Throw();
            }
        }
        CATCH(LogicError) {
            TRY {
                Throw(RuntimeError{});
            }
            CATCH(RuntimeError) {
                ++caught;
            }

            Throw();
        }
    }
    CATCH(LogicError) {
        ++caught;
    }

    ASSERT_EQ(caught, 4);
}

TEST(Simple, Case5) {
    size_t caught = 0;

    TRY {
        TRY {
            Throw(RuntimeError{});
        }
        CATCH(RuntimeError) {
            TRY {
                TRY {
                    TRY {
                        Throw(LogicError{});
                    }
                    CATCH(LogicError) {
                        ++caught;
                        Throw();
                    }
                }
                CATCH(RuntimeError) {
                    // should not enter
                    ASSERT_TRUE(false);
                }
            }
            CATCH(RuntimeError) {
                // should not enter
                ASSERT_TRUE(false);
            }
        }
    }
    CATCH(LogicError) {
        ++caught;
    }

    ASSERT_EQ(caught, 2);
}

TEST(Simple, Case6) {
    TRY {
        // OK
    }
    CATCH(RuntimeError) {
        // should not enter
        ASSERT_TRUE(false);
    }
    CATCH() {
        // should not enter
        ASSERT_TRUE(false);
    }
}

TEST(Simple, Case7) {
    size_t caught = 0;

    TRY {
        Throw(RuntimeError{});
    }
    CATCH(RuntimeError) {
        ++caught;

        TRY {
            // OK
        }
        CATCH(LogicError) {
            // should not enter
            ASSERT_TRUE(false);
        }
    }

    ASSERT_EQ(caught, 1);
}

TEST(Simple, Case8) {
    size_t caught = 0;

    TRY {
        TRY {
            TRY {
                TRY {
                    Throw(Exception{});
                }
                CATCH(Exception) {
                    ++caught;
                    Throw();
                }
            }
            CATCH(Exception) {
                ++caught;
                // OK
            }

            Throw(Exception{});
        }
        CATCH(Exception) {
            ++caught;

            TRY {
                // OK
            }
            CATCH() {
                // should not enter
                ASSERT_TRUE(false);
            }
        }

        Throw(Exception{});
    }
    CATCH(Exception) {
        ++caught;
    }

    ASSERT_EQ(caught, 4);
}

// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace except::test::impl
