#include "test_framework.h"
#include "../include/database.h"
#include <filesystem>
#include <fstream>

const std::string TEST_DB_PATH = "test_database.db";

// Helper function to clean up test database
void cleanupTestDb() {
    if (std::filesystem::exists(TEST_DB_PATH)) {
        std::filesystem::remove(TEST_DB_PATH);
    }
}

TEST(database_initialization) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Check if database file was created
    ASSERT_TRUE(std::filesystem::exists(TEST_DB_PATH));
    
    cleanupTestDb();
}

TEST(user_creation) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    auto user = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(user.has_value());
    ASSERT_STR_EQ("testuser", user->username);
    ASSERT_STR_EQ("test@example.com", user->email);
    ASSERT_STR_EQ("hashedpassword", user->password_hash);
    ASSERT_TRUE(user->id > 0);
    
    cleanupTestDb();
}

TEST(user_creation_duplicate) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create first user
    auto user1 = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(user1.has_value());
    
    // Try to create duplicate username
    auto user2 = db.createUser("testuser", "different@example.com", "hashedpassword");
    ASSERT_FALSE(user2.has_value());
    
    // Try to create duplicate email
    auto user3 = db.createUser("differentuser", "test@example.com", "hashedpassword");
    ASSERT_FALSE(user3.has_value());
    
    cleanupTestDb();
}

TEST(db_get_user_by_username) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create user
    auto created = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(created.has_value());
    
    // Get user by username
    auto retrieved = db.getUserByUsername("testuser");
    ASSERT_TRUE(retrieved.has_value());
    ASSERT_STR_EQ("testuser", retrieved->username);
    ASSERT_STR_EQ("test@example.com", retrieved->email);
    ASSERT_EQ(created->id, retrieved->id);
    
    // Try to get non-existent user
    auto nonexistent = db.getUserByUsername("nonexistent");
    ASSERT_FALSE(nonexistent.has_value());
    
    cleanupTestDb();
}

TEST(db_get_user_by_id) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create user
    auto created = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(created.has_value());
    
    // Get user by ID
    auto retrieved = db.getUserById(created->id);
    ASSERT_TRUE(retrieved.has_value());
    ASSERT_STR_EQ("testuser", retrieved->username);
    ASSERT_STR_EQ("test@example.com", retrieved->email);
    ASSERT_EQ(created->id, retrieved->id);
    
    // Try to get non-existent user
    auto nonexistent = db.getUserById(999);
    ASSERT_FALSE(nonexistent.has_value());
    
    cleanupTestDb();
}

TEST(db_user_exists) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Initially no users exist
    ASSERT_FALSE(db.userExists("testuser", "test@example.com"));
    
    // Create user
    auto created = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(created.has_value());
    
    // Now user exists
    ASSERT_TRUE(db.userExists("testuser", "different@example.com"));
    ASSERT_TRUE(db.userExists("differentuser", "test@example.com"));
    ASSERT_FALSE(db.userExists("nonexistent", "nonexistent@example.com"));
    
    cleanupTestDb();
}

TEST(todo_creation) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create user first
    auto user = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(user.has_value());
    
    // Create todo
    auto todo = db.createTodo("Test todo", user->id);
    ASSERT_TRUE(todo.id > 0);
    ASSERT_EQ(user->id, todo.user_id);
    ASSERT_STR_EQ("Test todo", todo.text);
    ASSERT_FALSE(todo.completed);
    ASSERT_TRUE(!todo.created_at.empty());
    ASSERT_TRUE(!todo.updated_at.empty());
    
    cleanupTestDb();
}

TEST(get_all_todos_by_user) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create two users
    auto user1 = db.createUser("user1", "user1@example.com", "hashedpassword");
    auto user2 = db.createUser("user2", "user2@example.com", "hashedpassword");
    ASSERT_TRUE(user1.has_value());
    ASSERT_TRUE(user2.has_value());
    
    // Create todos for user1
    auto todo1 = db.createTodo("User1 Todo1", user1->id);
    auto todo2 = db.createTodo("User1 Todo2", user1->id);
    
    // Create todo for user2
    auto todo3 = db.createTodo("User2 Todo1", user2->id);
    
    // Get todos for user1
    auto user1_todos = db.getAllTodos(user1->id);
    ASSERT_EQ(2, user1_todos.size());
    
    // Get todos for user2
    auto user2_todos = db.getAllTodos(user2->id);
    ASSERT_EQ(1, user2_todos.size());
    ASSERT_STR_EQ("User2 Todo1", user2_todos[0].text);
    
    cleanupTestDb();
}

TEST(todo_update) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create user
    auto user = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(user.has_value());
    
    // Create todo
    auto todo = db.createTodo("Original text", user->id);
    ASSERT_TRUE(todo.id > 0);
    
    // Update todo
    auto updated = db.updateTodo(todo.id, "Updated text", true, user->id);
    ASSERT_EQ(todo.id, updated.id);
    ASSERT_STR_EQ("Updated text", updated.text);
    ASSERT_TRUE(updated.completed);
    ASSERT_STR_EQ(todo.created_at, updated.created_at);
    ASSERT_TRUE(updated.updated_at != todo.updated_at);
    
    cleanupTestDb();
}

TEST(todo_delete) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create user
    auto user = db.createUser("testuser", "test@example.com", "hashedpassword");
    ASSERT_TRUE(user.has_value());
    
    // Create todo
    auto todo = db.createTodo("Todo to delete", user->id);
    ASSERT_TRUE(todo.id > 0);
    
    // Delete todo
    ASSERT_TRUE(db.deleteTodo(todo.id, user->id));
    
    // Try to delete again (should fail)
    ASSERT_FALSE(db.deleteTodo(todo.id, user->id));
    
    // Verify todo is gone
    auto todos = db.getAllTodos(user->id);
    ASSERT_EQ(0, todos.size());
    
    cleanupTestDb();
}

TEST(todo_isolation_between_users) {
    cleanupTestDb();
    
    Database db(TEST_DB_PATH);
    ASSERT_TRUE(db.initialize());
    
    // Create two users
    auto user1 = db.createUser("user1", "user1@example.com", "hashedpassword");
    auto user2 = db.createUser("user2", "user2@example.com", "hashedpassword");
    ASSERT_TRUE(user1.has_value());
    ASSERT_TRUE(user2.has_value());
    
    // Create todo for user1
    auto todo = db.createTodo("User1 Todo", user1->id);
    ASSERT_TRUE(todo.id > 0);
    
    // User2 should not be able to access user1's todo
    auto retrieved = db.getTodoById(todo.id, user2->id);
    ASSERT_EQ(-1, retrieved.id);
    
    // User2 should not be able to update user1's todo
    auto updated = db.updateTodo(todo.id, "Hacked", true, user2->id);
    ASSERT_EQ(-1, updated.id);
    
    // User2 should not be able to delete user1's todo
    ASSERT_FALSE(db.deleteTodo(todo.id, user2->id));
    
    // User1 should still have their todo
    auto user1_todos = db.getAllTodos(user1->id);
    ASSERT_EQ(1, user1_todos.size());
    ASSERT_STR_EQ("User1 Todo", user1_todos[0].text);
    
    cleanupTestDb();
}