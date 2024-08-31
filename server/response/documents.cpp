#include "documents.h"

#include "html_template.h"


using doc_page_template_t = html_template;

struct group_template_t
{
    html_template self; // open close group
    html_template preview;
    std::string_view without_preview;
    html_template description;
    html_template document_list;
};

struct document_template_t
{
    html_template self; // open close document
    html_template preview;
    std::string_view without_preview;
    html_template description;
    html_template link;
};

document::document (ship_requests * database)
{
    std::vector <group_t> group_list = database->documents.get_groups("order by id");
    std::vector <document_t> document_list = database->documents.get_documents("order by (group_id, priority) nulls last");
    
    std::map <int, std::pair <group_t, std::vector <document_t> > > documents_map;
    for (group_t & group : group_list)
        documents_map.insert({group.id, {std::move(group), {}}});
    for (document_t & document : document_list)
    {
        std::map <int, std::pair <group_t, std::vector <document_t> > > ::iterator it = 
            documents_map.find(document.group_id);
        if (it != documents_map.end())
            it->second.second.push_back(std::move(document));
    }
    
    static const constexpr doc_page_template_t doc_page_template = 
    {
            "<style>\n"
                ".main .document br {}\n"
                ".main .document {\n"
                    "min-width: 0px;\n"
                    "list-style-type: none;\n"
                "}\n"
                ".main .document_group { min-width: 1200px; }\n"
            "</style>\n"
            "<script>\n"
                "function toggle_preview (event)\n"
                "{\n"
                    "for (style_sheet of document.styleSheets)\n"
                    "{\n"
                        "for (css_rule of style_sheet.cssRules)\n"
                        "{\n"
                            "var selector = css_rule.selectorText;\n"
                            "var style_map = css_rule.styleMap\n;"
                            "if (!event.target.checked)\n"
                            "{\n"
                                "if (selector == \".main img\")\n"
                                    "style_map.set(\"display\", \"none\");\n"
                                "if (selector == \".main .document br\")\n"
                                    "style_map.set(\"display\", \"none\");\n"
                                "if (selector == \".main li\")\n"
                                    "style_map.set(\"max-width\", \"none\");\n"
                                "if (selector == \".main .document\")\n"
                                    "style_map.set(\"display\", \"list-item\");\n"
                            "}\n"
                            "else\n"
                            "{\n"
                                "if (selector == \".main img\")\n"
                                    "style_map.set(\"display\", \"inline\");\n"
                                "if (selector == \".main .document br\")\n"
                                    "style_map.set(\"display\", \"block\");\n"
                                "if (selector == \".main li\")\n"
                                    "style_map.set(\"max-width\", \"180px\");\n"
                                "if (selector == \".main .document\")\n"
                                    "style_map.set(\"display\", \"inline-block\");\n"
                            "}\n"
                        "}\n"
                    "}\n"
                    "event.target.nextSibling.textContent = "
                        "(!event.target.checked)? \" превью: выключено \" : \" превью: включено \";\n"
                "}\n"
            "</script>\n"
        "<div>\n"
            "<ul>\n",
            "</ul>\n"
        "</div>\n",
    };
    
    static const constexpr group_template_t group_template = 
    {
        .self = {"<li class=\"document_group\">", "</li><br>"},
        .preview = {"<img src=\"/pictures_small/docs/", "\">"},
        .without_preview = "",
        .description = {"<h2>", "</h2>"},
        .document_list = {"<ul>", "</ul>"}
    };
    
    static const constexpr document_template_t document_template = 
    {
        .self = {"<li class=\"document\">", "</a></li>"},
        .preview = {"<img src=\"/pictures_small/docs/", "\">"},
        .without_preview = "",
        .description = {"<br>", ""},
        .link = {"<a href=\"/documents/", "\">"}
    };
    
    value.append(doc_page_template.begin);
    for (auto const & group : documents_map)
    {
        value.append(group_template.self.begin);
        
        group_t const & group_description = group.second.first;
        if (group_description.path_preview)
        {
            value.append(group_template.preview.begin);
            value.append(*group.second.first.path_preview);
            value.append(group_template.preview.end);
        }
        else
            value.append(group_template.without_preview);
        
        value.append(group_template.description.begin);
        value.append(group_description.description);
        value.append(group_template.description.end);
        
        value.append(group_template.document_list.begin);
        for (document_t const & document : group.second.second)
        {
            value.append(document_template.self.begin);
            
            value.append(document_template.link.begin);
            value.append(document.path_document);
            value.append(document_template.link.end);
            
            if (document.path_preview)
            {
                value.append(document_template.preview.begin);
                value.append(*document.path_preview);
                value.append(document_template.preview.end);
            }
            else
                value.append(document_template.without_preview);
            
            value.append(document_template.description.begin);
            value.append(document.description);
            value.append(document_template.description.end);
            
            value.append(document_template.self.end);
        }
        value.append(group_template.document_list.end);
        
        value.append(group_template.self.end);
    }
    value.append(doc_page_template.end);
}

void document::response (simple_string & answer, std::string_view query, piece_t title)
{
    static const constexpr std::string_view title_text = "документы";
    answer.rewrite(title.position, title_text.substr(0, std::min(title_text.size(), title.size)));
    answer.append(value);
}


