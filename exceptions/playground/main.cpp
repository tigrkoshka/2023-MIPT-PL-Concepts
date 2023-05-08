#include <except>
#include <iostream>

int main() {
    TRY {
        Throw("Hello, world!");
    }
    CATCH(const char*, message) {
        std::cout << message;
    }

    return 0;
}
