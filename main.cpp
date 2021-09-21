#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <optional>
// 1.4 - We are going to use an optional value

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

// 1.6 - We are going to create an struct that contains
struct QueueFamilyIndexes {
    std::optional<uint32_t> graphicsFamily;

    // 1.7 Convienience method to verify that they have value
    bool isComplete() {
        return graphicsFamily.has_value();
    }
};
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

    // 1.2 - We need an instance that saves the reference of the physical device
    // used by Vulkan.
    // Important: This reference is cleanup when destroying Vulkan instance.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // 1.8 - Add logical device - It can be n by physical device.
    VkDevice device;

    // When logical device is created a graphics queue is created.
    VkQueue graphicsQueue;

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
        // 1 - Once the instance is created we need to select a physical device to interact with
        pickPhysicalDevice();
        biniutils::logstdout("Physical device being used.");

        // 9 - Once physical device is validated create logical devices.
        createLogicalDevice();

        // 11 - Create surface where we are going to be drawing.
        // We are going to use a Vulkan Extension - VK_KHR_surface para interactuar con una ventana.
        // VkSurfaceKHR surface;
    }

    void createLogicalDevice() {
        // Get the families.
        QueueFamilyIndexes indexes = findQueueFamilies(physicalDevice);

        // We start to create structs with information to create the logical device.
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indexes.graphicsFamily.value();
        queueCreateInfo.queueCount = 1; // For now.
        
        // In Vulkan you need to assign a priority to the queues.
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        // Struct that defines the requirements on the physical device. Nothing special at the moment.
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;

        // We add layers to validate in logical device.
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Now we create the device.
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get graphics queue reference to use on the future.
        vkGetDeviceQueue(device, 0, indexes.graphicsFamily.value(), &graphicsQueue);
    }

    void pickPhysicalDevice() {
        // Search Video Card that can run the vulkan instance.
        // 1.3 - Enumerate available devices
        uint32_t deviceCount = 0;

        // Get the available devices.
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        // If there are no devices, no way to run the app.
        if (deviceCount == 0) {
            throw std::runtime_error("No physical devices capable of run vulkan!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // Populate stuff.
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Loop devices and grab the first one that is lit.
        for (const auto& device: devices) {
            // Check if the device works depending on what we need.
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("No physical devices have the capabilities to run our program.");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        // Once the struct is defined we can modify it.
        QueueFamilyIndexes indices = findQueueFamilies(device);
        return indices.isComplete();
    }

    // 1.5 - Queues
    // All the actions that we give the GPU are put in a queue.
    // Las colas pertenecen a familias que tiene caracteristicas / capacidades particulares
    // es necesario que verifiquemos la capacidad de nuestro GPU vs nuestra expectativas.
    QueueFamilyIndexes findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndexes indexes;
        uint32_t queueFamilyCount = 0;
        
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indexes.graphicsFamily = i;
            }

            if (indexes.isComplete()) {
                break;
            }
            i++;
        }
        return indexes;
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
        info.apiVersion = VK_API_VERSION_1_0;

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
        VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
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

        // 1.10 - Destroy the logical device.
        vkDestroyDevice(device, nullptr);

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