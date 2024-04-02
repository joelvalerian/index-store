#include <string>
#include <unordered_set>
#include <vector>

#include "crow.h"
#include "crow/middlewares/cors.h"

#include "utils.hpp"
#include "index.hpp"

#define PROJECT_NAME "index-store"
#define PROJECT_VERSION "0.1.0"
#define PROJECT_AUTHOR "Joel Valerian"
#define PROJECT_EMAIL "joelvalerian@gmail.com"

int main()
{
    utils::load_env();
    const std::string DATA_FOLDER { utils::get_env("IS_DATA_FOLDER", "data") };
    const std::string INDEX_MAP_FILE { utils::get_env("IS_INDEX_MAP_FILE", "index.map") };
    const std::string INDEX_DEQUE_FILE { utils::get_env("IS_INDEX_DEQUE_FILE", "index.deque") };
    const std::string CHARSETS { utils::get_env("IS_CHARSETS", "ABCDE") };
    const std::unordered_set<std::string> ALLOWED_FILES { utils::get_allowed_files("IS_ALLOWED_FILES", "image/png,image/jpeg,text/csv,text/plain,application/pdf,application/json") };
    const int PORT { std::stoi(utils::get_env("IS_PORT", "5000")) };

    CROW_LOG_INFO << "Initializing IndexStore";
    CROW_LOG_INFO << "Config: ";
    CROW_LOG_INFO << "Data Folder: " << DATA_FOLDER;
    CROW_LOG_INFO << "Index Map File: " << INDEX_MAP_FILE;
    CROW_LOG_INFO << "Index Deque File: " << INDEX_DEQUE_FILE;
    CROW_LOG_INFO << "Index Max Length: " << CHARSETS.size();

    IndexStore indexStore {
        DATA_FOLDER,
        INDEX_MAP_FILE,
        INDEX_DEQUE_FILE,
        CHARSETS
    };

    CROW_LOG_INFO << "Initializing Crow API";

    crow::App<crow::CORSHandler> app;

    CROW_ROUTE(app, "/")([&indexStore]()
    {
        indexStore.print_deque(10);
        indexStore.print_map();
        crow::json::wvalue res {};
        res["files"] = indexStore.saved_files();
        return res;
    });

    CROW_ROUTE(app, "/uploadfiles").methods(crow::HTTPMethod::Post)([&indexStore, &ALLOWED_FILES] (const crow::request& req)
    {
        std::vector<std::string> savedFiles {};
        crow::multipart::message message { req };

        for (auto &&part : message.part_map)
        {
            if (part.first != "files[]")
            {
                continue;
            }

            CROW_LOG_DEBUG << "Part Key: " << part.first;

            std::string filetype {};
            std::string filename {};

            for (auto &&[h_key, h_value] : part.second.headers)
            {
                CROW_LOG_DEBUG << "Part Value Headers Key: " << h_key;
                CROW_LOG_DEBUG << "Part Value Headers Value Value: " << h_value.value;
                
                for (auto &&[p_key, p_value] : h_value.params)
                {
                    CROW_LOG_DEBUG << "Part Value Headers Value Params Key: " << p_key;
                    CROW_LOG_DEBUG << "Part Value Headers Value Params Value: " << p_value;
                }

                if (h_key == "Content-Type")
                {
                    if (ALLOWED_FILES.count(h_value.value) != 0)
                    {
                        filetype = h_value.value;
                    }
                }

                if (h_key == "Content-Disposition")
                {
                    if (auto search = h_value.params.find("filename"); search != h_value.params.end())
                    {
                        filename = search->second;
                    }
                }
            }

            if (!filetype.empty() && !filename.empty())
            {
                CROW_LOG_INFO << "Adding file " << filename;
                if (indexStore.add_file(filename, part.second.body))
                {
                    savedFiles.emplace_back(filename);
                }
            }
        }

        crow::json::wvalue response {};
        response["saved_files"] = savedFiles;

        return response;
    });

    CROW_ROUTE(app, "/downloadfile").methods(crow::HTTPMethod::Post)([&indexStore] (const crow::request& req)
    {
        auto body { crow::json::load(req.body) };
        if (!body.has("filename"))
        {
            return crow::response(500, "Invalid JSON request (must have parameter 'filename')");
        }
        const std::string filename { body["filename"].s() };
        const std::string filecontent { indexStore.get_file(filename) };

        if (filecontent.empty())
        {
            return crow::response(500, "File does not exist");
        }
        else
        {
            CROW_LOG_INFO << "Sending file " << filename;
            crow::response response {200, filecontent};
            response.add_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
            response.add_header("Content-Type", "application/octet-stream");
            return response;
        }
    });

    CROW_CATCHALL_ROUTE(app)([](crow::response& res)
    {
        res.body = "Unknown route";
        res.end();
    });

    app.port(PORT).run();
}