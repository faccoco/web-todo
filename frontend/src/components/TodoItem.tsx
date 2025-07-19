import React, { useState } from 'react';
import { Todo } from '../types';

interface TodoItemProps {
  todo: Todo;
  onUpdate: (id: number, text: string, completed: boolean) => void;
  onDelete: (id: number) => void;
}

const TodoItem: React.FC<TodoItemProps> = ({ todo, onUpdate, onDelete }) => {
  const [isEditing, setIsEditing] = useState(false);
  const [editText, setEditText] = useState(todo.text);

  const getDueDateStatus = () => {
    if (!todo.due_date) return '';
    const today = new Date();
    const dueDate = new Date(todo.due_date);
    const timeDiff = dueDate.getTime() - today.getTime();
    const daysDiff = Math.ceil(timeDiff / (1000 * 3600 * 24));
    
    if (daysDiff < 0) return 'overdue';
    if (daysDiff === 0) return 'due-today';
    if (daysDiff <= 3) return 'due-soon';
    return '';
  };

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (editText.trim()) {
      onUpdate(todo.id, editText.trim(), todo.completed);
      setIsEditing(false);
    }
  };

  const handleCancel = () => {
    setEditText(todo.text);
    setIsEditing(false);
  };

  const handleToggleComplete = () => {
    onUpdate(todo.id, todo.text, !todo.completed);
  };

  return (
    <li className={`todo-item ${todo.completed ? 'completed' : ''} ${getDueDateStatus()}`}>
      <div className="todo-content">
        <input
          type="checkbox"
          checked={todo.completed}
          onChange={handleToggleComplete}
          className="todo-checkbox"
        />
        
        {isEditing ? (
          <form onSubmit={handleSubmit} className="edit-form">
            <input
              type="text"
              value={editText}
              onChange={(e) => setEditText(e.target.value)}
              className="edit-input"
              autoFocus
            />
            <div className="edit-actions">
              <button type="submit" className="save-btn">Save</button>
              <button type="button" onClick={handleCancel} className="cancel-btn">Cancel</button>
            </div>
          </form>
        ) : (
          <div className="todo-text-container">
            <span className="todo-text">{todo.text}</span>
            {todo.due_date && (
              <span className="todo-due-date">
                Due: {new Date(todo.due_date).toLocaleDateString()}
              </span>
            )}
            <div className="todo-actions">
              <button onClick={() => setIsEditing(true)} className="edit-btn">Edit</button>
              <button onClick={() => onDelete(todo.id)} className="delete-btn">Delete</button>
            </div>
          </div>
        )}
      </div>
      
      <div className="todo-meta">
        <small>Created: {new Date(todo.created_at).toLocaleDateString()}</small>
        {todo.updated_at !== todo.created_at && (
          <small>Updated: {new Date(todo.updated_at).toLocaleDateString()}</small>
        )}
      </div>
    </li>
  );
};

export default TodoItem;