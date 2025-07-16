#include "test_framework.h"
#include "../include/auth_service.h"
#include "../include/todo_service.h"
#include <filesystem>
#include <thread>
#include <chrono>

// Helper function to clean up test database
void cleanupIntegrationTestDb() {
    std::vector<std::string> db_files = {
        "test_integration.db", "todos.db", "integration_test.db", "test_database.db"
    };
    for (const auto& file : db_files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
}

TEST(integration_user_registration_and_todo_creation) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        TodoService todoService;
        
        // Register a user
        auto user = auth.registerUser("integrationuser", "integration@example.com", "password123");
        ASSERT_TRUE(user.has_value());
        
        // Login the user
        auto userAuth = auth.loginUser("integrationuser", "password123");
        ASSERT_TRUE(userAuth.has_value());
        
        // Create a todo for the user
        auto todo = todoService.createTodo("Integration test todo", userAuth->user_id);
        ASSERT_TRUE(todo.id > 0);
        ASSERT_EQ(userAuth->user_id, todo.user_id);
        ASSERT_STR_EQ("Integration test todo", todo.text);
        
        // Get user's todos
        auto todos = todoService.getAllTodos(userAuth->user_id);
        ASSERT_EQ(1, todos.size());
        ASSERT_STR_EQ("Integration test todo", todos[0].text);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}

TEST(integration_multiple_users_todo_isolation) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        TodoService todoService;
        
        // Register two users
        auto user1 = auth.registerUser("user1", "user1@example.com", "password123");
        auto user2 = auth.registerUser("user2", "user2@example.com", "password123");
        ASSERT_TRUE(user1.has_value());
        ASSERT_TRUE(user2.has_value());
        
        // Login both users
        auto userAuth1 = auth.loginUser("user1", "password123");
        auto userAuth2 = auth.loginUser("user2", "password123");
        ASSERT_TRUE(userAuth1.has_value());
        ASSERT_TRUE(userAuth2.has_value());
        
        // Create todos for each user
        auto todo1 = todoService.createTodo("User 1 todo", userAuth1->user_id);
        auto todo2 = todoService.createTodo("User 2 todo", userAuth2->user_id);
        
        // Verify isolation - each user only sees their own todos
        auto user1_todos = todoService.getAllTodos(userAuth1->user_id);
        auto user2_todos = todoService.getAllTodos(userAuth2->user_id);
        
        ASSERT_EQ(1, user1_todos.size());
        ASSERT_EQ(1, user2_todos.size());
        ASSERT_STR_EQ("User 1 todo", user1_todos[0].text);
        ASSERT_STR_EQ("User 2 todo", user2_todos[0].text);
        
        // User 1 cannot access User 2's todo
        auto retrieved = todoService.getTodoById(todo2.id, userAuth1->user_id);
        ASSERT_EQ(-1, retrieved.id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}

TEST(integration_token_workflow) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        TodoService todoService;
        
        // Register user
        auto user = auth.registerUser("tokenuser", "token@example.com", "password123");
        ASSERT_TRUE(user.has_value());
        
        // Login and get token
        auto userAuth = auth.loginUser("tokenuser", "password123");
        ASSERT_TRUE(userAuth.has_value());
        
        std::string token = auth.generateToken(*userAuth);
        ASSERT_TRUE(!token.empty());
        
        // Validate token
        auto validatedAuth = auth.validateToken(token);
        ASSERT_TRUE(validatedAuth.has_value());
        ASSERT_EQ(userAuth->user_id, validatedAuth->user_id);
        ASSERT_STR_EQ(userAuth->username, validatedAuth->username);
        
        // Use validated user info to create todo
        auto todo = todoService.createTodo("Token test todo", validatedAuth->user_id);
        ASSERT_TRUE(todo.id > 0);
        ASSERT_EQ(validatedAuth->user_id, todo.user_id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}

TEST(integration_complete_todo_workflow) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        TodoService todoService;
        
        // Register and login user
        auto user = auth.registerUser("workflowuser", "workflow@example.com", "password123");
        ASSERT_TRUE(user.has_value());
        
        auto userAuth = auth.loginUser("workflowuser", "password123");
        ASSERT_TRUE(userAuth.has_value());
        
        // Create multiple todos
        auto todo1 = todoService.createTodo("First todo", userAuth->user_id);
        auto todo2 = todoService.createTodo("Second todo", userAuth->user_id);
        auto todo3 = todoService.createTodo("Third todo", userAuth->user_id);
        
        // Verify all todos exist
        auto todos = todoService.getAllTodos(userAuth->user_id);
        ASSERT_EQ(3, todos.size());
        
        // Update middle todo
        auto updated = todoService.updateTodo(todo2.id, "Updated second todo", true, userAuth->user_id);
        ASSERT_STR_EQ("Updated second todo", updated.text);
        ASSERT_TRUE(updated.completed);
        
        // Delete first todo
        bool deleted = todoService.deleteTodo(todo1.id, userAuth->user_id);
        ASSERT_TRUE(deleted);
        
        // Verify final state
        auto final_todos = todoService.getAllTodos(userAuth->user_id);
        ASSERT_EQ(2, final_todos.size());
        
        // Find the updated todo
        bool found_updated = false;
        for (const auto& todo : final_todos) {
            if (todo.id == todo2.id) {
                ASSERT_STR_EQ("Updated second todo", todo.text);
                ASSERT_TRUE(todo.completed);
                found_updated = true;
            }
        }
        ASSERT_TRUE(found_updated);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}

TEST(integration_password_security) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        
        // Register user
        auto user = auth.registerUser("securityuser", "security@example.com", "mypassword");
        ASSERT_TRUE(user.has_value());
        
        // Password should be hashed, not stored in plain text
        ASSERT_TRUE(user->password_hash != "mypassword");
        ASSERT_TRUE(user->password_hash.length() > 10); // Should be a hash
        
        // Login should work with original password
        auto userAuth = auth.loginUser("securityuser", "mypassword");
        ASSERT_TRUE(userAuth.has_value());
        
        // Login should fail with wrong password
        auto wrongAuth = auth.loginUser("securityuser", "wrongpassword");
        ASSERT_FALSE(wrongAuth.has_value());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}

TEST(integration_user_data_consistency) {
    cleanupIntegrationTestDb();
    
    try {
        AuthService auth;
        
        // Register user
        auto registered = auth.registerUser("consistencyuser", "consistency@example.com", "password123");
        ASSERT_TRUE(registered.has_value());
        
        // Login user
        auto loginAuth = auth.loginUser("consistencyuser", "password123");
        ASSERT_TRUE(loginAuth.has_value());
        
        // Get user by ID
        auto userById = auth.getUserById(registered->id);
        ASSERT_TRUE(userById.has_value());
        
        // All user data should be consistent
        ASSERT_EQ(registered->id, loginAuth->user_id);
        ASSERT_EQ(registered->id, userById->id);
        ASSERT_STR_EQ(registered->username, loginAuth->username);
        ASSERT_STR_EQ(registered->username, userById->username);
        ASSERT_STR_EQ(registered->email, loginAuth->email);
        ASSERT_STR_EQ(registered->email, userById->email);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupIntegrationTestDb();
}