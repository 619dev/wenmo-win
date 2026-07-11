#include "wenmo/InputEngine.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace wenmo {

InputEngine::InputEngine(const std::filesystem::path& dictionary_path) {
    std::ifstream input(dictionary_path);
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }

        std::istringstream row(line);
        std::string key;
        Candidate candidate;
        if (!std::getline(row, key, '\t') ||
            !std::getline(row, candidate.simplified, '\t') ||
            !std::getline(row, candidate.traditional)) {
            continue;
        }

        auto& entries = index_[key];
        if (entries.size() < 48) {
            entries.push_back(std::move(candidate));
        }
    }
}

void InputEngine::type(char character) {
    if (character >= 'a' && character <= 'z') {
        composition_.push_back(character);
    }
}

void InputEngine::backspace() {
    if (!composition_.empty()) {
        composition_.pop_back();
    }
}

void InputEngine::clear() { composition_.clear(); }

const std::string& InputEngine::composition() const noexcept {
    return composition_;
}

std::vector<std::string> InputEngine::candidates() const {
    const auto found = index_.find(composition_);
    if (found == index_.end()) {
        return {};
    }

    std::vector<std::string> result;
    result.reserve(found->second.size());
    for (const auto& candidate : found->second) {
        result.push_back(script_ == Script::Simplified
                             ? candidate.simplified
                             : candidate.traditional);
    }
    return result;
}

void InputEngine::set_script(Script script) noexcept { script_ = script; }
InputEngine::Script InputEngine::script() const noexcept { return script_; }

}  // namespace wenmo

