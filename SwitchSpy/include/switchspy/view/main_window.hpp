#pragma once
#include <string>

namespace switchspy { namespace view {
class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    bool initialize();
    void run();
    void shutdown();
private:
    void render();
    void renderMenuBar();
    void renderStatusBar();
    bool running_;
    void* window_;
};
}} // namespace
