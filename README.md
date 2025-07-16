# Web Todo App

A full-stack todo application with C++ backend and React frontend, optimized for Raspberry Pi deployment entirely developed by claude-code.

The development and testing of the entire project were completed by claude-code.

## Features

- **Backend (Pure C++)**
  - REST API with SQLite database
  - CRUD operations for todos
  - Lightweight and efficient
  - Cross-platform compatibility

- **Frontend (React + TypeScript)**
  - Modern, responsive UI
  - Real-time todo management
  - Mobile-friendly design
  - Error handling and loading states

- **Deployment**
  - Docker containerization
  - Multi-stage builds for optimization
  - Docker Compose for easy deployment
  - Nginx reverse proxy
  - Volume persistence for data

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   React App     │    │   Nginx         │    │   C++ Backend   │
│   (Frontend)    │◄──►│   (Reverse      │◄──►│   (REST API)    │
│   Port: 80      │    │    Proxy)       │    │   Port: 8080    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                         │
                                                         ▼
                                              ┌─────────────────┐
                                              │   SQLite DB     │
                                              │   (Persistent)  │
                                              └─────────────────┘
```

## Quick Start

### Prerequisites

- Docker and Docker Compose
- Git

### Deployment

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd web-todo
   ```

2. **Build and run with Docker Compose**
   ```bash
   docker-compose up -d
   ```

3. **Access the application**
   - Frontend: http://localhost
   - Backend API: http://localhost:8080/api/todos

### API Endpoints

- `GET /api/todos` - Get all todos
- `POST /api/todos` - Create new todo
- `PUT /api/todos/:id` - Update todo
- `DELETE /api/todos/:id` - Delete todo

### Example API Usage

```bash
# Get all todos
curl http://localhost:8080/api/todos

# Create a new todo
curl -X POST http://localhost:8080/api/todos \
  -H "Content-Type: application/json" \
  -d '{"text": "Learn Docker"}'

# Update a todo
curl -X PUT http://localhost:8080/api/todos/1 \
  -H "Content-Type: application/json" \
  -d '{"text": "Learn Docker Compose", "completed": true}'

# Delete a todo
curl -X DELETE http://localhost:8080/api/todos/1
```

## Development

### Backend Development

```bash
cd backend
mkdir build && cd build
cmake ..
make
./todo_backend
```

### Frontend Development

```bash
cd frontend
npm install
npm start
```

### Building for Production

```bash
# Build all services
docker-compose build

# Or build individually
docker build -t todo-backend ./backend
docker build -t todo-frontend ./frontend
```

## Deployment

### Option 1: Direct Docker Compose

```bash
# On your Raspberry Pi
git clone <repository-url>
cd web-todo
docker-compose up -d
```

### Option 2: Pre-built Images

```bash
# Build on development machine
docker-compose build
docker save todo-backend todo-frontend > todo-app.tar

# Transfer to Raspberry Pi
scp todo-app.tar pi@raspberrypi:~

# Load on Raspberry Pi
docker load < todo-app.tar
docker-compose up -d
```

### Performance Optimization for Raspberry Pi

The application is optimized for ARM devices:

- **Backend**: Compiled C++ binary with minimal dependencies
- **Frontend**: Static files served by Nginx
- **Database**: SQLite for low resource usage
- **Containers**: Multi-stage builds to reduce image size

## Configuration

### Environment Variables

**Backend**
- `DB_PATH`: SQLite database path (default: `/app/data/todos.db`)

**Frontend**
- `REACT_APP_API_URL`: Backend API URL (default: `http://localhost:8080`)

### Docker Compose Override

Create `docker-compose.override.yml` for custom configuration:

```yaml
version: '3.8'

services:
  frontend:
    ports:
      - "3000:80"  # Custom port
    environment:
      - REACT_APP_API_URL=http://localhost:8080

  backend:
    ports:
      - "8081:8080"  # Custom port
    environment:
      - DB_PATH=/app/data/custom.db
```

## Troubleshooting

### Common Issues

1. **Port conflicts**
   ```bash
   # Check what's using port 80
   sudo netstat -tulpn | grep :80
   
   # Use different ports in docker-compose.yml
   ports:
     - "8080:80"  # Use port 8080 instead
   ```

2. **Database permissions**
   ```bash
   # Fix volume permissions
   docker-compose down
   docker volume rm web-todo_todo_data
   docker-compose up -d
   ```

3. **API connection issues**
   - Check if backend is running: `docker logs todo-backend`
   - Verify network connectivity: `docker network ls`
   - Check firewall settings on Raspberry Pi

### Logs

```bash
# View all logs
docker-compose logs

# View specific service logs
docker-compose logs backend
docker-compose logs frontend
```

## Security

- CORS is configured for frontend-backend communication
- Nginx security headers are enabled
- SQLite database is stored in persistent volume
- No sensitive data in environment variables

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License.