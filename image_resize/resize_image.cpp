#include <Magick++.h>
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;


void convert_image (fs::path src_path, fs::path dst_path)
{
    try
    {
        Magick::Image image;
        image.read(src_path.c_str());
        image.resize(Magick::Geometry(240, 240));
        image.write(dst_path.c_str());
    }
    catch (Magick::Exception & e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {}
}


void convert_dir (fs::path src_root, fs::path dst_root, bool force)
{
    fs::create_directories(dst_root);
    for (fs::directory_entry const & dir_entry : fs::recursive_directory_iterator(src_root))
    {
        fs::path src_path = dir_entry.path();
        fs::path relative = fs::relative(src_path, src_root);
        fs::path dst_path = dst_root / relative;
        fs::directory_entry dst_entry(dst_path);
        if (!force && dst_entry.exists())
        {
            std::cout << dst_path << " exist\n";
            continue;
        }
        if (dir_entry.is_regular_file())
        {
            std::cout << src_path << " -> " << dst_path << '\n';
            convert_image(src_path, dst_path);
        }
        if (dir_entry.is_directory())
        {
            std::cout << "mkdir " << dst_path << '\n';
            fs::create_directory(dst_path);
        }
    }
}


int main (int argc, char * argv[])
{
    try
    {
        if (argc < 3 || argc > 4) {
            std::cout << argv[0] << " [-f] <src> <dst>";
            return 1;
        }

        size_t args_it = 1;
        bool force = 0;

        if (argc == 4)
        {
            args_it++;
            force |= std::string_view(argv[1]) == "-f";
            force |= std::string_view(argv[1]) == "-force";
        }

        fs::path src_path(argv[1]);
        fs::path dst_path(argv[2]);

        Magick::InitializeMagick(argv[0]);

        convert_dir(src_path, dst_path, force);
    }
    catch (std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    catch (...)
    {
        std::cerr << "unknown error" << std::endl;
        return 3;
    }
}

