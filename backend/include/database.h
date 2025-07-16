#pragma once

#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>

struct Todo {
    int id;
    int user_id;
    std::string text;
    bool completed;
    std::string created_at;
    std::string updated_at;
};

struct User {
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string created_at;
    std::string updated_at;
};

class Database {
public:
    Database(const std::string& db_path = "todos.db");
    ~Database();
    
    bool initialize();
    
    // Todo methods
    std::vector<Todo> getAllTodos(int user_id);
    Todo getTodoById(int id, int user_id);
    Todo createTodo(const std::string& text, int user_id);
    Todo updateTodo(int id, const std::string& text, bool completed, int user_id);
    bool deleteTodo(int id, int user_id);
    
    // User methods
    std::optional<User> createUser(const std::string& username, const std::string& email, const std::string& password_hash);
    std::optional<User> getUserByUsername(const std::string& username);
    std::optional<User> getUserById(int id);
    bool userExists(const std::string& username, const std::string& email);
    
private:
    sqlite3* db_;
    std::string db_path_;
    
    std::string getCurrentTimestamp();
};