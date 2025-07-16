#include "test_framework.h"
#include "../include/todo_service.h"
#include <filesystem>

// Helper function to clean up test database
void cleanupTodoTestDb() {
    std::vector<std::string> db_files = {
        "test_todo.db", "todos.db", "todo_test.db", "test_database.db"
    };
    for (const auto& file : db_files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
}

TEST(todo_service_initialization) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        // If we get here, initialization succeeded
        ASSERT_TRUE(true);
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(create_todo_success) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto todo = service.createTodo("Test todo item", user_id);
        
        ASSERT_TRUE(todo.id > 0);
        ASSERT_EQ(user_id, todo.user_id);
        ASSERT_STR_EQ("Test todo item", todo.text);
        ASSERT_FALSE(todo.completed);
        ASSERT_TRUE(!todo.created_at.empty());
        ASSERT_TRUE(!todo.updated_at.empty());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(create_multiple_todos) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto todo1 = service.createTodo("First todo", user_id);
        auto todo2 = service.createTodo("Second todo", user_id);
        
        ASSERT_TRUE(todo1.id > 0);
        ASSERT_TRUE(todo2.id > 0);
        ASSERT_TRUE(todo1.id != todo2.id);
        ASSERT_STR_EQ("First todo", todo1.text);
        ASSERT_STR_EQ("Second todo", todo2.text);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(get_all_todos_empty) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto todos = service.getAllTodos(user_id);
        
        ASSERT_EQ(0, todos.size());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(get_all_todos_with_items) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto todo1 = service.createTodo("First todo", user_id);
        auto todo2 = service.createTodo("Second todo", user_id);
        
        auto todos = service.getAllTodos(user_id);
        
        ASSERT_EQ(2, todos.size());
        
        // Should be ordered by creation date DESC (newest first)
        ASSERT_STR_EQ("Second todo", todos[0].text);
        ASSERT_STR_EQ("First todo", todos[1].text);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(get_todo_by_id_success) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto created = service.createTodo("Test todo", user_id);
        
        auto retrieved = service.getTodoById(created.id, user_id);
        
        ASSERT_EQ(created.id, retrieved.id);
        ASSERT_EQ(created.user_id, retrieved.user_id);
        ASSERT_STR_EQ(created.text, retrieved.text);
        ASSERT_EQ(created.completed, retrieved.completed);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(get_todo_by_id_not_found) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto retrieved = service.getTodoById(999, user_id);
        
        ASSERT_EQ(-1, retrieved.id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(update_todo_success) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto created = service.createTodo("Original text", user_id);
        
        auto updated = service.updateTodo(created.id, "Updated text", true, user_id);
        
        ASSERT_EQ(created.id, updated.id);
        ASSERT_STR_EQ("Updated text", updated.text);
        ASSERT_TRUE(updated.completed);
        ASSERT_STR_EQ(created.created_at, updated.created_at);
        ASSERT_TRUE(updated.updated_at != created.updated_at);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(update_todo_not_found) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto updated = service.updateTodo(999, "Updated text", true, user_id);
        
        ASSERT_EQ(-1, updated.id);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(delete_todo_success) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto created = service.createTodo("Todo to delete", user_id);
        
        bool deleted = service.deleteTodo(created.id, user_id);
        ASSERT_TRUE(deleted);
        
        // Verify it's gone
        auto todos = service.getAllTodos(user_id);
        ASSERT_EQ(0, todos.size());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(delete_todo_not_found) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        bool deleted = service.deleteTodo(999, user_id);
        ASSERT_FALSE(deleted);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(user_isolation_in_todos) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user1_id = 1;
        int user2_id = 2;
        
        auto user1_todo = service.createTodo("User 1 todo", user1_id);
        auto user2_todo = service.createTodo("User 2 todo", user2_id);
        
        // Each user should only see their own todos
        auto user1_todos = service.getAllTodos(user1_id);
        auto user2_todos = service.getAllTodos(user2_id);
        
        ASSERT_EQ(1, user1_todos.size());
        ASSERT_EQ(1, user2_todos.size());
        ASSERT_STR_EQ("User 1 todo", user1_todos[0].text);
        ASSERT_STR_EQ("User 2 todo", user2_todos[0].text);
        
        // User 1 should not be able to access User 2's todo
        auto retrieved = service.getTodoById(user2_todo.id, user1_id);
        ASSERT_EQ(-1, retrieved.id);
        
        // User 1 should not be able to update User 2's todo
        auto updated = service.updateTodo(user2_todo.id, "Hacked", true, user1_id);
        ASSERT_EQ(-1, updated.id);
        
        // User 1 should not be able to delete User 2's todo
        bool deleted = service.deleteTodo(user2_todo.id, user1_id);
        ASSERT_FALSE(deleted);
        
        // User 2's todo should still exist
        auto user2_todos_after = service.getAllTodos(user2_id);
        ASSERT_EQ(1, user2_todos_after.size());
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}

TEST(todo_completion_toggle) {
    cleanupTodoTestDb();
    
    try {
        TodoService service;
        
        int user_id = 1;
        auto created = service.createTodo("Todo to toggle", user_id);
        ASSERT_FALSE(created.completed);
        
        // Mark as completed
        auto completed = service.updateTodo(created.id, created.text, true, user_id);
        ASSERT_TRUE(completed.completed);
        
        // Mark as not completed
        auto uncompleted = service.updateTodo(completed.id, completed.text, false, user_id);
        ASSERT_FALSE(uncompleted.completed);
        
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // Should not throw
    }
    
    cleanupTodoTestDb();
}