#include "auth_service.h"
#include "database.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <random>
#include <algorithm>

// Simple hash function (for demo - in production use bcrypt or similar)
std::string simpleHash(const std::string& input, const std::string& salt) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(input + salt));
}

AuthService::AuthService() : db_(std::make_unique<Database>()) {
    if (!db_->initialize()) {
        throw std::runtime_error("Failed to initialize database");
    }
}

AuthService::~AuthService() = default;

std::string AuthService::hashPassword(const std::string& password) {
    // Simple salt generation (in production use secure random)
    std::string salt = "todo_app_salt_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    std::string hash = simpleHash(password, salt);
    return salt + ":" + hash;
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash) {
    size_t colon_pos = hash.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }
    
    std::string salt = hash.substr(0, colon_pos);
    std::string stored_hash = hash.substr(colon_pos + 1);
    
    return simpleHash(password, salt) == stored_hash;
}

std::string AuthService::generateToken(const UserAuth& user) {
    // Simple token generation (in production use JWT library)
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    std::string token_data = std::to_string(user.user_id) + ":" + 
                           user.username + ":" + 
                           std::to_string(timestamp);
    
    std::hash<std::string> hasher;
    std::string token_hash = std::to_string(hasher(token_data + "secret_key"));
    
    // Base64-like encoding (simplified)
    std::string token = token_data + ":" + token_hash;
    return token;
}

std::optional<UserAuth> AuthService::validateToken(const std::string& token) {
    std::vector<std::string> parts;
    std::stringstream ss(token);
    std::string item;
    
    while (std::getline(ss, item, ':')) {
        parts.push_back(item);
    }
    
    if (parts.size() != 4) {
        return std::nullopt;
    }
    
    try {
        int user_id = std::stoi(parts[0]);
        std::string username = parts[1];
        long timestamp = std::stol(parts[2]);
        std::string provided_hash = parts[3];
        
        // Check if token is expired (24 hours)
        auto now = std::chrono::system_clock::now();
        auto current_timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        if (current_timestamp - timestamp > 86400) { // 24 hours
            return std::nullopt;
        }
        
        // Verify hash
        std::string token_data = parts[0] + ":" + parts[1] + ":" + parts[2];
        std::hash<std::string> hasher;
        std::string expected_hash = std::to_string(hasher(token_data + "secret_key"));
        
        if (provided_hash != expected_hash) {
            return std::nullopt;
        }
        
        // Get user info
        auto user = db_->getUserById(user_id);
        if (!user) {
            return std::nullopt;
        }
        
        return UserAuth{user_id, username, user->email};
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::optional<User> AuthService::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    if (username.empty() || email.empty() || password.empty()) {
        return std::nullopt;
    }
    
    if (db_->userExists(username, email)) {
        return std::nullopt;
    }
    
    std::string password_hash = hashPassword(password);
    return db_->createUser(username, email, password_hash);
}

std::optional<UserAuth> AuthService::loginUser(const std::string& username, const std::string& password) {
    auto user = db_->getUserByUsername(username);
    if (!user) {
        return std::nullopt;
    }
    
    if (!verifyPassword(password, user->password_hash)) {
        return std::nullopt;
    }
    
    return UserAuth{user->id, user->username, user->email};
}

std::optional<User> AuthService::getUserById(int user_id) {
    return db_->getUserById(user_id);
}