#pragma once

#include "shape/shape.hpp"

#include <boost/filesystem.hpp>

#include <fstream>

namespace fs = boost::filesystem;

template <typename T>
struct TestParams
{
    std::string name;
    std::string description;
    bool write_json = false;
    bool write_svg = false;


    static std::vector<T> read_dir(
            const std::string& dir_path)
    {
        std::vector<T> params;
        if (fs::is_directory(dir_path)) {
            std::vector<fs::path> files_in_directory;
            std::copy(fs::directory_iterator(dir_path), fs::directory_iterator(), std::back_inserter(files_in_directory));
            std::sort(files_in_directory.begin(), files_in_directory.end());   // Sort paths in alphabetical order
            for (const fs::path& entry : files_in_directory) {
                if (entry.extension().string() == ".json") {
                    T test_params = T::read_json(entry.string());
                    test_params.name = entry.string();
                    params.emplace_back(test_params);
                }
            }
        }
        return params;
    }
};
