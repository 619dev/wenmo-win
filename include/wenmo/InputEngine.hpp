#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace wenmo {

struct Candidate {
    std::string simplified;
    std::string traditional;

    bool operator==(const Candidate&) const = default;
};

class InputEngine {
public:
    enum class Script { Simplified, Traditional };

    explicit InputEngine(const std::filesystem::path& dictionary_path);

    void type(char character);
    void backspace();
    void clear();

    [[nodiscard]] const std::string& composition() const noexcept;
    [[nodiscard]] std::vector<std::string> candidates() const;

    void set_script(Script script) noexcept;
    [[nodiscard]] Script script() const noexcept;

private:
    std::string composition_;
    Script script_ = Script::Simplified;
    std::unordered_map<std::string, std::vector<Candidate>> index_;
};

}  // namespace wenmo

