#include <iostream>

#include "../include/except"

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
        Throw(except::Type::A);
    }
    CATCH(except::Type::A) {
        std::cout << "caught exception A in Bar" << std::endl;

        AutoObject<NonTrivial> object2;

        TRY {
            Baz();
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught inner exception A in Bar" << std::endl;

            Baz();
            AutoObject<NonTrivial> object3;
        }

        Throw(except::Type::C);
    }
}

void Foo() {
    TRY {
        Bar();
    }
    CATCH(except::Type::B) {
        std::cout << "caught exception B in Foo" << std::endl;

        TRY {
            Baz();
            Throw(except::Type::A);
        }
        CATCH(except::Type::A) {
            std::cout << "caught inner exception A in Foo" << std::endl;

            Baz();
            AutoObject<NonTrivial> object3;
        }
    }

    Throw(except::Type::D);
}

int main() {
    Foo();
    return 0;
}
