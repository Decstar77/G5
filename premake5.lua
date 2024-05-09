local GLFW_DIR = "vendor/glfw"
local GLAD_DIR = "vendor/glad"
local GLM_DIR = "vendor/glm"
local OPENAL_DIR = "vendor/openal"
local ASSIMP_DIR = "vendor/assimp"
local AUDIO_FILE_DIR = "vendor/audio"
local STB_DIR = "vendor/stb"
local JSON_DIR = "vendor/json"
local ENET_DIR  = "vendor/enet"
local FONTSTASH_DIR = "vendor/fontstash"
local ABSOLUTE_SOL_PATH = path.getabsolute(".")
local FMOD_DIR = "vendor/fmod"
local FPM = "vendor/fpm"
local BACKWARD = "vendor/backward"
local VULKAN_DIR = "C:/VulkanSDK/1.3.239.0"
local NUKLEAR_DIR = "vendor/nuklear"
local ZT_DIR = "vendor/zt"

solution "Atto"
    location("")
    startproject "Atto"
    configurations { "Release", "Debug" }
    platforms "x64"
    architecture "x64"
    
    filter "configurations:Release"
        defines {
            "_RELEASE"
        }
        optimize "Full"
    filter "configurations:Debug*"
        defines{
            "_DEBUG"
        }
        optimize "Debug"
        symbols "On"

project "atto_core"
    location("atto_core")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"
    warnings "Default"
    flags { "FatalWarnings", "MultiProcessorCompile" }	
    debugdir "bin"
    
    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

    disablewarnings { 
        "4057", -- Slightly different base types. Converting from type with volatile to without.
        "4100", -- Unused formal parameter. I think unusued parameters are good for documentation.
        "4152", -- Conversion from function pointer to void *. Should be ok.
        "4200", -- Zero-sized array. Valid C99.
        "4201", -- Nameless struct/union. Valid C11.
        "4204", -- Non-constant aggregate initializer. Valid C99.
        "4206", -- Translation unit is empty. Might be #ifdefed out.
        "4214", -- Bool bit-fields. Valid C99.
        "4221", -- Pointers to locals in initializers. Valid C99.
        "4702", -- Unreachable code. We sometimes want return after exit() because otherwise we get an error about no return value.
        "4996"  -- Deprecated functions
    }

    includedirs
    {
        path.join(GLFW_DIR, "include"),
        path.join(GLAD_DIR, "include"),
        path.join(FMOD_DIR, "include"),
        path.join(ENET_DIR, "include"),
		path.join(VULKAN_DIR, "include"),
        path.join(ZT_DIR, "include"),
        JSON_DIR,
        STB_DIR,
        FPM,
        BACKWARD,
        path.join(FONTSTASH_DIR, "src"),
        GLM_DIR,
        AUDIO_FILE_DIR,
        "atto/src/",
        NUKLEAR_DIR
    }
    
    libdirs
    {
        path.join(FMOD_DIR, "lib"),
        path.join(VULKAN_DIR, "lib"),
        path.join(ZT_DIR, "lib")
    }
    
    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        path.join(STB_DIR, "stb_vorbis/stb_vorbis.c")
    }

    links { "opengl32", "glfw", "glad", "fmod_vc", "libzt" }

    filter "system:windows"
        links { "kernel32", "user32", "Dbghelp", "vulkan-1" }

    dependson { "AttoTypeGen", "atto_game" }

    prebuildcommands
    {
    }

project "atto_server"
    location("atto_server")

    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"
    warnings "Default"
    flags { "FatalWarnings", "MultiProcessorCompile" }
    debugdir "bin"
    
    defines { "ATTO_SERVER" }

    includedirs
    {
        path.join(GLFW_DIR, "include"),
        JSON_DIR,
        GLM_DIR,
        FPM,
        path.join(ENET_DIR, "include")
    }

    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        ---"atto_core/src/shared/**.h",
        ---"atto_core/src/shared/**.cpp",
        ---"atto_core/src/sim/**.h",
        ---"atto_core/src/sim/**.cpp",
    }

    disablewarnings { 
        "4057", -- Slightly different base types. Converting from type with volatile to without.
        "4100", -- Unused formal parameter. I think unusued parameters are good for documentation.
        "4152", -- Conversion from function pointer to void *. Should be ok.
        "4200", -- Zero-sized array. Valid C99.
        "4201", -- Nameless struct/union. Valid C11.
        "4204", -- Non-constant aggregate initializer. Valid C99.
        "4206", -- Translation unit is empty. Might be #ifdefed out.
        "4214", -- Bool bit-fields. Valid C99.
        "4221", -- Pointers to locals in initializers. Valid C99.
        "4702", -- Unreachable code. We sometimes want return after exit() because otherwise we get an error about no return value.
    }

    links { "glfw" }

    filter "system:windows"
        links { "kernel32", "user32" }

    dependson { "AttoTypeGen", "atto_core" }

project "glad"
    location(GLAD_DIR)
    kind "StaticLib"
    language "C"
    files {
        path.join(GLAD_DIR, "inlcude/glad/glad.h"),
        path.join(GLAD_DIR, "inlcude/KHR/khrplatform.h"),
        path.join(GLAD_DIR, "src/glad.c"),
    }
    includedirs { 
        path.join(GLAD_DIR, "include") 
    }

    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"

project "glfw"
    location(GLFW_DIR)
    kind "StaticLib"
    language "C"
    files
    {
        path.join(GLFW_DIR, "include/GLFW/*.h"),
        path.join(GLFW_DIR, "src/context.c"),
        path.join(GLFW_DIR, "src/egl_context.*"),
        path.join(GLFW_DIR, "src/init.c"),
        path.join(GLFW_DIR, "src/input.c"),
        path.join(GLFW_DIR, "src/internal.h"),
        path.join(GLFW_DIR, "src/monitor.c"),
        path.join(GLFW_DIR, "src/null*.*"),
        path.join(GLFW_DIR, "src/osmesa_context.*"),
        path.join(GLFW_DIR, "src/platform.c"),
        path.join(GLFW_DIR, "src/vulkan.c"),
        path.join(GLFW_DIR, "src/window.c"),
    }
    includedirs { path.join(GLFW_DIR, "include") }
    filter "system:windows"
        defines "_GLFW_WIN32"
        files
        {
            path.join(GLFW_DIR, "src/win32_*.*"),
            path.join(GLFW_DIR, "src/wgl_context.*")
        }

    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        