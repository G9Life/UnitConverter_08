#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr const char* kPromptPrefix = "Insert value for converting";

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
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

std::string pathForShell(const std::filesystem::path& path) {
    return path.generic_string();
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
        if (!input) {
            return {};
        }
        input << scenario << '\n';
    }

    const std::string exeArg = pathForShell(exe);
    const std::string inputArg = pathForShell(inputPath);
    const std::string outputArg = pathForShell(outputPath);

    std::ostringstream command;
#ifdef _WIN32
    command << "cmd /c \"\"" << exeArg << "\" < \""
            << inputArg << "\" > \""
            << outputArg << "\" 2>nul\"";
#else
    command << '"' << exeArg << "\" < \""
            << inputArg << "\" > \""
            << outputArg << "\" 2>/dev/null";
#endif

    (void)std::system(command.str().c_str());
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

void printUnifiedDiff(const std::string& expected, const std::string& actual) {
    const auto expectedLines = splitLines(expected);
    const auto actualLines = splitLines(actual);
    const std::size_t maxLines = std::max(expectedLines.size(), actualLines.size());

    UNSCOPED_INFO("--- expected");
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

std::string extractSectionBody(const std::string& document, const std::string& scenario) {
    const std::string header = '[' + scenario + ']';
    const auto headerPos = document.find(header);
    if (headerPos == std::string::npos) {
        return {};
    }

    auto bodyStart = headerPos + header.size();
    if (bodyStart < document.size() && document[bodyStart] == '\r') {
        ++bodyStart;
    }
    if (bodyStart < document.size() && document[bodyStart] == '\n') {
        ++bodyStart;
    }

    const auto sectionEnd = document.find("\n---", bodyStart);
    if (sectionEnd == std::string::npos) {
        const std::string tail = document.substr(bodyStart);
        if (tail.rfind("---", 0) == 0) {
            return {};
        }
        return tail;
    }
    return document.substr(bodyStart, sectionEnd - bodyStart);
}

std::string trimTrailingNewlines(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }
    return text;
}

void expectEq(const std::string& expected, const std::string& actual,
              const std::string& scenario) {
    const std::string normalizedExpected =
        trimTrailingNewlines(normalizeNewlines(expected));
    const std::string normalizedActual =
        trimTrailingNewlines(normalizeNewlines(actual));
    if (normalizedExpected == normalizedActual) {
        return;
    }
    INFO("Golden Master section [" << scenario << ']');
    printUnifiedDiff(normalizedExpected, normalizedActual);
    FAIL("GM mismatch for [" << scenario << ']');
}

}  // namespace

// Catch2 TEST_CASE_METHOD maps to Google Test TEST_F(GoldenMasterTest, ...).
class GoldenMasterTest {
public:
    GoldenMasterTest()
        : exePath_(UNIT_CONVERTER_EXE),
          expectedPath_(GOLDEN_MASTER_EXPECTED) {
        REQUIRE(std::filesystem::exists(exePath_));
        REQUIRE(std::filesystem::exists(expectedPath_));
        expectedDocument_ = normalizeNewlines(readFile(expectedPath_));
        REQUIRE_FALSE(expectedDocument_.empty());
    }

    void compareScenario(const std::string& scenario) const {
        const std::string expected =
            normalizeNewlines(extractSectionBody(expectedDocument_, scenario));
        const std::string actual =
            normalizeNewlines(captureStdoutForScenario(exePath_, scenario));
        expectEq(expected, actual, scenario);
    }

private:
    std::filesystem::path exePath_;
    std::filesystem::path expectedPath_;
    std::string expectedDocument_;
};

// GM-TC-01: TEST_F(GoldenMasterTest, UnitConverter_meter_2_5)
TEST_CASE_METHOD(GoldenMasterTest, "UnitConverter_meter_2_5", "[golden][regression][r01]") {
    compareScenario("meter:2.5");
}

// GM-TC-02: TEST_F(GoldenMasterTest, UnitConverter_feet_1_0)
TEST_CASE_METHOD(GoldenMasterTest, "UnitConverter_feet_1_0", "[golden][regression][r01]") {
    compareScenario("feet:1.0");
}

// GM-TC-03: TEST_F(GoldenMasterTest, UnitConverter_yard_1_0)
TEST_CASE_METHOD(GoldenMasterTest, "UnitConverter_yard_1_0", "[golden][regression][r01]") {
    compareScenario("yard:1.0");
}

// GM-TC-04: TEST_F(GoldenMasterTest, UnitConverter_meter_0_0)
TEST_CASE_METHOD(GoldenMasterTest, "UnitConverter_meter_0_0", "[golden][regression][r01]") {
    compareScenario("meter:0.0");
}
