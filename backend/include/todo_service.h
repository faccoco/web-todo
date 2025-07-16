#pragma once

#include <string>
#include <vector>
#include <memory>
#include "database.h"

class TodoService {
public:
    TodoService();
    ~TodoService();
    
    std::vector<Todo> getAllTodos(int user_id);
    Todo getTodoById(int id, int user_id);
    Todo createTodo(const std::string& text, int user_id);
    Todo updateTodo(int id, const std::string& text, bool completed, int user_id);
    bool deleteTodo(int id, int user_id);
    
private:
    std::unique_ptr<Database> db_;
};