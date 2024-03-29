#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <optional>
#include <cstdint>
#include <algorithm>

// 1.4 - We are going to use an optional value
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Validation layers
// Vulkan can configure the validation layers in which it needs to work.
// Validate errors.
// Define a vector that contains the layers to be validated.
const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

// 23 - Add an extension layer.
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// Leaving the possibility to remove validation layers.
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace biniutils
{
    void logstdout(const char *msg)
    {
        std::cout << msg << std::endl;
    }
}

// 1.6 - We are going to create an struct that contains
struct QueueFamilyIndexes
{
    std::optional<uint32_t> graphicsFamily;

    // 18 - Add a second index for the presentation queue family.
    std::optional<uint32_t> presentFamily;

    // 1.7 Convienience method to verify that they have value
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// 25 - We need to do more than just check for chain swap support.
// We need to setup the chain swap and save several references that will
// be used later.
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// how to define a class in C++
// can be done in a single file
// can be separated in header + definition
class FirstVulkanExample
{
    // in a c++ class we classify first by access modifier
public:
    void run()
    {
        // fun stuff here later!
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // We can declare attributes here.
    // The first thing is the window object.
    GLFWwindow *window;

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

    // 13 - Create the surface
    // Surface - Space where the render will be presented.
    VkSurfaceKHR surface;

    // 17 - Add a reference to work with the presentation queue.
    VkQueue presentQueue;

    // 33 - Create an instance to save our newly created swap chain.
    VkSwapchainKHR swapChain;

    // 35 - Declare the images that will be used by the swap chain.
    std::vector<VkImage> swapChainImages;

    // 37 - Save the reference to the format and extent that we got as result
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", nullptr, nullptr);
    }

    void initVulkan()
    {
        // First we need to check validation layers
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            biniutils::logstdout("Oh no! No support :(");
            throw std::runtime_error("Validation layers requested are not available with this system!");
        }
        // Process of vulkan setup
        createVulkanInstance();

        // 14 - Create the surface
        createSurface();

        // 1 - Once the instance is created we need to select a physical device to interact with
        pickPhysicalDevice();
        biniutils::logstdout("Physical device being used.");

        // 31 - Method to create the swap chain
        createSwapChain();

        // 9 - Once physical device is validated create logical devices.
        createLogicalDevice();

        // 11 - Create surface where we are going to be drawing.
        // We are going to use a Vulkan Extension - VK_KHR_surface para interactuar con una ventana.
        // VkSurfaceKHR surface;
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        // Retrieve the 3 values we just made methods for.
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // Establish the minimum amount of images within the swap chain.
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        // Setup max images.
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.presentMode = presentMode;

        // Get queue families and determine ownership of images in the swap chain.
        QueueFamilyIndexes indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        // 2 possibilities, they are the same family, or not.
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;     // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        // Apply transformation already done to the world.
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        // Is there a need to blend?
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // 36 - Save references to images that are going to be used.
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        // 38 - After declare we save the attributes
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Problem creating the surface");
        }
    }

    void createLogicalDevice()
    {
        // Get the families.
        QueueFamilyIndexes indexes = findQueueFamilies(physicalDevice);

        // 20 - Changes made in order to consider several queues.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indexes.graphicsFamily.value(), indexes.presentFamily.value()};

        // Not messing around with these yet.
        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            // Push the newly created VkDeviceQueueCreateInfo struct into the vector
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // This was changed afterwards to add logic to consider the presentation queue.
        /*
        // We start to create structs with information to create the logical device.
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indexes.graphicsFamily.value();
        queueCreateInfo.queueCount = 1; // For now.
        
        // In Vulkan you need to assign a priority to the queues.
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        */

        // Struct that defines the requirements on the physical device. Nothing special at the moment.
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // More changes considering the newly created queue vector.
        // createInfo.pQueueCreateInfos = &queueCreateInfos;
        // createInfo.queueCreateInfoCount = queueCreateInfos.size();

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        // 24 - Modify create info to consider extension support in the logical device.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // We add layers to validate in logical device.
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        // Now we create the device.
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get graphics queue reference to use on the future.
        vkGetDeviceQueue(device, 0, indexes.graphicsFamily.value(), &graphicsQueue);

        // 22 - Same as we did with the graphics queue, we retrieve the reference for the presentation queue
        vkGetDeviceQueue(device, 0, indexes.presentFamily.value(), &presentQueue);
    }

    // 26 - Implement method to return populated chain swap detail struct.
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        // 3 values to receive
        // First - Surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // Check for formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount > 0)
        {
            // First - Resize vector
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // Finally check for presentations modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount > 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    // 28 - Pickup the best format.
    // We need to pick 3 values.
    // Surface format - Color.
    // Presentation Mode - How to swap the images in the swap chain.
    // Swap extent - Resolution that will be used to display the render.

    // First let's get the surface format available.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        // Iterate and search for the best.
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        // Just return the first if no other is available.
        return availableFormats[0];
    }

    // 29 - Presentation Mode.
    // VK_PRESENT_MODE_IMMEDIATE_KHR
    // As soon as the image is there, put it. Causes Screen Tearing.
    // VK_PRESENT_MODE_FIFO_KHR
    // Queue, gets the first one, if it's full wait. Near V-Sync of modern games.
    // VK_PRESENT_MODE_RELAXED_KHR
    // If the queue was empty, it sends it right awayt. Causes Screen Tearing.
    // VK_PRESENT_MODE_MAILBOX_KHR
    // Queue, gets the first one, substitutes frames instead of wait.
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // 30 - Swap Extent
    // Extent - Size of the render in the screen.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void pickPhysicalDevice()
    {
        // Search Video Card that can run the vulkan instance.
        // 1.3 - Enumerate available devices
        uint32_t deviceCount = 0;

        // Get the available devices.
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        // If there are no devices, no way to run the app.
        if (deviceCount == 0)
        {
            throw std::runtime_error("No physical devices capable of run vulkan!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // Populate stuff.
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Loop devices and grab the first one that is lit.
        for (const auto &device : devices)
        {
            // Check if the device works depending on what we need.
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No physical devices have the capabilities to run our program.");
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // Once the struct is defined we can modify it.
        QueueFamilyIndexes indices = findQueueFamilies(device);

        // 21 - Check for support for all of our extensions.
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // 27 - Check support for swapchains
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // iterate through the extensions found on the physical device.
        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    // 1.5 - Queues
    // All the actions that we give the GPU are put in a queue.
    // Las colas pertenecen a familias que tiene caracteristicas / capacidades particulares
    // es necesario que verifiquemos la capacidad de nuestro GPU vs nuestra expectativas.
    QueueFamilyIndexes findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndexes indexes;
        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indexes.graphicsFamily = i;
            }

            // 19 - Check for the presentation queue family
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indexes.presentFamily = i;
            }

            if (indexes.isComplete())
            {
                break;
            }
            i++;
        }
        return indexes;
    }

    void createVulkanInstance()
    {
        // Vulkan common paradigm
        // Struct with values to create an instance of something.
        VkApplicationInfo info{};
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
        const char **glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &info;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        // Add to instance validation layers.
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        // Optional use the object result to verify results.
        VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    }

    bool checkValidationLayerSupport()
    {
        // Returns if it's posssible to validate the layers defined in the vector of validation layers.
        uint32_t layerCount;

        // Get how many validation layers are available in this system.
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        // Create a vector that contains the strings that define what layers
        std::vector<VkLayerProperties> availableLayers(layerCount);

        // Invoke the method again but now the vector will be filled with the corresponding information.
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Check desired layers vs available layers.
        for (const char *layerName : validationLayers)
        {
            bool found = false;
            for (const auto &availableLayer : availableLayers)
            {
                if (strcmp(availableLayer.layerName, layerName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return false;
            }
        }
        return true;
    }

    void mainLoop()
    {
        // Create GLFW loop.
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    // In some cases / implementations, a destructor is used instead of this method.
    // Destructor is a method that is called when an object is terminated (we do not have one!)
    // Destructor is normally used to free internal pointers, is good practice.
    void cleanup()
    {
        // very important - we DON'T have a garbage collector so we need to clean up
        // Clean GFLW
        biniutils::logstdout("Cleaning up application.");

        glfwDestroyWindow(window);
        glfwTerminate();

        // 34 - Clean before device.
        vkDestroySwapchainKHR(device, swapChain, nullptr);

        // 1.10 - Destroy the logical device.
        vkDestroyDevice(device, nullptr);

        // 16 - Clean the surface.
        // KHR es de extensiones, en este caso, la extension de dibujar en un surface.
        vkDestroySurfaceKHR(instance, surface, nullptr);

        // Clean Vulkan
        vkDestroyInstance(instance, nullptr);
    }
};

int main()
{
    FirstVulkanExample app;

    try
    {
        biniutils::logstdout("Initializing application.");
        app.run();
    }
    catch (const std::exception &e)
    {
        // Salida para errores
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}