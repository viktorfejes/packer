workspace "Packer"
    configurations { "Debug", "Release" }
    architecture "x64"
	startproject "Packer"

project "Packer"
    location "Packer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}"

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    filter "configurations:Debug"
        defines "PACKER_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "PACKER_RELEASE"
        runtime "Release"
        optimize "on"