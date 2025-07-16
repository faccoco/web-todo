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
    <li className={`todo-item ${todo.completed ? 'completed' : ''}`}>
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