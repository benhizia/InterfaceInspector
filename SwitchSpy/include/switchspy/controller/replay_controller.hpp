#pragma once
#include <string>
namespace switchspy { namespace controller {
class ReplayController {
public:
    void startReplay(const std::string& filename);
    void stopReplay();
};
}} // namespace
