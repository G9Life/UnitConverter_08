#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr const char* kPromptPrefix = "Insert value for converting";

const std::vector<std::string> kScenarios = {
    "meter:2.5",
    "feet:1.0",
    "yard:1.0",
    "meter:0.0",
};

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path, std::ios::trunc);
    REQUIRE(out.good());
    out << content;
}

std::string normalizeNewlines(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                ++i;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(text[i]);
        }
    }
    return normalized;
}

std::string stripPromptPrefixFromLine(std::string line) {
    if (line.rfind(kPromptPrefix, 0) != 0) {
        return line;
    }
    const std::string marker = "): ";
    const auto markerPos = line.find(marker);
    if (markerPos == std::string::npos) {
        return {};
    }
    return line.substr(markerPos + marker.size());
}

std::string stripPromptLine(const std::string& stdoutText) {
    std::string result;
    std::istringstream stream(stdoutText);
    std::string line;
    while (std::getline(stream, line)) {
        const std::string stripped = stripPromptPrefixFromLine(line);
        if (stripped.empty()) {
            continue;
        }
        if (!result.empty()) {
            result.push_back('\n');
        }
        result += stripped;
    }
    if (!result.empty() && result.back() != '\n') {
        result.push_back('\n');
    }
    return result;
}

std::string captureStdoutForScenario(const std::filesystem::path& exe,
                                     const std::string& scenario) {
    const auto tempDir = std::filesystem::temp_directory_path() / "unit_converter_golden";
    std::filesystem::create_directories(tempDir);

    const auto inputPath = tempDir / "input.txt";
    const auto outputPath = tempDir / "actual.txt";

    {
        std::ofstream input(inputPath, std::ios::trunc);
        REQUIRE(input.good());
        input << scenario << '\n';
    }

    std::ostringstream command;
#ifdef _WIN32
    command << "cmd /c \"\"" << exe.string() << "\" < \""
            << inputPath.string() << "\" > \""
            << outputPath.string() << "\" 2>nul\"";
#else
    command << '"' << exe.string() << "\" < \""
            << inputPath.string() << "\" > \""
            << outputPath.string() << "\" 2>/dev/null";
#endif

    const int exitCode = std::system(command.str().c_str());
    (void)exitCode;

    return stripPromptLine(readFile(outputPath));
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

std::string buildGoldenDocument(const std::filesystem::path& exe) {
    std::ostringstream document;
    for (std::size_t i = 0; i < kScenarios.size(); ++i) {
        const std::string& scenario = kScenarios[i];
        document << '[' << scenario << "]\n";
        document << captureStdoutForScenario(exe, scenario);
        document << "---\n";
    }
    return document.str();
}

void printUnifiedDiff(const std::string& expected, const std::string& actual) {
    const auto expectedLines = splitLines(expected);
    const auto actualLines = splitLines(actual);
    const std::size_t maxLines = std::max(expectedLines.size(), actualLines.size());

    UNSCOPED_INFO("--- expected (tests/golden_master_expected.txt)");
    UNSCOPED_INFO("+++ actual");
    for (std::size_t i = 0; i < maxLines; ++i) {
        const bool hasExpected = i < expectedLines.size();
        const bool hasActual = i < actualLines.size();
        if (hasExpected && hasActual && expectedLines[i] == actualLines[i]) {
            continue;
        }
        if (hasExpected) {
            UNSCOPED_INFO('-' << expectedLines[i]);
        }
        if (hasActual) {
            UNSCOPED_INFO('+' << actualLines[i]);
        }
    }
}

}  // namespace

TEST_CASE("Golden Master stdout regression", "[golden][regression][r01]") {
#ifdef UNIT_CONVERTER_RED_PHASE
    SKIP("Golden Master requires GREEN build (UNIT_CONVERTER_RED_PHASE=OFF)");
#else
    const std::filesystem::path exePath(UNIT_CONVERTER_EXE);
    const std::filesystem::path expectedPath(GOLDEN_MASTER_EXPECTED);

    INFO("UnitConverter executable: " << exePath.string());
    REQUIRE(std::filesystem::exists(exePath));

    const std::string actual = buildGoldenDocument(exePath);

    if (!std::filesystem::exists(expectedPath)) {
        writeFile(expectedPath, actual);
        WARN("Created baseline: " << expectedPath.string()
             << " — review and commit with: git add tests/golden_master_expected.txt");
        return;
    }

    const std::string expected = normalizeNewlines(readFile(expectedPath));
    const std::string normalizedActual = normalizeNewlines(actual);

    if (expected != normalizedActual) {
        printUnifiedDiff(expected, normalizedActual);
        FAIL("Golden master mismatch — run scripts/generate_golden_master.ps1 to refresh baseline");
    }
#endif
}
