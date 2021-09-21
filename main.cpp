#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Validation layers
// Vulkan can configure the validation layers in which it needs to work.
// Validate errors.
// Define a vector that contains the layers to be validated.
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

// Leaving the possibility to remove validation layers.
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace biniutils {
    void logstdout(const char* msg) {
        std::cout << msg << std::endl;
    }
}

// how to define a class in C++
// can be done in a single file 
// can be separated in header + definition

class FirstVulkanExample {

    // in a c++ class we classify first by access modifier
    public:

        void run(){
            // fun stuff here later!
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
    // We can declare attributes here.
    // The first thing is the window object.
    GLFWwindow* window;

    // Need to create a Vulkan instance.
    VkInstance instance;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", nullptr, nullptr);
    }

    void initVulkan() {
        // First we need to check validation layers
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            biniutils::logstdout("Oh no! No support :(");
            throw std::runtime_error("Validation layers requested are not available with this system!");
        }
        // Process of vulkan setup
        createVulkanInstance();
    }

    void createVulkanInstance() {
        // Vulkan common paradigm
        // Struct with values to create an instance of something.
        VkApplicationInfo info {};
        // General app values.
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pApplicationName = "Vulkan 101";
        info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        info.pEngineName = "None";
        info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        info.apiVersion = VK_VERSION_1_0;

        // Variables needed to get extensions.
        // We want that the instance of the Vulkan app can interact with GLFW.
        // Extensions - Funcionalidad modularizada
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &info;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        // Add to instance validation layers.
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Optional use the object result to verify results.
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    }

    bool checkValidationLayerSupport() {
        // Returns if it's posssible to validate the layers defined in the vector of validation layers.
        uint32_t layerCount;

        // Get how many validation layers are available in this system.
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        // Create a vector that contains the strings that define what layers
        std::vector<VkLayerProperties> availableLayers(layerCount);

        // Invoke the method again but now the vector will be filled with the corresponding information.
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Check desired layers vs available layers.
        for (const char* layerName : validationLayers) {
            bool found = false;
            for (const auto& availableLayer : availableLayers) {
                if (strcmp(availableLayer.layerName, layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    void mainLoop() {
        // Create GLFW loop.
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    // In some cases / implementations, a destructor is used instead of this method.
    // Destructor is a method that is called when an object is terminated (we do not have one!)
    // Destructor is normally used to free internal pointers, is good practice.
    void cleanup() {
        // very important - we DON'T have a garbage collector so we need to clean up
        // Clean GFLW
        biniutils::logstdout("Cleaning up application.");
        glfwDestroyWindow(window);
        glfwTerminate();
        // Clean Vulkan
        vkDestroyInstance(instance, nullptr);
    }

};

int main() {
    FirstVulkanExample app;

    try {
        biniutils::logstdout("Initializing application.");
        app.run();
    } catch (const std::exception& e) {
        // Salida para errores
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}