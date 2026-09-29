#pragma once
#include <string>
namespace switchspy { namespace controller {
class RecordingController {
public:
    void startRecording(const std::string& name, const std::string& filename);
    void stopRecording(const std::string& name);
};
}} // namespace
