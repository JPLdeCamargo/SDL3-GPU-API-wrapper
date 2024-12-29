#include "SDL3_wrapper.h"

using namespace Render;

int main() {
    auto wrapper = Render::SDL3Wrapper("Test", 500, 500, false);
    wrapper.main_loop();
}