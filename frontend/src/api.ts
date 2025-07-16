import { Todo } from './types';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:8080';

const getAuthHeaders = () => {
  const token = localStorage.getItem('auth_token');
  console.log('Auth token:', token ? `${token.substring(0, 20)}...` : 'null');
  return {
    'Content-Type': 'application/json',
    ...(token && { 'Authorization': `Bearer ${token}` }),
  };
};

export class TodoAPI {
  static async getAllTodos(): Promise<Todo[]> {
    const response = await fetch(`${API_BASE_URL}/api/todos`, {
      headers: getAuthHeaders(),
    });
    
    if (!response.ok) {
      if (response.status === 401) {
        throw new Error('Unauthorized - please login again');
      }
      throw new Error('Failed to fetch todos');
    }
    return response.json();
  }

  static async createTodo(text: string): Promise<Todo> {
    const response = await fetch(`${API_BASE_URL}/api/todos`, {
      method: 'POST',
      headers: getAuthHeaders(),
      body: JSON.stringify({ text }),
    });
    
    if (!response.ok) {
      if (response.status === 401) {
        throw new Error('Unauthorized - please login again');
      }
      throw new Error('Failed to create todo');
    }
    return response.json();
  }

  static async updateTodo(id: number, text: string, completed: boolean): Promise<Todo> {
    const response = await fetch(`${API_BASE_URL}/api/todos/${id}`, {
      method: 'PUT',
      headers: getAuthHeaders(),
      body: JSON.stringify({ text, completed }),
    });
    
    if (!response.ok) {
      if (response.status === 401) {
        throw new Error('Unauthorized - please login again');
      }
      throw new Error('Failed to update todo');
    }
    return response.json();
  }

  static async deleteTodo(id: number): Promise<void> {
    const response = await fetch(`${API_BASE_URL}/api/todos/${id}`, {
      method: 'DELETE',
      headers: getAuthHeaders(),
    });
    
    if (!response.ok) {
      if (response.status === 401) {
        throw new Error('Unauthorized - please login again');
      }
      throw new Error('Failed to delete todo');
    }
  }
}