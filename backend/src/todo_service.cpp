#include "todo_service.h"
#include "database.h"

TodoService::TodoService() : db_(std::make_unique<Database>()) {
    if (!db_->initialize()) {
        throw std::runtime_error("Failed to initialize database");
    }
}

TodoService::~TodoService() = default;

std::vector<Todo> TodoService::getAllTodos(int user_id) {
    return db_->getAllTodos(user_id);
}

Todo TodoService::getTodoById(int id, int user_id) {
    return db_->getTodoById(id, user_id);
}

Todo TodoService::createTodo(const std::string& text, int user_id) {
    return db_->createTodo(text, user_id);
}

Todo TodoService::updateTodo(int id, const std::string& text, bool completed, int user_id) {
    return db_->updateTodo(id, text, completed, user_id);
}

bool TodoService::deleteTodo(int id, int user_id) {
    return db_->deleteTodo(id, user_id);
}