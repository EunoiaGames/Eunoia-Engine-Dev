workspace "Eunoia-Dev"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Eunoia-Engine"
	location "Eunoia-Engine"
	kind "SharedLib"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
		"%{prj.name}/Src/**.c"
	}

	includedirs
	{
		"%{prj.name}/Libs/Vulkan/Include",
		"%{prj.name}/Libs/FreeType/Include",
		"%{prj.name}/Libs/Bullet/Include"
	}

	links  
	{
		"Eunoia-Engine/Libs/Vulkan/x64/vulkan-1",
		"Eunoia-Engine/Libs/FreeType/x64/freetype.lib",
		"Xinput9_1_0"
	}

	defines
	{
		"EU_BUILD_DLL",
		"EU_ENGINE",
	}

	prebuildcommands
	{
		"call \"../Bin/Dist-windows-x86_64/Eunoia-Introspection/Eunoia-Introspection.exe\" \"Src/Eunoia/Eunoia.h\" \"Src/Eunoia/Metadata/EunoiaGenerated.cpp\""
	}

	filter "configurations:Debug"
		buildoptions "/MDd" 
		links
		{
			"Eunoia-Engine/Libs/Bullet/x64/Debug/BulletCollision_Debug",
			"Eunoia-Engine/Libs/Bullet/x64/Debug/BulletDynamics_Debug",
			"Eunoia-Engine/Libs/Bullet/x64/Debug/BulletSoftBody_Debug",
			"Eunoia-Engine/Libs/Bullet/x64/Debug/LinearMath_Debug"
		}

	filter "configurations:Release"
		buildoptions "/MD"
		links 
		{
			"Eunoia-Engine/Libs/Bullet/x64/BulletCollision",
			"Eunoia-Engine/Libs/Bullet/x64/BulletDynamics",
			"Eunoia-Engine/Libs/Bullet/x64/BulletSoftBody",
			"Eunoia-Engine/Libs/Bullet/x64/LinearMath"
		}


	filter "configurations:Dist"
		buildoptions "/MD"
		links 
		{
			"Eunoia-Engine/Libs/Bullet/x64/BulletCollision",
			"Eunoia-Engine/Libs/Bullet/x64/BulletDynamics",
			"Eunoia-Engine/Libs/Bullet/x64/BulletSoftBody",
			"Eunoia-Engine/Libs/Bullet/x64/LinearMath"
		}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines
		{
			"EU_PLATFORM_WINDOWS"
		}

		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../Bin/" .. outputdir .. "/Eunoia-Editor",
		}

	filter "configurations:Debug"
		defines "EU_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "EU_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "EU_DIST"
		optimize "On"

project "Eunoia-EumdlModelConverter"
	location "Eunoia-EumdlModelConverter"
	kind  "ConsoleApp"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/Libs/Assimp/include"
	}

	filter "configurations:Debug"
		symbols "On"
		links "Eunoia-EumdlModelConverter/Libs/Assimp/Debug/assimp-vc142-mtd"

	filter "configurations:Release"
		optimize "Off"
		links "Eunoia-EumdlModelConverter/Libs/Assimp/assimp-vc142-mtd"

	filter "configurations:Dist"
		optimize "Off"
		links "Eunoia-EumdlModelConverter/Libs/Assimp/assimp-vc142-mtd"


project "Eunoia-EutexTextureConverter"
	location "Eunoia-EutexTextureConverter"
	kind  "ConsoleApp"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Dist"
		optimize "On"

project "Eunoia-Introspection"
	location "Eunoia-Introspection"
	kind "ConsoleApp"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
	}

	filter "configurations:Debug"
		defines "EU_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "EU_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "EU_DIST"
		optimize "On"

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

project "Eunoia-Editor"
	location "Eunoia-Editor"
	kind "ConsoleApp"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
	}

	includedirs
	{
		"Eunoia-Engine/Src",
		"Eunoia-Engine/Libs/Vulkan/Include",
		"Eunoia-Engine/Libs/Bullet/Include",
		"Eunoia-Engine/Libs/FreeType/Include"
	}

	links
	{
		"Eunoia-Engine"
	}

	defines
	{
		"EU_EDITOR",
		"STB_IMAGE_IMPLEMENTATION",
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
		buildoptions "/MDd" 

		links
		{
			"Eunoia-Editor/Libs/Bullet/x64/Debug/BulletCollision_Debug",
			"Eunoia-Editor/Libs/Bullet/x64/Debug/BulletDynamics_Debug",
			"Eunoia-Editor/Libs/Bullet/x64/Debug/BulletSoftBody_Debug",
			"Eunoia-Editor/Libs/Bullet/x64/Debug/LinearMath_Debug"
		}

	filter "configurations:Release"
		defines "EU_RELEASE"
		optimize "On"
		buildoptions "/MD" 

		links
		{
			"Eunoia-Editor/Libs/Bullet/x64/BulletCollision",
			"Eunoia-Editor/Libs/Bullet/x64/BulletDynamics",
			"Eunoia-Editor/Libs/Bullet/x64/BulletSoftBody",
			"Eunoia-Editor/Libs/Bullet/x64/LinearMath"
		}

	filter "configurations:Dist"
		defines "EU_DIST"
		optimize "On"
		buildoptions "/MD" 

		links
		{
			"Eunoia-Editor/Libs/Bullet/x64/BulletCollision",
			"Eunoia-Editor/Libs/Bullet/x64/BulletDynamics",
			"Eunoia-Editor/Libs/Bullet/x64/BulletSoftBody",
			"Eunoia-Editor/Libs/Bullet/x64/LinearMath"
		}

project "Eunoia-ShaderCompiler"
	location "Eunoia-ShaderCompiler"
	kind "ConsoleApp"
	language "C++"

	targetdir ("Bin/"..outputdir.."/%{prj.name}")
	objdir ("Bin-Int/"..outputdir.."/%{prj.name}")

	files
	{
		"%{prj.name}/Src/**.h",
		"%{prj.name}/Src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/Libs/shaderc/Include"
	}

	filter "system:windows"
		defines
		{
			"EU_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		buildoptions "/MDd" 

		links
		{
			"Eunoia-ShaderCompiler/Libs/shaderc/Debug/shaderc_combinedd"
		}

	filter "configurations:Release"
		buildoptions "/MD"

		links
		{
			"Eunoia-ShaderCompiler/Libs/shaderc/shaderc_combined"
		}

	filter "configurations:Dist"
		buildoptions "/MD"

		links
		{
			"Eunoia-ShaderCompiler/Libs/shaderc/shaderc_combined"
		}
