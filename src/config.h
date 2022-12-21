#pragma once
#include <vector>
#include <string>

enum OutputFormat {
    format_raw_default, // when its the default value
    format_raw,
    format_waybar,
    format_custom
};

struct ProgramOptions {
    std::vector<std::string> icons;
    OutputFormat output_format = format_raw_default;
    char *custom_format;
    bool dont_follow;
};

extern int parse_opts(int argc, char *const argv[], ProgramOptions *opts);