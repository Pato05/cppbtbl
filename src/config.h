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
    OutputFormat output_format = format_raw_default;
    // for custom format
    std::vector<std::string> icons;
    const char *custom_format;
    const char *devices_separator;
    
    bool dont_follow;
};

extern int parse_opts(int argc, char *const argv[], ProgramOptions *opts);