package cmd

import (
	"fmt"
	"os"
	"strings"
	"vortex/m/v2/pkg"

	"github.com/chzyer/readline"
	"github.com/olekukonko/tablewriter"
	"github.com/spf13/cobra"
)

// clientCmd represents the client command
var clientCmd = &cobra.Command{
	Use:   "client",
	Short: "Creates a REPL to task agents",
	Run: func(cmd *cobra.Command, args []string) {
		db, _ := pkg.NewDb("db.json")

		// Define the main REPL completer
		mainCompleter := readline.NewPrefixCompleter(
			readline.PcItem("list"),
			readline.PcItem("interact", readline.PcItemDynamic(func(line string) []string {
				ids := make([]string, len(db.Agents))
				for i, agent := range db.Agents {
					ids[i] = agent.ID
				}
				return ids
			})),
			readline.PcItem("exit"),
		)

		// Set up readline for the main REPL with autocompletion
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
				table := tablewriter.NewWriter(os.Stdout)
				table.SetHeader([]string{"ID", "Name", "Host", "Alive"})

				for _, v := range db.Agents {
					isActive := pkg.Ternary(v.IsActive, "True", "False")
					table.Append([]string{v.ID, v.Name, v.Host, pkg.AnyToString(isActive)})
				}
				table.Render()
			case "interact":
				if len(args) < 2 {
					fmt.Println("Please specify an agent ID to interact with.")
					continue
				}
				agentID := args[1]

				// Define a new completer for the interact prompt
				interactCompleter := readline.NewPrefixCompleter(
					readline.PcItem("sleep"),
					readline.PcItem("back"),
					readline.PcItem("exit"),
				)

				// Update the readline config with the inner prompt and completer
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

					taskId, _ := pkg.GenerateRandomID(6)
					println(agentID)
					task := pkg.Task{TaskType: agentArgs[0], AgentID: agentID, ID: taskId}
					switch agentArgs[0] {
					case "sleep":
						if len(agentArgs) > 1 {
							task.Args = agentArgs[1]
						}
					case "exit":
						os.Exit(0)
					}
					if agentArgs[0] == "back" {
						rl.SetPrompt("v0rtex> ")
						rl.Config.AutoComplete = mainCompleter
						break // Exit the interact loop, returning to the outer REPL
					} else {
						db.AddTask(task)
					}
				}
			case "exit":
				os.Exit(0)
			default:
				fmt.Println("Unknown command:", args[0])
			}
		}
	},
}

func init() {
	rootCmd.AddCommand(clientCmd)
}
