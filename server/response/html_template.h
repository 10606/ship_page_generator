#ifndef HTML_TEMPLATE_H
#define HTML_TEMPLATE_H

#include <string_view>
#include "simple_string.h"
#include "ship_requests.h"
#include "pictures.h"

struct html_template
{
    std::string_view begin;
    std::string_view end;
};

struct html_template_3
{
    std::string_view begin;
    std::string_view middle;
    std::string_view end;
};

struct pictures_template
{
    struct picture_template
    {
        constexpr picture_template
        (
            std::string_view _begin__full,
            std::string_view _full__small,
            std::string_view _small__descr = "\" onclick=view_pictures(event)></a><br>",
            std::string_view _descr__end   = "</li>\n"
        ) :
            begin__full (_begin__full),
            full__small (_full__small),
            small__descr(_small__descr),
            descr__end  (_descr__end)
        {}
            
        std::string_view begin__full;
        std::string_view full__small;
        std::string_view small__descr;
        std::string_view descr__end;
    } picture;
    
    html_template_3 group;
    
    constexpr pictures_template
    (
        picture_template _picture,
        html_template_3 _group = 
        {
            "",
            "<ul>",
            "</ul><br>"
        }
    ) :
        picture(_picture),
        group(_group)
    {}
};

template <typename string_type>
struct add_pictures_t
{
    add_pictures_t (string_type & _answer, pictures_template const & _pictures) :
        answer(_answer),
        pictures(_pictures),
        closed(0),
        have_head(0)
    {}

    ~add_pictures_t ()
    {
        try
        {
            close();
        }
        catch (...)
        {}
    }
     
    void operator () (ship_requests::pictures_t::picture const & info)
    {
        if constexpr (std::is_same_v <string_type, simple_string> )
        {
            if (!have_head)
                answer.append(pictures.group.begin,
                              pictures.group.middle);
            answer.append
            (
                pictures.picture.begin__full,
                info.path_full,
                pictures.picture.full__small,
                info.path_small,
                pictures.picture.small__descr,
                info.description,
                pictures.picture.descr__end
            );
        }
        else
        {
            if (!have_head)
                answer.append(pictures.group.begin)
                      .append(pictures.group.middle);
            answer.append(pictures.picture.begin__full)
                  .append(info.path_full)
                  .append(pictures.picture.full__small)
                  .append(info.path_small)
                  .append(pictures.picture.small__descr)
                  .append(info.description)
                  .append(pictures.picture.descr__end);
        }
        have_head = 1;
    }
    
    void new_group (std::string_view name)
    {
        if (have_head)
            answer.append(pictures.group.end);
        if constexpr (std::is_same_v <string_type, simple_string> )
        {
            answer.append(pictures.group.begin,
                          name,
                          pictures.group.middle);
        }
        else
        {
            answer.append(pictures.group.begin)
                  .append(name)
                  .append(pictures.group.middle);
        }
        have_head = 1;
    }
    
    void close ()
    {
        if (closed)
            return; // idempotent
        if (have_head) 
            answer.append(pictures.group.end);
        else
            answer.append("<br>");
        closed = 1;
    }

private:
    string_type & answer;
    pictures_template const & pictures;
    bool closed;
    bool have_head;
};

template <typename partial_response, typename text_response>
void add_pictures
(
    simple_string & answer,
    pictures_template const & pictures,
    std::vector <partial_response> const & list, 
    std::vector <std::vector <ship_requests::pictures_t::picture> > pictures_cache,
    std::vector <text_response> text_cache
)
{
    size_t picture_count = 0;
    for (partial_response const & item : list)
        picture_count += pictures_cache[item.index].size();
    bool too_many_pictures = picture_count > 20;
    if (too_many_pictures)
        answer.append("<br>");
    add_pictures_t add_pictures(answer, pictures);
    for (partial_response const & item : list)
    {
        if (too_many_pictures && !pictures_cache[item.index].empty())
            add_pictures.new_group(text_cache[item.index].name);
        for (ship_requests::pictures_t::picture const & picture : pictures_cache[item.index])
            add_pictures(picture);
    }
    add_pictures.close();
}

#endif

