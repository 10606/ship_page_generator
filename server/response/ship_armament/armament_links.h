#ifndef ARMAMENT_LINKS_H
#define ARMAMENT_LINKS_H

#include <string>
#include <string_view>
#include "html_template.h"

namespace armament_links
{
    static const constexpr html_template_3 link_template = {"<a href=\"", "\">", "</a>"};

    inline std::string filtered (std::string_view link, std::string_view text, size_t class_id)
    {
        return std::string(link_template.begin)
                .append(link)
                .append("&filter=class,")
                .append(std::to_string(class_id))
                .append(link_template.middle)
                .append(text)
                .append(link_template.end);
    }
    
    inline std::string base (std::string_view link, std::string_view text)
    {
        return std::string(link_template.begin)
                .append(link)
                .append(link_template.middle)
                .append(text)
                .append(link_template.end);
    }
}

#endif

