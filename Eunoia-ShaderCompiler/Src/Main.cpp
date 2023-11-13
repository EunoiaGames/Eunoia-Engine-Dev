#include <shaderc\shaderc.h>
#include <vector>
#include "String.h"
#include "FileUtils.h"

typedef unsigned int u32;

static shaderc_compiler_t			ShaderCompiler;
static shaderc_compile_options_t	ShaderCompilerOptions;

struct MacroDefinition
{
	String name;
	String value;
};

std::vector<MacroDefinition> Macros;

static void ReadMacroFile()
{
	b32 loaded;
	String macros = FileUtils::LoadTextFile("Macros.txt", &loaded);

	if (!loaded)
	{
		FileUtils::WriteFile("Error.txt", "Could not find macro file");
		return;
	}

	u32 startIndex = 0;
	s32 commaIndex = macros.FindFirstOf(",");
	while (commaIndex != -1)
	{
		String macroDefinition = macros.SubString(startIndex, commaIndex - 1);
		macroDefinition = macroDefinition.Trim();
		s32 spaceIndex = macroDefinition.FindFirstOf(" ");
		MacroDefinition macro;
		macro.name = macroDefinition.SubString(0, spaceIndex - 1).Trim();
		macro.value = macroDefinition.SubString(spaceIndex + 1).Trim();

		Macros.push_back(macro);

		startIndex = commaIndex + 1;
		commaIndex = macros.FindFirstOf(",", commaIndex + 1);
	}

	String lastDefinition = macros.SubString(startIndex).Trim();
	s32 spaceIndex = lastDefinition.FindFirstOf(" ");
	MacroDefinition macro;
	macro.name = lastDefinition.SubString(0, spaceIndex - 1).Trim();
	macro.value = lastDefinition.SubString(spaceIndex + 1).Trim();
	Macros.push_back(macro);
}

static void Init()
{
	ShaderCompiler = shaderc_compiler_initialize();
	ShaderCompilerOptions = shaderc_compile_options_initialize();
	
	for (u32 i = 0; i < Macros.size(); i++)
	{
		shaderc_compile_options_add_macro_definition(ShaderCompilerOptions, Macros[i].name.C_Str(),
			Macros[i].name.Length(), Macros[i].value.C_Str(), Macros[i].value.Length());
	}

	shaderc_compile_options_set_source_language(ShaderCompilerOptions, shaderc_source_language_glsl);
	shaderc_compile_options_set_optimization_level(ShaderCompilerOptions, shaderc_optimization_level_zero);
	shaderc_compile_options_set_target_env(ShaderCompilerOptions, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
	shaderc_compile_options_set_generate_debug_info(ShaderCompilerOptions);
}

static void CompileShaderAndWriteToFile(const String& inputPath)
{
	String shaderText = FileUtils::LoadTextFileWithIncludePreProcessor(inputPath);

	s32 vertexFlagIndex = shaderText.FindFirstOf("#EU_Vertex");
	s32 fragmentFlagIndex = shaderText.FindFirstOf("#EU_Fragment");

	if (vertexFlagIndex == -1)
		printf("Could not find #EU_Vertex flag in shader");
	if (fragmentFlagIndex == -1)
		printf("Could not find #EU_Fragment flag in shader");
	if (vertexFlagIndex == -1 || fragmentFlagIndex == -1)
		return;

	String vertexCode = shaderText.SubString(vertexFlagIndex + 10, fragmentFlagIndex - 1);
	String fragmentCode = shaderText.SubString(fragmentFlagIndex + 12);

	shaderc_compilation_result_t vertexResult = shaderc_compile_into_spv(ShaderCompiler, vertexCode.C_Str(),
		vertexCode.Length(), shaderc_vertex_shader, inputPath.C_Str(), "main", ShaderCompilerOptions);

	shaderc_compilation_result_t fragmentResult = shaderc_compile_into_spv(ShaderCompiler, fragmentCode.C_Str(),
		fragmentCode.Length(), shaderc_fragment_shader, inputPath.C_Str(), "main", ShaderCompilerOptions);

	const char* vertexErrorMsg = shaderc_result_get_error_message(vertexResult);
	const char* fragmentErrorMsg = shaderc_result_get_error_message(fragmentResult);

	if (shaderc_result_get_num_errors(vertexResult) > 0)
		printf(vertexErrorMsg);
	if (shaderc_result_get_num_errors(fragmentResult) > 0)
		printf(fragmentErrorMsg);

	const char* vertexBytes = shaderc_result_get_bytes(vertexResult);
	u32 vertexLength = shaderc_result_get_length(vertexResult);;
	const char* fragmentBytes = shaderc_result_get_bytes(fragmentResult);
	u32 fragmentLength = shaderc_result_get_length(fragmentResult);

	s32 lastDotIndex = inputPath.FindLastOf(".");
	String nameWithoutExtension = inputPath.SubString(0, lastDotIndex - 1);

	String errorText = "";

	if (!vertexBytes)
	{
		errorText += String("EU_Vertex\n") + vertexErrorMsg + "\n";
	}
	if (!fragmentBytes)
	{
		errorText += String("EU_Fragment\n") + fragmentErrorMsg;
	}
	if (!vertexBytes || !fragmentBytes)
	{
		errorText += "\n" + Macros[0].name + " = " + Macros[0].value;
		FileUtils::WriteFile(nameWithoutExtension + "-CompilationError.txt", errorText);
	}

	String vertexOutputName = nameWithoutExtension + ".vert.spirv";
	String fragmentOutputName = nameWithoutExtension + ".frag.spirv";

	FileUtils::WriteFileBinary(vertexOutputName, (const u8*)vertexBytes, vertexLength);
	FileUtils::WriteFileBinary(fragmentOutputName, (const u8*)fragmentBytes, fragmentLength);
}

int main(int argc, char** args)
{
	ReadMacroFile();

	Init();

	for (u32 i = 1; i < argc; i++)
	{
		CompileShaderAndWriteToFile(args[i]);
	}
}