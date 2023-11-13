newoption {
	trigger = "name",
	description = "Name of application"
}

ApplicationName = _OPTIONS["name"]

workspace (""..ApplicationName)
	architecture "x64"
	location ("../EunoiaProjects/"..ApplicationName)

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project (""..ApplicationName)
	location ("../EunoiaProjects/"..ApplicationName)
	kind "SharedLib"
	language "C++"

	targetdir ("../EunoiaProjects/"..ApplicationName.."/Bin/"..outputdir)
	objdir ("../EunoiaProjects/"..ApplicationName.."/Bin-Int/"..outputdir)
	symbolspath("$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString(\"HH_mm_ss_fff\")).pdb")

    files
	{
		"../EunoiaProjects/%{prj.name}/Src/**.h",
		"../EunoiaProjects/%{prj.name}/Src/**.cpp",
	}

    includedirs
	{
		"../Eunoia-Engine/Src",
		"../Eunoia-Engine/Libs/Vulkan/Include",
		"../Eunoia-Engine/Libs/Bullet/Include",
		"../Eunoia-Engine/Libs/FreeType/Include"
	}

    links
	{
		"../Bin/"..outputdir.."/Eunoia-Engine/Eunoia-Engine"
	}

    defines
	{
		"EU_PROJECT",
		"STB_IMAGE_IMPLEMENTATION",
		"EU_PROJ=__declspec(dllexport)"
	}

    prebuildcommands
	{
		--"del Projects/%{prj.name}/Bin/"..outputdir.."/*.pdb > NUL 2> NUL",
		"call \"../../Bin/Dist-windows-x86_64/Eunoia-Introspection/Eunoia-Introspection.exe\" \"Src/"..ApplicationName..".h\" \"Src/"..ApplicationName.."_Generated.cpp\""
	}

    filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines
		{
			"EU_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "EU_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "EU_RELEASE"
		symbols "On"
		optimize "On"

	filter "configurations:Dist"
		defines "EU_DIST"
		symbols "On"
		optimize "On"

