#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <getopt.h>
#include <optional>

#include "config.h"

std::optional<OutputFormat> optarg_to_format() {
    if (!optarg) return std::nullopt;

    if (strcmp(optarg, "waybar") == 0) return format_waybar;
    if (strcmp(optarg, "custom") == 0) return format_custom;
    if (strcmp(optarg, "raw") == 0) return format_raw;

    return std::nullopt;
}


std::size_t replace(std::string &s, const std::string &search, const std::string &replace, size_t pos=0) {
    pos = s.find(search, pos);

    if(pos == std::string::npos)
        return std::string::npos;
    // Replace by erasing and inserting
    s.replace(pos, search.length(), replace);
    
    return pos;
}

void replace_all(std::string &s, const std::string &search, const std::string &rep) {
    for(size_t pos = 0; ; pos += rep.length()) {
        if ((pos = replace(s, search, rep, pos)) == std::string::npos)
            break;
    }
}

// Help menu
void help(char *name) {
    std::cout
        << "Usage: " << name << " -f [format] [-o [format]] [-e]\n"
        << "-f/--format [format]    valid options: waybar, custom, raw (default: raw)\n"
        << "-o/--output [format]    the custom format to be used if -f is 'custom',\n"
        << "                        this must be a string, and you can include use the following variables:\n"
        << "                        {icon}, {percentage}, {name}.\n"
        << "                        please note that the 'icon' variable will be taken from the predefined\n"
        << "                        list of fontawesome icons.\n"
        << "-i/--icons [list]       comma-separated list of icons, one of which will be used\n"
        << "                        in proportion to the device's percentage as the {icon}\n"
        << "                        parameter in '--output'\n"
        << "                        example: ',,,,'\n"
        << "-h/--help               show this help screen\n"
        << "-e/--dont-follow        output info and exit" << std::endl;
}

void incorrect_format_usage(char *name) {
    std::cerr << "Can't use '--format custom' without the --output and --icons parameters.\n"
        << "Example of correct usage: " << name << " --format custom --output '{icon} {percentage}' --icons ',,,,'" << std::endl;
}

std::vector<std::string> split(char* haystack, const char delim) {
    std::vector<std::string> tokens;
    std::string item;
    std::stringstream ss(haystack);
    while (std::getline(ss, item, delim)) {
        tokens.push_back(std::move(item));
    }

    return tokens;
}