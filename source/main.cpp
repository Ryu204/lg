#include <exception>
#include <iostream>

#include <stay/program/application.hpp>

int main() {
    try {
        stay::program::Application app;
        app.run();
        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return -1;
    } catch (...) {
        return -2;
    }
}