#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

class SimpleHttpServer {
private:
    int server_fd;
    struct sockaddr_in address;
    int port;
    
public:
    SimpleHttpServer(int p) : port(p) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Bind failed");
        }
        
        if (listen(server_fd, 3) < 0) {
            throw std::runtime_error("Listen failed");
        }
    }
    
    ~SimpleHttpServer() {
        close(server_fd);
    }
    
    void start() {
        std::cout << "Server listening on port " << port << std::endl;
        
        while (true) {
            int client_socket;
            int addrlen = sizeof(address);
            
            client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_socket < 0) {
                continue;
            }
            
            std::thread([this, client_socket]() {
                handleRequest(client_socket);
            }).detach();
        }
    }
    
private:
    void handleRequest(int client_socket) {
        char buffer[4096] = {0};
        read(client_socket, buffer, 4096);
        
        std::string request(buffer);
        std::string response = processRequest(request);
        
        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }
    
    std::string processRequest(const std::string& request) {
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        std::string body;
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            body = request.substr(body_start + 4);
        }
        
        std::string response_body;
        std::string content_type = "application/json";
        int status_code = 200;
        
        if (method == "GET" && path == "/api/todos") {
            response_body = handleGetTodos();
        } else if (method == "POST" && path == "/api/todos") {
            response_body = handleCreateTodo(body);
            status_code = 201;
        } else if (method == "PUT" && path.find("/api/todos/") == 0) {
            int id = std::stoi(path.substr(11));
            response_body = handleUpdateTodo(id, body);
        } else if (method == "DELETE" && path.find("/api/todos/") == 0) {
            int id = std::stoi(path.substr(11));
            response_body = handleDeleteTodo(id);
            status_code = 204;
        } else {
            response_body = "{\"error\":\"Not found\"}";
            status_code = 404;
        }
        
        std::string status_text = (status_code == 200) ? "OK" : 
                                 (status_code == 201) ? "Created" :
                                 (status_code == 204) ? "No Content" : "Not Found";
        
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        response << "Content-Type: " << content_type << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "Content-Length: " << response_body.length() << "\r\n";
        response << "\r\n";
        response << response_body;
        
        return response.str();
    }
    
    std::string handleGetTodos() {
        return "[{\"id\":1,\"text\":\"Sample todo\",\"completed\":false,\"created_at\":\"2024-01-01 12:00:00\",\"updated_at\":\"2024-01-01 12:00:00\"}]";
    }
    
    std::string handleCreateTodo(const std::string& body) {
        return "{\"id\":2,\"text\":\"New todo\",\"completed\":false,\"created_at\":\"2024-01-01 12:00:00\",\"updated_at\":\"2024-01-01 12:00:00\"}";
    }
    
    std::string handleUpdateTodo(int id, const std::string& body) {
        return "{\"id\":" + std::to_string(id) + ",\"text\":\"Updated todo\",\"completed\":true,\"created_at\":\"2024-01-01 12:00:00\",\"updated_at\":\"2024-01-01 12:00:00\"}";
    }
    
    std::string handleDeleteTodo(int id) {
        return "";
    }
};