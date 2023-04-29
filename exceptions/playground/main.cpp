#include <iostream>

#include <except>

struct NonTrivial {
    NonTrivial()
        : index_(index++) {
        std::cout << "hi from constructor #" << index_ << std::endl;
    }
    ~NonTrivial() {
        std::cout << "hi from destructor #" << index_ << std::endl;
    }

    NonTrivial(const NonTrivial&)            = default;
    NonTrivial& operator=(const NonTrivial&) = default;

    NonTrivial(NonTrivial&&)            = default;
    NonTrivial& operator=(NonTrivial&&) = default;

   private:
    size_t index_;
    static size_t index;
};

size_t NonTrivial::index = 0;

void Baz() {
    AutoObject<NonTrivial> object;
}

void Bar() {
    AutoObject<NonTrivial> object;

    TRY {
        AutoObject<NonTrivial> object2;
        THROW(except::Type::A);
    }
    CATCH(except::Type::A) {
        std::cout << "caught exception A in Bar" << std::endl;

        AutoObject<NonTrivial> object2;

        TRY {
            Baz();
            THROW(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught inner exception A in Bar" << std::endl;

            Baz();
            AutoObject<NonTrivial> object3;
        }
        ENDTRY;

        THROW(except::Type::C);
    }
    ENDTRY;
}

void Foo() {
    TRY {
        Baz();
        Bar();
    }
    CATCH(except::Type::B) {
        std::cout << "caught exception B in Foo" << std::endl;

        TRY {
            Baz();
            THROW(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught inner exception A in Foo" << std::endl;

            Baz();
            AutoObject<NonTrivial> object3;
        }
        ENDTRY;
    }
    CATCH_ALL {
        std::cout << "caught unknown exception in Foo" << std::endl;
    }
    ENDTRY;

    THROW(except::Type::D);
}

int main() {
    Foo();
    return 0;
}
