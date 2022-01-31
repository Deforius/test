#include <drogon/drogon.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
//#include <fstream>

using namespace drogon;
namespace fs = std::filesystem;


std::string fileName;
std::vector<std::string> fileList;
std::string fileListSum = "";

std::string fileNameConvert(const fs::path& p) { return (p.filename().string()); }

int main()
{
    app().registerHandler(
        "/",
        [](const HttpRequestPtr&,
            std::function<void(const HttpResponsePtr&)>&& callback) {
                auto resp = HttpResponse::newHttpViewResponse("FileUpload");
                callback(resp);
        });

    app().registerHandler(
        "/upload",
        [](const HttpRequestPtr& req,
            std::function<void(const HttpResponsePtr&)>&& callback) {
                MultiPartParser fileUpload;
                if (fileUpload.parse(req) != 0 || fileUpload.getFiles().size() != 1)
                {
                    auto resp = HttpResponse::newHttpResponse();
                    resp->setBody("Must only be one file");
                    resp->setStatusCode(k403Forbidden);
                    callback(resp);
                    return;
                }

                auto& file = fileUpload.getFiles()[0];
                auto md5 = file.getMd5();
                auto resp = HttpResponse::newHttpResponse();
                resp->setBody(
                    "The server has calculated the file's MD5 hash to be " + md5);
                file.save();
                LOG_INFO << "The uploaded file has been saved to the ./uploads "
                    "directory\n" << file.getFileName() << " MD5 hash: " << md5;
                callback(resp);
        },
        { Post });

    app().registerHandler(
        "/list",
        [](const HttpRequestPtr&,
            std::function<void(const HttpResponsePtr&)>&& callback) {
                auto resp = HttpResponse::newHttpResponse();
                MultiPartParser fileUpload;
                std::string Path = app().getUploadPath();
                for (auto& p : fs::directory_iterator(Path))
                {
                    if (p.is_regular_file())
                    {             
                        fileName = fileNameConvert(p.path());
                        LOG_INFO << fileName;
                        fileName += "\n</br>";
                       // fileList.push_back(fileName);
                        fileListSum += fileName;
                    }
                }
/*
                for (int i = 0; i < fileList.size(); i++)
                {
                     += fileList[i];
                }
                
*/
                resp->setBody(fileListSum);
                callback(resp);
                fileListSum = "";

/*
                for (int i = 0; i < fileList.size(); i++)
                {
                    fileListSum.pop_back();
                }
*/                
        },
        { Get });

    LOG_INFO << "Server running on 127.0.0.1:8848";
    app()
        .setClientMaxBodySize(20 * 2000 * 2000)
        .setUploadPath("./uploads")
        .addListener("127.0.0.1", 8848)
        .run();
}