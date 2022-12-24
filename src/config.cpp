#include <iostream>
#include <getopt.h>
#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include <algorithm>

#include "config.h"
#include "utils.h"

#define get_config_file(config_home) config_home / "cppbtbl" / "config"

static struct option long_options[] = {
    {"help",              no_argument,       NULL, 'h'},
    {"format",            required_argument, NULL, 'f'},
    {"output",            required_argument, NULL, 'o'},
    {"icons",             required_argument, NULL, 'i'},
    {"devices-separator", required_argument, NULL, 's'},
    {"dont-follow",       no_argument,       NULL, 'e'},
};


std::optional<std::filesystem::path> get_config_home() {
    if (char *home = std::getenv("XDG_CONFIG_HOME"); home != nullptr)
        return home;

    if (char *home = std::getenv("HOME"); home != nullptr)
        return std::filesystem::path(home) / ".config";


    std::cerr << "Unable to get config home. Ignoring config." << std::endl;
    return std::nullopt;
}

int _parse_format(ProgramOptions *opts) {
    auto output_format = optarg_to_format();
    if (!output_format) {
        std::cerr << "Invalid format!\n"
            << "Valid values are: waybar, custom, raw." << std::endl;

        return 1;
    }

    opts->output_format = *output_format;
    return -1;
}

int parse_config(ProgramOptions *opts) {
    auto config_home = get_config_home();
    if (!config_home) return -1;

    std::filesystem::path config_file = get_config_file(*config_home);

    // std::cerr << "[DEBUG] config_file_path = " << config_file << std::endl;

    if (!std::filesystem::exists(config_file))
        return -1;
    
    std::ifstream file(config_file);
    std::string line;
    
    // first arg is program name, we don't need it
    std::vector<const char*> _argv = { "cppbtbl" };
    while (std::getline(file, line)) {
        line.insert(0, "--");

        const char *copy = strdup(line.c_str());
        // std::cerr << static_cast<const void *>(line.c_str()) << ": " << line << std::endl;
        _argv.push_back(std::move(copy));
    }

    // std::cerr << "[DEBUG] _argv.size() = " << _argv.size() << std::endl;

    // reset getopt
    optind = 1;
    int opt;
    while ((opt = getopt_long_only(
        _argv.size(),
        const_cast<char *const *>(_argv.data()),
        "ef:o:i:",
        long_options,
        nullptr
    )) != -1) {
        //std::cerr << "[DEBUG] getopt_long_only opt=" << (char)opt << ", optarg=" << optarg << std::endl;
        switch(opt) {
            case 'f':
                if (opts->output_format != format_raw_default) break;
                if (int e = _parse_format(opts); e != -1)
                    return e;
                break;
            case 'o':
                if (opts->custom_format != nullptr) break; 
                // i need a copy here, because the pointer gets free'd later on
                opts->custom_format = strdup(optarg);
                break;
            case 'e':
                opts->dont_follow = true;
                break;
            case 'i':
                if (!opts->icons.empty()) break;
                opts->icons = split(optarg, ',');
                break;
            case 's':
                if (opts->devices_separator != nullptr) break;
                opts->devices_separator = strdup(optarg);
            default:
                return 1;
        }
    }
    
    // clear all duplicated strings
    std::for_each(
        ++_argv.begin(),
        _argv.end(),
        [](const char *ptr) {
            std::free((void *)ptr);
        }
    );

    return -1;
}

int parse_opts(int argc, char *const argv[], ProgramOptions *opts) {
    // reset getopt
    optind = 1;
    int opt;
    while ((opt = getopt_long(argc, argv, "hef:o:i:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                help(argv[0]);
                return 0;
            case 'f':
                if (int e = _parse_format(opts); e != -1)
                    return e;
                break;
            case 'o':
                opts->custom_format = std::move(optarg);
                break;
            case 'e':
                opts->dont_follow = true;
                break;
            case 'i':
                opts->icons = split(optarg, ',');
                break;
            case 's':
                opts->devices_separator = std::move(optarg);
            default:
                return 1;
        }
    }
    if (int e = parse_config(opts); e != -1) {
        std::cerr << "Error occurred while parsing config." << std::endl;
        return e;
    }

    if (opts->output_format == format_custom and
        (!opts->custom_format or opts->icons.empty())) {
        incorrect_format_usage(argv[0]);
        return 1;
    }

    return -1;
}