// package internal

// import (
// 	"encoding/json"
// 	"io"
// 	"os"
// 	"sync"
// 	"time"
// )

// // Task defines the structure for each task.
// type Task struct {
// 	TaskType  string    `json:"task_type"`
// 	FilePath  string    `json:"file_path,omitempty"`
// 	Args      string    `json:"args,omitempty"`
// 	Result    []byte    `json:"result,omitempty"`
// 	ID        string    `json:"id"`
// 	AgentID   string    `json:"agent_id"`
// 	FileID    string    `json:"file_id,omitempty"`
// 	Finished  bool      `json:"finished"`
// 	CreatedAt time.Time `json:"created_at"`
// }

// // Agent defines the structure for each agent.
// type Agent struct {
// 	ID       string `json:"id"`
// 	Name     string `json:"name"`
// 	Host     string `json:"host"`
// 	IsActive bool   `json:"is_active"`
// }

// // DB represents the database structure containing tasks, agents, and a file.
// type DB struct {
// 	File   *os.File
// 	Tasks  []Task     // Publicly accessible tasks table
// 	Agents []Agent    // Publicly accessible agents table
// 	mu     sync.Mutex // Mutex for thread-safe access
// }

// type AgentNotFoundError struct {
// 	Str string
// }

// func (e *AgentNotFoundError) Error() string {
// 	return "agent not found " + e.Str
// }

// type TaskNotFoundError struct {
// 	Str string
// }

// func (e *TaskNotFoundError) Error() string {
// 	return "task not found " + e.Str
// }

// // NewDb initializes a new DB. If the file exists, it loads tasks and agents from it.
// // Otherwise, it creates a new file and initializes empty arrays for tasks and agents.
// func NewDb(path string) (*DB, error) {
// 	var tasks []Task
// 	var agents []Agent

// 	// Check if the file exists
// 	if _, err := os.Stat(path); err == nil {
// 		// File exists, open it
// 		file, err := os.OpenFile(path, os.O_RDWR, 0644)
// 		if err != nil {
// 			return nil, err
// 		}

// 		// Read the file contents
// 		fileData, err := io.ReadAll(file)
// 		if err != nil {
// 			file.Close()
// 			return nil, err
// 		}

// 		// Attempt to deserialize the file contents into tasks and agents
// 		if len(fileData) > 0 {
// 			data := struct {
// 				Tasks  []Task  `json:"tasks"`
// 				Agents []Agent `json:"agents"`
// 			}{}
// 			err = json.Unmarshal(fileData, &data)
// 			if err != nil {
// 				file.Close()
// 				return nil, err
// 			}
// 			tasks := []Task{}
// 			for _, task := range data.Tasks {
// 				if !task.Finished {
// 					tasks = append(tasks, task)
// 				}
// 			}
// 			println(len(tasks))
// 			agents = data.Agents
// 		}

// 		// Return the DB with tasks and agents loaded from file
// 		return &DB{File: file, Tasks: tasks, Agents: agents}, nil
// 	}

// 	// If the file doesn't exist, create a new file
// 	file, err := os.Create(path)
// 	if err != nil {
// 		return nil, err
// 	}

// 	// Initialize with empty tasks and agents arrays
// 	db := &DB{
// 		File:   file,
// 		Tasks:  []Task{},
// 		Agents: []Agent{},
// 	}

// 	// Initialize the file with empty tasks and agents tables
// 	err = db.WriteToFile()
// 	if err != nil {
// 		return nil, err
// 	}

// 	return db, nil
// }

// // WriteToFile serializes the tasks and agents and writes them to the file.
// func (db *DB) WriteToFile() error {
// 	// db.mu.Lock()
// 	// defer db.mu.Unlock()

// 	data := struct {
// 		Tasks  []Task  `json:"tasks"`
// 		Agents []Agent `json:"agents"`
// 	}{
// 		Tasks:  db.Tasks,
// 		Agents: db.Agents,
// 	}

// 	// Serialize the data
// 	fileData, err := json.Marshal(data)
// 	if err != nil {
// 		return err
// 	}

// 	// Write the data to the file (truncate the file first)
// 	db.File.Truncate(0)
// 	db.File.Seek(0, 0)
// 	_, err = db.File.Write(fileData)
// 	db.File.Sync()
// 	return err
// }

// // Refresh reloads tasks and agents from the file, updating the DB struct.
// func (db *DB) Refresh() error {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	// Read the file contents
// 	fileData, err := io.ReadAll(db.File)
// 	if err != nil {
// 		return err
// 	}

// 	// Deserialize the file contents into tasks and agents
// 	var data struct {
// 		Tasks  []Task  `json:"tasks"`
// 		Agents []Agent `json:"agents"`
// 	}
// 	if err := json.Unmarshal(fileData, &data); err != nil {
// 		return err
// 	}

// 	// Update the DB's tasks and agents with the new data
// 	db.Tasks = data.Tasks
// 	db.Agents = data.Agents
// 	return nil
// }

// func (db *DB) AddTask(task Task) error {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	task.CreatedAt = time.Now()
// 	db.Tasks = append(db.Tasks, task)
// 	return db.WriteToFile()
// }

// func (db *DB) AddAgent(agent Agent) error {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	db.Agents = append(db.Agents, agent)
// 	return db.WriteToFile()
// }

// func (db *DB) GetAgentById(id string) (*Agent, error) {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	for _, agent := range db.Agents {
// 		if agent.ID == id {
// 			return &agent, nil
// 		}
// 	}
// 	return nil, &AgentNotFoundError{Str: id}
// }

// func (db *DB) GetAgentByHost(host string) (*Agent, error) {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	for _, agent := range db.Agents {
// 		if agent.Host == host {
// 			return &agent, nil
// 		}
// 	}
// 	return nil, &AgentNotFoundError{Str: host}
// }

// func (db *DB) GetOldestTaskByAgentID(id string) (*Task, error) {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	var oldestTask *Task
// 	for _, task := range db.Tasks {
// 		if task.AgentID == id {
// 			// If this is the first matching task or it's older than the current oldest task
// 			if (oldestTask == nil || task.CreatedAt.Before(oldestTask.CreatedAt)) && !task.Finished {
// 				oldestTask = &task
// 			}
// 		}
// 	}

// 	if oldestTask == nil {
// 		return nil, &TaskNotFoundError{Str: id} // No tasks found with matching AgentID
// 	}
// 	return oldestTask, nil
// }

// func (db *DB) GetTaskByID(id string) (*Task, error) {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	for _, task := range db.Tasks {
// 		if task.ID == id {
// 			return &task, nil
// 		}
// 	}

// 	return nil, &TaskNotFoundError{Str: id} // No tasks found with matching AgentID
// }

// func (db *DB) FinishTask(id string, result string) error {
// 	db.mu.Lock()
// 	defer db.mu.Unlock()

// 	for idx, task := range db.Tasks {
// 		if task.ID == id {
// 			db.Tasks[idx].Finished = true
// 			db.Tasks[idx].Result = []byte(result)
// 			return db.WriteToFile()
// 		}
// 	}

// 	return &TaskNotFoundError{Str: id} // No tasks found with matching AgentID
//}

package internal

import (
	"time"

	"github.com/asdine/storm/v3"
	"github.com/asdine/storm/v3/q"
)

// Task defines the structure for each task.
type Task struct {
	ID        string    `storm:"id"`    // Primary key
	TaskType  string    `storm:"index"` // Indexed field
	FilePath  string    `storm:"index"` // Optional indexed field
	Args      string    `json:"args,omitempty"`
	Result    []byte    `json:"result,omitempty"`
	AgentID   string    `storm:"index"` // Indexed for query
	FileID    string    `json:"file_id,omitempty"`
	Finished  bool      `storm:"index"` // Indexed for faster queries
	CreatedAt time.Time `storm:"index"` // Indexed for sorting
}

// Agent defines the structure for each agent.
type Agent struct {
	ID       string `storm:"id"`    // Primary key
	Name     string `storm:"index"` // Indexed field
	Host     string `storm:"index"` // Indexed field
	IsActive bool   `storm:"index"` // Indexed for queries
}

// DB represents the database wrapper using storm.
type DB struct {
	db *storm.DB
}

// NewDb initializes a new DB instance with tables for tasks and agents.
func NewDb(path string) (*DB, error) {
	// Open or create the database file
	db, err := storm.Open(path)
	if err != nil {
		return nil, err
	}

	// Initialize the tables (buckets) by ensuring the types exist
	if err := db.Init(&Task{}); err != nil {
		return nil, err
	}
	if err := db.Init(&Agent{}); err != nil {
		return nil, err
	}

	return &DB{db: db}, nil
}

// Close closes the database connection.
func (db *DB) Close() error {
	return db.db.Close()
}

// AddTask adds a new task to the Task table.
func (db *DB) AddTask(task *Task) error {
	task.CreatedAt = time.Now()
	return db.db.Save(task) // Automatically saved to the Task "table"
}

// AddAgent adds a new agent to the Agent table.
func (db *DB) AddAgent(agent *Agent) error {
	return db.db.Save(agent) // Automatically saved to the Agent "table"
}

// GetAllAgents retrieves all agents from the database.
func (db *DB) GetAllAgents() ([]Agent, error) {
	var agents []Agent
	err := db.db.All(&agents) // Fetch all records in the Agent "table"
	if err != nil {
		return nil, err
	}
	return agents, nil
}

// GetAgentByID retrieves an agent by its ID from the Agent table.
func (db *DB) GetAgentByID(id string) (*Agent, error) {
	var agent Agent
	err := db.db.One("ID", id, &agent)
	return &agent, err
}

// GetAgentByHost retrieves an agent by its host from the Agent table.
func (db *DB) GetAgentByHost(host string) (*Agent, error) {
	var agent Agent
	err := db.db.One("Host", host, &agent)
	return &agent, err
}

// GetOldestTaskByAgentID retrieves the oldest unfinished task for a specific agent ID from the Task table.
func (db *DB) GetOldestTaskByAgentID(agentID string) (*Task, error) {
	var tasks []Task
	err := db.db.Select(
		q.Eq("AgentID", agentID),
		q.Eq("Finished", false),
	).OrderBy("CreatedAt").Limit(1).Find(&tasks)

	if err != nil || len(tasks) == 0 {
		return nil, storm.ErrNotFound
	}

	return &tasks[0], nil
}

// GetTaskByID retrieves a task by its ID from the Task table.
func (db *DB) GetTaskByID(id string) (*Task, error) {
	var task Task
	err := db.db.One("ID", id, &task)
	return &task, err
}

// FinishTask marks a task as finished and stores the result in the Task table.
func (db *DB) FinishTask(id string, result string) error {
	var task Task
	err := db.db.One("ID", id, &task)
	if err != nil {
		return err
	}

	task.Finished = true
	task.Result = []byte(result)

	return db.db.Save(&task)
}
