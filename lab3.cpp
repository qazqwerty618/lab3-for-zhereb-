#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Task {
    std::string name;
    std::string description;
    std::string deadline;
};

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    
    void addTask(const std::string &name, const std::string &description, const std::string &deadline);
    void editTask(int index, const std::string &name, const std::string &description, const std::string &deadline);
    void removeTask(int index);
    std::vector<Task>& getTasks();
    
    void saveToFile(const std::string &filename);
    void loadFromFile(const std::string &filename);
    
private:
    std::vector<Task> tasks;
};

#endif // TASKMANAGER_H
#include "taskmanager.h"
#include <fstream>

TaskManager::TaskManager() {}

TaskManager::~TaskManager() {}

void TaskManager::addTask(const std::string &name, const std::string &description, const std::string &deadline) {
    Task task {name, description, deadline};
    tasks.push_back(task);
}

void TaskManager::editTask(int index, const std::string &name, const std::string &description, const std::string &deadline) {
    if (index >= 0 && index < tasks.size()) {
        tasks[index].name = name;
        tasks[index].description = description;
        tasks[index].deadline = deadline;
    }
}

void TaskManager::removeTask(int index) {
    if (index >= 0 && index < tasks.size()) {
        tasks.erase(tasks.begin() + index);
    }
}

std::vector<Task>& TaskManager::getTasks() {
    return tasks;
}

void TaskManager::saveToFile(const std::string &filename) {
    nlohmann::json jsonData;
    for (const auto& task : tasks) {
        jsonData.push_back({{"name", task.name}, {"description", task.description}, {"deadline", task.deadline}});
    }
    
    std::ofstream file(filename);
    if (file.is_open()) {
        file << jsonData.dump(4); // Красиве форматування JSON
        file.close();
    }
}

void TaskManager::loadFromFile(const std::string &filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        
        tasks.clear();
        for (const auto& item : jsonData) {
            Task task;
            task.name = item.at("name").get<std::string>();
            task.description = item.at("description").get<std::string>();
            task.deadline = item.at("deadline").get<std::string>();
            tasks.push_back(task);
        }
    }
}
#include "taskmanager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

TaskManager taskManager;
std::string taskName = "";
std::string taskDescription = "";
std::string taskDeadline = "";

void renderUI() {
    ImGui::Begin("Task Manager");

    // Форма для додавання нового завдання
    ImGui::InputText("Task Name", &taskName[0], 100);
    ImGui::InputText("Task Description", &taskDescription[0], 200);
    ImGui::InputText("Task Deadline", &taskDeadline[0], 50);

    if (ImGui::Button("Add Task")) {
        taskManager.addTask(taskName, taskDescription, taskDeadline);
        taskName = "";
        taskDescription = "";
        taskDeadline = "";
    }

    ImGui::Separator();

    // Відображення списку завдань
    auto& tasks = taskManager.getTasks();
    for (int i = 0; i < tasks.size(); ++i) {
        ImGui::Text("Task %d: %s", i + 1, tasks[i].name.c_str());
        ImGui::Text("Description: %s", tasks[i].description.c_str());
        ImGui::Text("Deadline: %s", tasks[i].deadline.c_str());
        if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
            taskManager.removeTask(i);
        }
        ImGui::Separator();
    }

    ImGui::End();
}

int main(int, char**) {
    // Ініціалізація GLFW
    if (!glfwInit())
        return -1;

    // Створюємо вікно
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Task Manager", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Ввімкнення V-Sync

    // Ініціалізація ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Завантажуємо попередні завдання
    taskManager.loadFromFile("tasks.json");

    // Головний цикл
    while (!glfwWindowShouldClose(window)) {
        // Обробка подій
        glfwPollEvents();

        // Початок нового кадру ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Малюємо інтерфейс
        renderUI();

        // Відображаємо вікно
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Зберігаємо завдання перед виходом
    taskManager.saveToFile("tasks.json");

    // Очищення ресурсів
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
