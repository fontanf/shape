#pragma once

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


    static T from_json(
            nlohmann::basic_json<>& json_item)
    {
        T t;
        if (json_item.contains("description"))
            t.description = json_item["description"];
        if (json_item.contains("write_json"))
            t.write_json = json_item["write_json"];
        if (json_item.contains("write_svg"))
            t.write_svg = json_item["write_svg"];
        return t;
    }

    static T read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::TestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return T::from_json(json);
    }

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
                    T test_params = read_json(entry.string());
                    test_params.name = entry.string();
                    params.emplace_back(test_params);
                }
            }
        }
        return params;
    }
};
