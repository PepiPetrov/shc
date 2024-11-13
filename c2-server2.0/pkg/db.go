package pkg

import (
	"encoding/json"
	"io"
	"os"
	"time"
)

// Task defines the structure for each task.
type Task struct {
	TaskType  string    `json:"task_type"`
	FilePath  string    `json:"file_path,omitempty"`
	Args      string    `json:"args,omitempty"`
	Result    []byte    `json:"result,omitempty"`
	ID        string    `json:"id"`
	AgentID   string    `json:"agent_id"`
	FileID    string    `json:"file_id,omitempty"`
	Finished  bool      `json:"finished"`
	CreatedAt time.Time `json:"created_at"`
}

// Agent defines the structure for each agent.
type Agent struct {
	ID       string `json:"id"`
	Name     string `json:"name"`
	Host     string `json:"host"`
	IsActive bool   `json:"is_active"`
}

// DB represents the database structure which contains tasks, agents, and a file.
type DB struct {
	File   *os.File
	Tasks  []Task  // Publicly accessible tasks table
	Agents []Agent // Publicly accessible agents table
}

type AgentNotFoundError struct {
	Str string
}

func (e *AgentNotFoundError) Error() string {
	return "agent not found " + e.Str
}

type TaskNotFoundError struct {
	Str string
}

func (e *TaskNotFoundError) Error() string {
	return "agent not found " + e.Str
}

// newDb initializes a new DB. If the file exists, it loads the tasks and agents from the file.
// Otherwise, it creates a new file and initializes both tables (tasks and agents) as empty arrays.
func NewDb(path string) (*DB, error) {
	var tasks []Task
	var agents []Agent

	// Check if the file exists
	if _, err := os.Stat(path); err == nil {
		// File exists, open it
		file, err := os.OpenFile(path, os.O_RDWR, 0644)
		if err != nil {
			return nil, err
		}

		// Read the file contents
		fileData, err := io.ReadAll(file)
		if err != nil {
			file.Close()
			return nil, err
		}

		// Attempt to deserialize the file contents into tasks and agents
		if len(fileData) > 0 {
			data := struct {
				Tasks  []Task  `json:"tasks"`
				Agents []Agent `json:"agents"`
			}{}
			err = json.Unmarshal(fileData, &data)
			if err != nil {
				file.Close()
				return nil, err
			}
			tasks = data.Tasks
			agents = data.Agents
		}

		// Return the DB with tasks and agents loaded from file
		return &DB{File: file, Tasks: tasks, Agents: agents}, nil
	}

	// If the file doesn't exist, create a new file
	file, err := os.Create(path)
	if err != nil {
		return nil, err
	}

	// Initialize with empty tasks and agents arrays
	db := &DB{
		File:   file,
		Tasks:  []Task{},
		Agents: []Agent{},
	}

	// Initialize the file with empty tasks and agents tables
	err = db.WriteToFile()
	if err != nil {
		return nil, err
	}

	return db, nil
}

// WriteToFile serializes the tasks and agents and writes them to the file.
func (db *DB) WriteToFile() error {
	data := struct {
		Tasks  []Task  `json:"tasks"`
		Agents []Agent `json:"agents"`
	}{
		Tasks:  db.Tasks,
		Agents: db.Agents,
	}

	// Serialize the data
	fileData, err := json.Marshal(data)
	if err != nil {
		return err
	}

	// Write the data to the file (truncate the file first)
	db.File.Truncate(0)
	db.File.Seek(0, 0)
	_, err = db.File.Write(fileData)
	return err
}

func (db *DB) AddTask(task Task) error {
	task.CreatedAt = time.Now()
	db.Tasks = append(db.Tasks, task)
	return db.WriteToFile()
}

func (db *DB) AddAgent(agent Agent) error {
	db.Agents = append(db.Agents, agent)
	return db.WriteToFile()
}

func (db *DB) GetAgentById(id string) (*Agent, error) {
	for _, agent := range db.Agents {
		if agent.ID == id {
			return &agent, nil
		}
	}
	return nil, &AgentNotFoundError{Str: id}
}

func (db *DB) GetAgentByHost(host string) (*Agent, error) {
	for _, agent := range db.Agents {
		if agent.Host == host {
			return &agent, nil
		}
	}
	return nil, &AgentNotFoundError{Str: host}
}

func (db *DB) GetOldestTaskByAgentID(id string) (*Task, error) {
	var oldestTask *Task
	for _, task := range db.Tasks {
		if task.AgentID == id {
			// If this is the first matching task or it's older than the current oldest task
			if oldestTask == nil || task.CreatedAt.Before(oldestTask.CreatedAt) {
				oldestTask = &task
			}
		}
	}

	if oldestTask == nil {
		return nil, &TaskNotFoundError{Str: id} // No tasks found with matching AgentID
	}
	return oldestTask, nil
}
