import React from 'react';
import { useAuth } from '../contexts/AuthContext';

const Header: React.FC = () => {
  const { user, logout } = useAuth();

  return (
    <header className="app-header">
      <h1>Todo App</h1>
      {user && (
        <div className="user-info">
          <span>Welcome, {user.username}!</span>
          <button onClick={logout} className="logout-button">
            Logout
          </button>
        </div>
      )}
    </header>
  );
};

export default Header;