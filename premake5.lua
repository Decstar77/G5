local GLFW_DIR = "vendor/glfw"
local GLAD_DIR = "vendor/glad"
local GLM_DIR = "vendor/glm"
local OPENAL_DIR = "vendor/openal"
local AUDIO_FILE_DIR = "vendor/audio"
local FREE_TYPE_DIR = "vendor/freetype"
local STB_DIR = "vendor/stb"
local JSON_DIR = "vendor/json"
local ENET_DIR  = "vendor/enet"
local ABSOLUTE_SOL_PATH = path.getabsolute(".")

solution "Atto"
    location("")
    startproject "Atto"
    configurations { "Release", "Debug" }
    platforms "x64"
    architecture "x64"
    
    filter "configurations:Release"
        defines {
            "NDEBUG"
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
    }

    includedirs
    {
        path.join(GLFW_DIR, "include"),
        path.join(GLAD_DIR, "include"),
        path.join(OPENAL_DIR, "include"),
        path.join(FREE_TYPE_DIR, "include"),
        path.join(ENET_DIR, "include"),
        JSON_DIR,
        STB_DIR,
        GLM_DIR,
        AUDIO_FILE_DIR,
        "atto/src/"
    }
    
    libdirs
    {
        path.join(OPENAL_DIR, "lib"),
    }
    
    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        path.join(STB_DIR, "stb_vorbis/stb_vorbis.c")
    }

    links { "opengl32", "glfw", "glad", "OpenAL32", "freetype" }

    filter "system:windows"
        links { "kernel32", "user32", "ws2_32", "winmm" }

    dependson { "AttoTypeGen", "atto_game"}

    prebuildcommands
    {
        string.format("\"%s\"", path.join(ABSOLUTE_SOL_PATH, "bin/%{cfg.architecture}/atto_type_gen.exe"))
    }

project "atto_game"
    location("atto_game")

    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"
    warnings "Default"
    flags { "FatalWarnings", "MultiProcessorCompile" }
    debugdir "bin"
    
    includedirs
    {
        JSON_DIR,
        GLM_DIR,
        path.join(ENET_DIR, "include")
    }

    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        "atto_core/src/shared/**.h",
        "atto_core/src/shared/**.cpp",
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

    dependson { "AttoTypeGen", "atto_core" }

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
    
    includedirs
    {
        JSON_DIR,
        GLM_DIR,
        path.join(ENET_DIR, "include")
    }

    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        "atto_core/src/shared/**.h",
        "atto_core/src/shared/**.cpp",
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

    dependson { "AttoTypeGen", "atto_core" }

project "atto_type_gen"
    location("atto_type_gen")
    kind "ConsoleApp"
    language "C#"
    dotnetframework "4.7.2"
    
    files {
        "atto_type_gen/src/**.cs",
    }
    
    links {
        "System",
        "System.Core",
        "System.Data",
        "System.Net.Http",
    }

    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

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

project "freetype"
    location(FREE_TYPE_DIR)
    kind "StaticLib"
    language "C"
    
    defines {
        "_LIB",
        "FT2_BUILD_LIBRARY",
        "_CRT_SECURE_NO_WARNINGS",
        "_CRT_NONSTDC_NO_WARNINGS"
    }
    
    includedirs {
        path.join(FREE_TYPE_DIR, "include")
    }

    files {
        path.join(FREE_TYPE_DIR, "src/autofit/autofit.c"),
        path.join(FREE_TYPE_DIR, "src/bdf/bdf.c"),
        path.join(FREE_TYPE_DIR, "src/cff/cff.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftbase.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftbitmap.c"),
        path.join(FREE_TYPE_DIR, "src/cache/ftcache.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftfstype.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftgasp.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftglyph.c"),
        path.join(FREE_TYPE_DIR, "src/gzip/ftgzip.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftinit.c"),
        path.join(FREE_TYPE_DIR, "src/lzw/ftlzw.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftstroke.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftsystem.c"),
        path.join(FREE_TYPE_DIR, "src/smooth/smooth.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftbbox.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftmm.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftpfr.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftsynth.c"),
        path.join(FREE_TYPE_DIR, "src/base/fttype1.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftwinfnt.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftlcdfil.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftgxval.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftotval.c"),
        path.join(FREE_TYPE_DIR, "src/base/ftpatent.c"),
        path.join(FREE_TYPE_DIR, "src/pcf/pcf.c"),
        path.join(FREE_TYPE_DIR, "src/pfr/pfr.c"),
        path.join(FREE_TYPE_DIR, "src/psaux/psaux.c"),
        path.join(FREE_TYPE_DIR, "src/pshinter/pshinter.c"),
        path.join(FREE_TYPE_DIR, "src/psnames/psmodule.c"),
        path.join(FREE_TYPE_DIR, "src/raster/raster.c"),
        path.join(FREE_TYPE_DIR, "src/sfnt/sfnt.c"),
        path.join(FREE_TYPE_DIR, "src/truetype/truetype.c"),
        path.join(FREE_TYPE_DIR, "src/type1/type1.c"),
        path.join(FREE_TYPE_DIR, "src/cid/type1cid.c"),
        path.join(FREE_TYPE_DIR, "src/type42/type42.c"),
        
        path.join(FREE_TYPE_DIR, "src/sdf/ftbsdf.c"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdf.c"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdf.c"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdfcommon.c"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdfcommon.h"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdferrs.h"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdfrend.c"),
        path.join(FREE_TYPE_DIR, "src/sdf/ftsdfrend.h"),
        path.join(FREE_TYPE_DIR, "src/sdf/sdf.c"),

        path.join(FREE_TYPE_DIR, "src/svg/ftsvg.c"),
        path.join(FREE_TYPE_DIR, "src/svg/ftsvg.h"),
        path.join(FREE_TYPE_DIR, "src/svg/svg.c"),
        path.join(FREE_TYPE_DIR, "src/svg/svgtypes.h"),

        path.join(FREE_TYPE_DIR, "src/winfonts/winfnt.c"),
        path.join(FREE_TYPE_DIR, "include/ft2build.h"),
        path.join(FREE_TYPE_DIR, "include/freetype/config/ftconfig.h"),
        path.join(FREE_TYPE_DIR, "include/freetype/config/ftheader.h"),
        path.join(FREE_TYPE_DIR, "include/freetype/config/ftmodule.h"),
        path.join(FREE_TYPE_DIR, "include/freetype/config/ftoption.h"),
        path.join(FREE_TYPE_DIR, "include/freetype/config/ftstdlib.h")
    }

    filter "system:windows"
            files {
                path.join(FREE_TYPE_DIR, "builds/win32/ftdebug.c")
            }

