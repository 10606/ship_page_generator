#ifndef VALIDATE_PATH_H
#define VALIDATE_PATH_H

#include <string>
#include <string_view>

bool simple_check (std::string_view path);
std::string filesystem_check (std::string_view path);

#endif

