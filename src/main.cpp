#include "app/App.h"

#include <cstdio>

int main()
{
    App app;
    if (!app.init()) {
        std::fprintf(stderr, "[main] App initialisation failed – aborting.\n");
        return 1;
    }
    app.run();
    return 0;
}
