export interface Todo {
  id: number;
  user_id: number;
  text: string;
  completed: boolean;
  created_at: string;
  updated_at: string;
  due_date?: string;
}

export interface User {
  id: number;
  username: string;
  email: string;
  created_at?: string;
  updated_at?: string;
}

export interface AuthResponse {
  user: User;
  token: string;
}

export interface LoginData {
  username: string;
  password: string;
}

export interface RegisterData {
  username: string;
  email: string;
  password: string;
}