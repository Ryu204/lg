#include <iostream>

#include <stay_editor/editor.hpp>

int main() {
    try {
        stay::editor::Editor editor;
        editor.run();
        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown error\n";
        return -2;
    }
}