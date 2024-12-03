#include "main.hpp"
#include <iostream>
Engine engine(80,50);

int main() {
    engine.load();
    while (!TCODConsole::isWindowClosed() ) {
        engine.update();
        engine.render();
        //std::cout << "Before flush" << std::endl;
        TCODConsole::flush();
        //std::cout << "After flush" << std::endl;
    }
    engine.save();
    return 0;
}
