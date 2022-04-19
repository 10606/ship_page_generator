#ifndef UPGRADE_TO_HTTPS_H
#define UPGRADE_TO_HTTPS_H

#include "mongoose.h"
#include <string>

bool upgrade_if_need (mg_connection * nc, mg_http_message * http_msg, std::string const & port) noexcept;

#endif

