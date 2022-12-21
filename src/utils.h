#pragma once
#include <vector>
#include <string>
#include <optional>

#include "config.h"


extern std::optional<OutputFormat> optarg_to_format();
extern int replace(std::string &s, const std::string &search, const std::string &replace, size_t pos=0);
extern void replace_all(std::string &s, const std::string &search, const std::string &rep);
extern void help(char *name);
extern void incorrect_format_usage(char *name);
extern std::vector<std::string> split(char* haystack, const char delim);
