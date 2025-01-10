#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

// Function to check if a string contains only digits (for PID validation)
bool isNumber(const string& str) {
    return all_of(str.begin(), str.end(), ::isdigit);
}

// Function to list running processes
vector<pair<string, string>> listProcesses() {
    vector<pair<string, string>> processes;

    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        perror("Error opening /proc directory");
        return processes;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (isNumber(entry->d_name)) {
            string pid = entry->d_name;
            string path = "/proc/" + pid + "/comm";
            ifstream file(path);

            if (file.is_open()) {
                string processName;
                getline(file, processName);
                processes.push_back({processName, pid});
                file.close();
            }
        }
    }
    closedir(dir);

    // Sort processes alphabetically by name
    sort(processes.begin(), processes.end());
    return processes;
}

// Function to display all processes
void displayProcesses(const vector<pair<string, string>>& processes) {
    cout << "Running Processes:" << endl;
    cout << "-------------------------------------" << endl;
    for (const auto& process : processes) {
        cout << "Process Name: " << process.first << " | PID: " << process.second << endl;
    }
    cout << "-------------------------------------" << endl;
}

// Function to search for a process by name using binary search
void searchProcessByName(const vector<pair<string, string>>& processes, const string& processName) {
    int left = 0, right = processes.size() - 1;
    bool found = false;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (processes[mid].first == processName) {
            cout << "Process Found: " << processName << " (PID: " << processes[mid].second << ")" << endl;
            found = true;
            break;
        } else if (processes[mid].first < processName) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (!found) {
        cout << "Process '" << processName << "' not found." << endl;
    }
}

// Function to kill a process by PID
void killProcess(const string& pid) {
    if (kill(stoi(pid), SIGKILL) == 0) {
        cout << "Process with PID " << pid << " has been killed." << endl;
    } else {
        perror("Error killing process");
    }
}

// Function to create a new child process using fork() and exec()
void createNewProcess(const char* command) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execlp(command, command, NULL);
        perror("Error executing command");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        wait(NULL);  // Wait for child process to finish
        cout << "Child process completed." << endl;
    } else {
        perror("Fork failed");
    }
}

int main() {
    int choice;

    do {
        cout << "\nLinux Process Management Tool" << endl;
        cout << "1. List Running Processes" << endl;
        cout << "2. Search Process by Name" << endl;
        cout << "3. Kill a Process by PID" << endl;
        cout << "4. Create a New Process" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                auto processes = listProcesses();
                displayProcesses(processes);
                break;
            }
            case 2: {
                auto processes = listProcesses();
                string processName;
                cout << "Enter process name to search: ";
                cin >> processName;
                searchProcessByName(processes, processName);
                break;
            }
            case 3: {
                string pid;
                cout << "Enter PID to kill: ";
                cin >> pid;
                killProcess(pid);
                break;
            }
            case 4: {
                string command;
                cout << "Enter command to execute (e.g., ls, date): ";
                cin >> command;
                createNewProcess(command.c_str());
                break;
            }
            case 5:
                cout << "Exiting..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 5);

    return 0;
}
