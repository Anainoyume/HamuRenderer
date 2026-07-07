#include "test.h"

int main() {
    if (hamu::initialize()) {
        hamu::update();
        hamu::shutdown();
    }
}