#pragma once

#include <string>
#include <memory>
#include <optional>
#include "database.h"

struct UserAuth {
    int user_id;
    std::string username;
    std::string email;
};

class AuthService {
public:
    AuthService();
    ~AuthService();
    
    std::optional<User> registerUser(const std::string& username, const std::string& email, const std::string& password);
    std::optional<UserAuth> loginUser(const std::string& username, const std::string& password);
    std::optional<UserAuth> validateToken(const std::string& token);
    std::string generateToken(const UserAuth& user);
    std::optional<User> getUserById(int user_id);
    
private:
    std::unique_ptr<Database> db_;
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
};