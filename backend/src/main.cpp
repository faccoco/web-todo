#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <thread>
#include <atomic>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "todo_service.h"
#include "auth_service.h"

std::string escapeJson(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c; break;
        }
    }
    return output;
}

std::string todoToJson(const Todo& todo) {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\":" << todo.id << ",";
    ss << "\"user_id\":" << todo.user_id << ",";
    ss << "\"text\":\"" << escapeJson(todo.text) << "\",";
    ss << "\"completed\":" << (todo.completed ? "true" : "false") << ",";
    ss << "\"created_at\":\"" << escapeJson(todo.created_at) << "\",";
    ss << "\"updated_at\":\"" << escapeJson(todo.updated_at) << "\",";
    ss << "\"due_date\":";
    if (todo.due_date.empty()) {
        ss << "null";
    } else {
        ss << "\"" << escapeJson(todo.due_date) << "\"";
    }
    ss << "}";
    return ss.str();
}

std::string todosToJson(const std::vector<Todo>& todos) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < todos.size(); ++i) {
        if (i > 0) ss << ",";
        ss << todoToJson(todos[i]);
    }
    ss << "]";
    return ss.str();
}

std::string userToJson(const User& user) {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\":" << user.id << ",";
    ss << "\"username\":\"" << escapeJson(user.username) << "\",";
    ss << "\"email\":\"" << escapeJson(user.email) << "\",";
    ss << "\"created_at\":\"" << escapeJson(user.created_at) << "\",";
    ss << "\"updated_at\":\"" << escapeJson(user.updated_at) << "\"";
    ss << "}";
    return ss.str();
}

std::string authResponseToJson(const UserAuth& user, const std::string& token) {
    std::stringstream ss;
    ss << "{";
    ss << "\"user\":{";
    ss << "\"id\":" << user.user_id << ",";
    ss << "\"username\":\"" << escapeJson(user.username) << "\",";
    ss << "\"email\":\"" << escapeJson(user.email) << "\"";
    ss << "},";
    ss << "\"token\":\"" << escapeJson(token) << "\"";
    ss << "}";
    return ss.str();
}

std::string extractJsonField(const std::string& json, const std::string& field) {
    std::regex pattern("\"" + field + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    return "";
}

bool extractJsonBool(const std::string& json, const std::string& field) {
    std::regex pattern("\"" + field + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str() == "true";
    }
    return false;
}

std::string extractAuthToken(const std::string& headers) {
    std::regex pattern("Authorization:\\s*Bearer\\s+([^\\s]+)");
    std::smatch match;
    if (std::regex_search(headers, match, pattern)) {
        return match[1].str();
    }
    return "";
}

class SimpleHttpServer {
private:
    int server_fd;
    struct sockaddr_in address;
    int port;
    std::atomic<bool> running_{false};
    TodoService todoService_;
    AuthService authService_;
    
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
        running_ = true;
        
        while (running_) {
            int client_socket;
            int addrlen = sizeof(address);
            
            client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_socket < 0) {
                if (running_) {
                    std::cerr << "Accept failed" << std::endl;
                }
                continue;
            }
            
            std::thread([this, client_socket]() {
                handleRequest(client_socket);
            }).detach();
        }
    }
    
    void stop() {
        running_ = false;
        close(server_fd);
    }
    
private:
    void handleRequest(int client_socket) {
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            close(client_socket);
            return;
        }
        
        std::string request(buffer, bytes_read);
        std::string response = processRequest(request);
        
        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }
    
    std::string processRequest(const std::string& request) {
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        // Handle OPTIONS for CORS
        if (method == "OPTIONS") {
            return "HTTP/1.1 200 OK\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                   "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                   "Content-Length: 0\r\n\r\n";
        }
        
        std::string headers = request.substr(0, request.find("\r\n\r\n"));
        std::string body;
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            body = request.substr(body_start + 4);
        }
        
        std::string response_body;
        std::string content_type = "application/json";
        int status_code = 200;
        
        try {
            // Authentication endpoints
            if (method == "POST" && path == "/api/auth/register") {
                response_body = handleRegister(body);
                status_code = 201;
            } else if (method == "POST" && path == "/api/auth/login") {
                response_body = handleLogin(body);
            } else if (method == "GET" && path == "/api/auth/me") {
                std::string token = extractAuthToken(headers);
                response_body = handleGetMe(token);
            }
            // Todo endpoints (require authentication)
            else if (path.find("/api/todos") == 0) {
                std::string token = extractAuthToken(headers);
                auto user_auth = authService_.validateToken(token);
                if (!user_auth) {
                    response_body = "{\"error\":\"Unauthorized\"}";
                    status_code = 401;
                } else {
                    if (method == "GET" && path == "/api/todos") {
                        auto todos = todoService_.getAllTodos(user_auth->user_id);
                        response_body = todosToJson(todos);
                    } else if (method == "POST" && path == "/api/todos") {
                        std::string text = extractJsonField(body, "text");
                        std::string due_date = extractJsonField(body, "dueDate");
                        if (!text.empty()) {
                            auto todo = todoService_.createTodo(text, user_auth->user_id, due_date);
                            response_body = todoToJson(todo);
                            status_code = 201;
                        } else {
                            response_body = "{\"error\":\"Text field is required\"}";
                            status_code = 400;
                        }
                    } else if (method == "PUT" && path.find("/api/todos/") == 0) {
                        int id = std::stoi(path.substr(11));
                        std::string text = extractJsonField(body, "text");
                        bool completed = extractJsonBool(body, "completed");
                        
                        auto todo = todoService_.updateTodo(id, text, completed, user_auth->user_id);
                        if (todo.id != -1) {
                            response_body = todoToJson(todo);
                        } else {
                            response_body = "{\"error\":\"Todo not found\"}";
                            status_code = 404;
                        }
                    } else if (method == "DELETE" && path.find("/api/todos/") == 0) {
                        int id = std::stoi(path.substr(11));
                        bool success = todoService_.deleteTodo(id, user_auth->user_id);
                        if (success) {
                            response_body = "";
                            status_code = 204;
                        } else {
                            response_body = "{\"error\":\"Todo not found\"}";
                            status_code = 404;
                        }
                    }
                }
            } else {
                response_body = "{\"error\":\"Not found\"}";
                status_code = 404;
            }
        } catch (const std::exception& e) {
            response_body = "{\"error\":\"Internal server error\"}";
            status_code = 500;
            std::cerr << "Error processing request: " << e.what() << std::endl;
        }
        
        std::string status_text = (status_code == 200) ? "OK" : 
                                 (status_code == 201) ? "Created" :
                                 (status_code == 204) ? "No Content" :
                                 (status_code == 400) ? "Bad Request" :
                                 (status_code == 401) ? "Unauthorized" :
                                 (status_code == 404) ? "Not Found" : "Internal Server Error";
        
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        response << "Content-Type: " << content_type << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        response << "Content-Length: " << response_body.length() << "\r\n";
        response << "\r\n";
        response << response_body;
        
        return response.str();
    }
    
    std::string handleRegister(const std::string& body) {
        std::string username = extractJsonField(body, "username");
        std::string email = extractJsonField(body, "email");
        std::string password = extractJsonField(body, "password");
        
        if (username.empty() || email.empty() || password.empty()) {
            return "{\"error\":\"Username, email, and password are required\"}";
        }
        
        auto user = authService_.registerUser(username, email, password);
        if (!user) {
            return "{\"error\":\"User already exists or registration failed\"}";
        }
        
        UserAuth user_auth{user->id, user->username, user->email};
        std::string token = authService_.generateToken(user_auth);
        
        return authResponseToJson(user_auth, token);
    }
    
    std::string handleLogin(const std::string& body) {
        std::string username = extractJsonField(body, "username");
        std::string password = extractJsonField(body, "password");
        
        if (username.empty() || password.empty()) {
            return "{\"error\":\"Username and password are required\"}";
        }
        
        auto user_auth = authService_.loginUser(username, password);
        if (!user_auth) {
            return "{\"error\":\"Invalid username or password\"}";
        }
        
        std::string token = authService_.generateToken(*user_auth);
        return authResponseToJson(*user_auth, token);
    }
    
    std::string handleGetMe(const std::string& token) {
        auto user_auth = authService_.validateToken(token);
        if (!user_auth) {
            return "{\"error\":\"Invalid token\"}";
        }
        
        auto user = authService_.getUserById(user_auth->user_id);
        if (!user) {
            return "{\"error\":\"User not found\"}";
        }
        
        return userToJson(*user);
    }
};

SimpleHttpServer* server = nullptr;

void signalHandler(int) {
    std::cout << "\nShutting down server..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        server = new SimpleHttpServer(8080);
        std::cout << "Todo API Server with Authentication starting..." << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "Authentication:" << std::endl;
        std::cout << "  POST   /api/auth/register - Register new user" << std::endl;
        std::cout << "  POST   /api/auth/login    - Login user" << std::endl;
        std::cout << "  GET    /api/auth/me       - Get current user" << std::endl;
        std::cout << "Todos (authenticated):" << std::endl;
        std::cout << "  GET    /api/todos         - Get user's todos" << std::endl;
        std::cout << "  POST   /api/todos         - Create new todo" << std::endl;
        std::cout << "  PUT    /api/todos/:id     - Update todo" << std::endl;
        std::cout << "  DELETE /api/todos/:id     - Delete todo" << std::endl;
        std::cout << std::endl;
        
        server->start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    delete server;
    return 0;
}