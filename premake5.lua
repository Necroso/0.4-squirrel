workspace "SquirrelPlugin"

    ----------------------------------------------------------------------------
    -- Workspace
    ----------------------------------------------------------------------------

    configurations
    {
        "Debug",
        "Release"
    }

    platforms
    {
        "x86",
        "x64"
    }

    location "build"

    startproject "SquirrelPlugin"

    ----------------------------------------------------------------------------
    -- Global Settings
    ----------------------------------------------------------------------------

    filter "platforms:x86"
        architecture "x86"

    filter "platforms:x64"
        architecture "x86_64"

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter "system:linux"
        pic "On"

    ----------------------------------------------------------------------------
    -- Character Encoding
    ----------------------------------------------------------------------------

    filter { "system:windows", "configurations:Debug" }
        characterset "Unicode"

    filter { "system:windows", "configurations:Release" }
        characterset "MBCS"

    filter "configurations:Debug"

        runtime "Debug"

        symbols "On"
        optimize "Off"

        defines
        {
            "DEBUG",
            "_DEBUG",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Release"

        runtime "Release"

        symbols "Off"
        optimize "Full"

        defines
        {
            "NDEBUG",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter {}

--------------------------------------------------------------------------------
-- Project: squirrel
--------------------------------------------------------------------------------

project "squirrel"

    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"
    objdir    "build/obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"

    includedirs
    {
        "squirrelsrc/include",
        "squirrelsrc/squirrel"
    }

    files
    {
        "squirrelsrc/squirrel/**.h",
        "squirrelsrc/squirrel/**.cpp"
    }

--------------------------------------------------------------------------------
-- Project: sqstdlib
--------------------------------------------------------------------------------

project "sqstdlib"

    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"
    objdir    "build/obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"

    includedirs
    {
        "squirrelsrc/include",
        "squirrelsrc/squirrel"
    }

    files
    {
        "squirrelsrc/sqstdlib/**.h",
        "squirrelsrc/sqstdlib/**.cpp"
    }

--------------------------------------------------------------------------------
-- Project: SquirrelPlugin
--------------------------------------------------------------------------------

project "SquirrelPlugin"

    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"
    objdir    "build/obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"

    includedirs
    {
        ".",
        "sqrat",
        "sqrat/sqrat",
        "squirrelsrc/include"
    }

    files
    {
        "*.h",
        "*.cpp"
    }

    links
    {
        "squirrel",
        "sqstdlib"
    }

    ----------------------------------------------------------------------------
    -- Output File Name
    ----------------------------------------------------------------------------

    filter "configurations:Debug"
        targetname "squirrel04dbg"

    filter "configurations:Release"
        targetname "squirrel04rel"

    filter "platforms:x86"
        targetsuffix "32"

    filter "platforms:x64"
        targetsuffix "64"

    ----------------------------------------------------------------------------
    -- Windows
    ----------------------------------------------------------------------------

    filter "system:windows"

        links
        {
            "ws2_32"
        }

    ----------------------------------------------------------------------------
    -- Linux
    ----------------------------------------------------------------------------

    filter "system:linux"

        targetprefix ""

        links
        {
            "pthread",
            "dl",
            "m"
        }

    filter {}