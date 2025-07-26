#pragma once

#include <boost/filesystem.hpp>

#include <fstream>

namespace fs = boost::filesystem;

template <typename T>
struct TestParams
{
    std::string name;
    std::string description;


    static T from_json(
            nlohmann::basic_json<>& json_item)
    {
        T t;
        if (json_item.contains("description"))
            t.description = json_item["description"];
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
        if (fs::is_directory(dir_path))
            for (auto& entry : fs::directory_iterator(dir_path)) {
                if (entry.path().extension().string() == ".json") {
                    T test_params = read_json(entry.path().string());
                    test_params.name = entry.path().string();
                    params.emplace_back(test_params);
                }
            }
        return params;
    }
};
