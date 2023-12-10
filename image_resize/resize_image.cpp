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


bool is_exist (fs::path dst_path, bool force)
{
    fs::directory_entry dst_entry(dst_path);
    if (!force && dst_entry.exists())
    {
        std::cout << dst_path << "\033[01;36m exist\033[0m\n";
        return 1;
    }
    return 0;
}


void convert_dir (fs::path src_root, fs::path dst_root, bool force)
{
    fs::recursive_directory_iterator source_dir(src_root);
    fs::create_directories(dst_root);
    for (fs::directory_entry const & dir_entry : source_dir)
    {
        fs::path src_path = dir_entry.path();
        fs::path relative = fs::relative(src_path, src_root);
        fs::path dst_path = dst_root / relative;
        if (is_exist(dst_path, force))
            continue;
        if (dir_entry.is_regular_file())
        {
            std::cout << "\033[01;32m" <<  src_path << " -> " << dst_path << "\033[0m\n";
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

        fs::path src_path(argv[args_it]);
        fs::path dst_path(argv[args_it + 1]);

        Magick::InitializeMagick(argv[0]);

        try
        {
            convert_dir(src_path, dst_path, force);
        }
        catch (fs::filesystem_error & e)
        {
            std::cout << "just file\n";
            if (!is_exist(dst_path, force))
                convert_image(src_path, dst_path);
        }
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

