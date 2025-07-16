import React, { useState, useEffect } from 'react';
import { Todo } from '../types';
import { TodoAPI } from '../api';
import TodoItem from './TodoItem';

const TodoList: React.FC = () => {
  const [todos, setTodos] = useState<Todo[]>([]);
  const [newTodoText, setNewTodoText] = useState('');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    loadTodos();
  }, []);

  const loadTodos = async () => {
    try {
      setLoading(true);
      const fetchedTodos = await TodoAPI.getAllTodos();
      setTodos(fetchedTodos);
      setError(null);
    } catch (err) {
      if (err instanceof Error && err.message.includes('Unauthorized')) {
        setError('Please login to view your todos');
      } else {
        setError('Failed to load todos');
      }
      console.error('Error loading todos:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleCreateTodo = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!newTodoText.trim()) return;

    try {
      const newTodo = await TodoAPI.createTodo(newTodoText.trim());
      setTodos([newTodo, ...todos]);
      setNewTodoText('');
      setError(null);
    } catch (err) {
      setError('Failed to create todo');
      console.error('Error creating todo:', err);
    }
  };

  const handleUpdateTodo = async (id: number, text: string, completed: boolean) => {
    try {
      const updatedTodo = await TodoAPI.updateTodo(id, text, completed);
      setTodos(todos.map(todo => todo.id === id ? updatedTodo : todo));
      setError(null);
    } catch (err) {
      setError('Failed to update todo');
      console.error('Error updating todo:', err);
    }
  };

  const handleDeleteTodo = async (id: number) => {
    try {
      await TodoAPI.deleteTodo(id);
      setTodos(todos.filter(todo => todo.id !== id));
      setError(null);
    } catch (err) {
      setError('Failed to delete todo');
      console.error('Error deleting todo:', err);
    }
  };

  if (loading) {
    return <div className="loading">Loading todos...</div>;
  }

  const completedCount = todos.filter(todo => todo.completed).length;
  const totalCount = todos.length;

  return (
    <div className="todo-list-container">
      {error && <div className="error-message">{error}</div>}
      
      <form onSubmit={handleCreateTodo} className="add-todo-form">
        <input
          type="text"
          value={newTodoText}
          onChange={(e) => setNewTodoText(e.target.value)}
          placeholder="Add a new todo..."
          className="add-todo-input"
        />
        <button type="submit" className="add-todo-btn">Add Todo</button>
      </form>

      <div className="todo-stats">
        <span>{completedCount} of {totalCount} completed</span>
      </div>

      <ul className="todo-list">
        {todos.map(todo => (
          <TodoItem
            key={todo.id}
            todo={todo}
            onUpdate={handleUpdateTodo}
            onDelete={handleDeleteTodo}
          />
        ))}
        {todos.length === 0 && (
          <li className="empty-state">No todos yet. Add one above!</li>
        )}
      </ul>
    </div>
  );
};

export default TodoList;