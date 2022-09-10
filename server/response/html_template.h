#ifndef HTML_TEMPLATE_H
#define HTML_TEMPLATE_H

#include <string_view>
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
        std::string_view begin__full;
        std::string_view full__small;
        std::string_view small__descr;
        std::string_view descr__end;
    } picture;
    
    html_template all;
};

template <typename string_type>
struct add_pictures_t
{
    add_pictures_t (string_type & _answer, pictures_template const & _pictures) :
        answer(_answer),
        pictures(_pictures),
        closed(0)
    {
        answer.append(pictures.all.begin);
    }

    ~add_pictures_t ()
    {
        try
        {
            if (!closed)
                close();
        }
        catch (...)
        {}
    }
     
    void operator () (ship_requests::pictures_t::picture const & info)
    {
        answer.append(pictures.picture.begin__full)
              .append(info.path_full)
              .append(pictures.picture.full__small)
              .append(info.path_small)
              .append(pictures.picture.small__descr)
              .append(info.description)
              .append(pictures.picture.descr__end);
    }
    
    void close ()
    {
        answer.append(pictures.all.end);
        closed = 1;
    }

private:
    string_type & answer;
    pictures_template const & pictures;
    bool closed;
};

#endif

