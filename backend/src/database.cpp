#include "database.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

Database::Database(const std::string& db_path) : db_(nullptr), db_path_(db_path) {}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Database::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // Create users table
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );
    )";
    
    char* err_msg = nullptr;
    rc = sqlite3_exec(db_, create_users_table, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating users table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    // Create todos table with user_id foreign key
    const char* create_todos_table = R"(
        CREATE TABLE IF NOT EXISTS todos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            text TEXT NOT NULL,
            completed INTEGER DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            due_date TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";
    
    rc = sqlite3_exec(db_, create_todos_table, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating todos table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

std::string Database::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Todo methods
std::vector<Todo> Database::getAllTodos(int user_id) {
    std::vector<Todo> todos;
    const char* sql = "SELECT id, user_id, text, completed, created_at, updated_at, due_date FROM todos WHERE user_id = ? ORDER BY created_at DESC";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return todos;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Todo todo;
        todo.id = sqlite3_column_int(stmt, 0);
        todo.user_id = sqlite3_column_int(stmt, 1);
        todo.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        todo.completed = sqlite3_column_int(stmt, 3) != 0;
        todo.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        todo.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* due_date_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        todo.due_date = due_date_text ? due_date_text : "";
        todos.push_back(todo);
    }
    
    sqlite3_finalize(stmt);
    return todos;
}

Todo Database::getTodoById(int id, int user_id) {
    Todo todo = {-1, -1, "", false, "", "", ""};
    const char* sql = "SELECT id, user_id, text, completed, created_at, updated_at, due_date FROM todos WHERE id = ? AND user_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return todo;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        todo.id = sqlite3_column_int(stmt, 0);
        todo.user_id = sqlite3_column_int(stmt, 1);
        todo.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        todo.completed = sqlite3_column_int(stmt, 3) != 0;
        todo.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        todo.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* due_date_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        todo.due_date = due_date_text ? due_date_text : "";
    }
    
    sqlite3_finalize(stmt);
    return todo;
}

Todo Database::createTodo(const std::string& text, int user_id, const std::string& due_date) {
    std::string timestamp = getCurrentTimestamp();
    const char* sql = "INSERT INTO todos (user_id, text, completed, created_at, updated_at, due_date) VALUES (?, ?, 0, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {-1, -1, "", false, "", "", ""};
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(), -1, SQLITE_STATIC);
    if (due_date.empty()) {
        sqlite3_bind_null(stmt, 5);
    } else {
        sqlite3_bind_text(stmt, 5, due_date.c_str(), -1, SQLITE_STATIC);
    }
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert todo: " << sqlite3_errmsg(db_) << std::endl;
        return {-1, -1, "", false, "", "", ""};
    }
    
    int id = sqlite3_last_insert_rowid(db_);
    return {id, user_id, text, false, timestamp, timestamp, due_date};
}

Todo Database::updateTodo(int id, const std::string& text, bool completed, int user_id) {
    std::string timestamp = getCurrentTimestamp();
    const char* sql = "UPDATE todos SET text = ?, completed = ?, updated_at = ? WHERE id = ? AND user_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {-1, -1, "", false, "", "", ""};
    }
    
    sqlite3_bind_text(stmt, 1, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, completed ? 1 : 0);
    sqlite3_bind_text(stmt, 3, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);
    sqlite3_bind_int(stmt, 5, user_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to update todo: " << sqlite3_errmsg(db_) << std::endl;
        return {-1, -1, "", false, "", "", ""};
    }
    
    return getTodoById(id, user_id);
}

bool Database::deleteTodo(int id, int user_id) {
    const char* sql = "DELETE FROM todos WHERE id = ? AND user_id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

// User methods
std::optional<User> Database::createUser(const std::string& username, const std::string& email, const std::string& password_hash) {
    std::string timestamp = getCurrentTimestamp();
    const char* sql = "INSERT INTO users (username, email, password_hash, created_at, updated_at) VALUES (?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, timestamp.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert user: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }
    
    int id = sqlite3_last_insert_rowid(db_);
    return User{id, username, email, password_hash, timestamp, timestamp};
}

std::optional<User> Database::getUserByUsername(const std::string& username) {
    const char* sql = "SELECT id, username, email, password_hash, created_at, updated_at FROM users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> Database::getUserById(int id) {
    const char* sql = "SELECT id, username, email, password_hash, created_at, updated_at FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool Database::userExists(const std::string& username, const std::string& email) {
    const char* sql = "SELECT 1 FROM users WHERE username = ? OR email = ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return exists;
}