#include "test_framework.h"
#include "../include/auth_service.h"
#include <filesystem>

// Helper function to clean up test database
void cleanupAuthTestDb() {
    std::vector<std::string> db_files = {
        "test_auth.db", "todos.db", "auth_test.db", "test_database.db"
    };
    for (const auto& file : db_files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
}

TEST(auth_service_initialization) {
    cleanupAuthTestDb();
    
    // Test database management is handled by cleanup function
    
    try {
        AuthService auth;
        // If we get here, initialization succeeded
        ASSERT_TRUE(true);
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_registration_success) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        auto user = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(user.has_value());
        ASSERT_STR_EQ("testuser", user->username);
        ASSERT_STR_EQ("test@example.com", user->email);
        ASSERT_TRUE(user->id > 0);
        ASSERT_TRUE(!user->password_hash.empty());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_registration_duplicate) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register first user
        auto user1 = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(user1.has_value());
        
        // Try to register duplicate username
        auto user2 = auth.registerUser("testuser", "different@example.com", "password123");
        ASSERT_FALSE(user2.has_value());
        
        // Try to register duplicate email
        auto user3 = auth.registerUser("differentuser", "test@example.com", "password123");
        ASSERT_FALSE(user3.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_registration_empty_fields) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Empty username
        auto user1 = auth.registerUser("", "test@example.com", "password123");
        ASSERT_FALSE(user1.has_value());
        
        // Empty email
        auto user2 = auth.registerUser("testuser", "", "password123");
        ASSERT_FALSE(user2.has_value());
        
        // Empty password
        auto user3 = auth.registerUser("testuser", "test@example.com", "");
        ASSERT_FALSE(user3.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_login_success) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register user first
        auto registered = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(registered.has_value());
        
        // Login with correct credentials
        auto user_auth = auth.loginUser("testuser", "password123");
        ASSERT_TRUE(user_auth.has_value());
        ASSERT_STR_EQ("testuser", user_auth->username);
        ASSERT_STR_EQ("test@example.com", user_auth->email);
        ASSERT_EQ(registered->id, user_auth->user_id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_login_wrong_password) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register user first
        auto registered = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(registered.has_value());
        
        // Login with wrong password
        auto user_auth = auth.loginUser("testuser", "wrongpassword");
        ASSERT_FALSE(user_auth.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(user_login_nonexistent_user) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Login with nonexistent user
        auto user_auth = auth.loginUser("nonexistent", "password123");
        ASSERT_FALSE(user_auth.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(token_generation_and_validation) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register and login user
        auto registered = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(registered.has_value());
        
        auto user_auth = auth.loginUser("testuser", "password123");
        ASSERT_TRUE(user_auth.has_value());
        
        // Generate token
        std::string token = auth.generateToken(*user_auth);
        ASSERT_TRUE(!token.empty());
        
        // Validate token
        auto validated = auth.validateToken(token);
        ASSERT_TRUE(validated.has_value());
        ASSERT_STR_EQ(user_auth->username, validated->username);
        ASSERT_STR_EQ(user_auth->email, validated->email);
        ASSERT_EQ(user_auth->user_id, validated->user_id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(token_validation_invalid_token) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Test with invalid token formats
        auto result1 = auth.validateToken("invalid_token");
        ASSERT_FALSE(result1.has_value());
        
        auto result2 = auth.validateToken("1:user:123");
        ASSERT_FALSE(result2.has_value());
        
        auto result3 = auth.validateToken("");
        ASSERT_FALSE(result3.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(token_validation_expired_token) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Create a token that appears expired (old timestamp)
        std::string expired_token = "1:testuser:1000000000:1234567890";
        
        auto result = auth.validateToken(expired_token);
        ASSERT_FALSE(result.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(auth_get_user_by_id) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register user
        auto registered = auth.registerUser("testuser", "test@example.com", "password123");
        ASSERT_TRUE(registered.has_value());
        
        // Get user by ID
        auto user = auth.getUserById(registered->id);
        ASSERT_TRUE(user.has_value());
        ASSERT_STR_EQ("testuser", user->username);
        ASSERT_STR_EQ("test@example.com", user->email);
        ASSERT_EQ(registered->id, user->id);
        
        // Get nonexistent user
        auto nonexistent = auth.getUserById(999);
        ASSERT_FALSE(nonexistent.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}

TEST(password_hashing_different_each_time) {
    cleanupAuthTestDb();
    
    try {
        AuthService auth;
        
        // Register two users with same password
        auto user1 = auth.registerUser("user1", "user1@example.com", "samepassword");
        auto user2 = auth.registerUser("user2", "user2@example.com", "samepassword");
        
        ASSERT_TRUE(user1.has_value());
        ASSERT_TRUE(user2.has_value());
        
        // Password hashes should be different (due to salt)
        ASSERT_TRUE(user1->password_hash != user2->password_hash);
        
        // But both should be able to login
        auto auth1 = auth.loginUser("user1", "samepassword");
        auto auth2 = auth.loginUser("user2", "samepassword");
        
        ASSERT_TRUE(auth1.has_value());
        ASSERT_TRUE(auth2.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupAuthTestDb();
}