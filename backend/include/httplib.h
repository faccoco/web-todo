#pragma once

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <vector>

// Minimal httplib header for single-header inclusion
// For production, use the full cpp-httplib library
namespace httplib {
    struct Request {
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::map<std::string, std::string> params;
        
        bool has_param(const std::string& key) const {
            return params.find(key) != params.end();
        }
        
        std::string get_param_value(const std::string& key) const {
            auto it = params.find(key);
            return it != params.end() ? it->second : "";
        }
    };
    
    struct Response {
        int status = 200;
        std::map<std::string, std::string> headers;
        std::string body;
        
        void set_content(const std::string& content, const std::string& content_type) {
            body = content;
            headers["Content-Type"] = content_type;
        }
    };
    
    using Handler = std::function<void(const Request&, Response&)>;
    
    class Server {
    public:
        Server();
        ~Server();
        
        void Get(const std::string& pattern, Handler handler);
        void Post(const std::string& pattern, Handler handler);
        void Put(const std::string& pattern, Handler handler);
        void Delete(const std::string& pattern, Handler handler);
        
        void set_mount_point(const std::string& mount_point, const std::string& dir);
        
        bool listen(const std::string& host, int port);
        void stop();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}