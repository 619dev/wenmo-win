#include "wenmo/InputEngine.hpp"

#include <cassert>
#include <iostream>

int main() {
    wenmo::InputEngine engine(WENMO_TEST_DICTIONARY);

    engine.type('n');
    engine.type('i');
    assert(engine.composition() == "ni");
    assert(!engine.candidates().empty());

    engine.set_script(wenmo::InputEngine::Script::Traditional);
    assert(!engine.candidates().empty());

    engine.backspace();
    assert(engine.composition() == "n");
    engine.clear();
    assert(engine.composition().empty());

    engine.type('A');
    engine.type('1');
    assert(engine.composition().empty());

    std::cout << "wenmo_core_tests passed\n";
}

