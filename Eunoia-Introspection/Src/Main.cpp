#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long int s64;
typedef u32 b32;
typedef float r32;
typedef double r64;

static FILE* GeneratedFile;
static std::string InputFileName;
static bool IsEngine;
static std::vector<std::string> ParsedFiles;
static u32 CurrentID;
static std::vector<std::string> MetaTypes;

enum token_type
{
	TOKEN_IDENTIFIER,
	TOKEN_OPEN_PAREN,
	TOKEN_CLOSE_PAREN,
	TOKEN_OPEN_BRACKET,
	TOKEN_CLOSE_BRACKET,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_STRING_LITERAL,
	TOKEN_ASTERISK,
	TOKEN_NUMBER_LITERAL,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_SUBTRACT,
	TOKEN_DIVIDE,
	TOKEN_DOT,
	TOKEN_MOD,
	TOKEN_COMMA,
	TOKEN_LESS_THAN,
	TOKEN_GREATER_THAN,
	TOKEN_EQUAL,
	TOKEN_AND,
	TOKEN_PIPE,
	TOKEN_TILDE,
	TOKEN_POUND,
	TOKEN_CHAR_LITERAL,
	TOKEN_ARROW,
	TOKEN_STRUCT,
	TOKEN_CLASS,
	TOKEN_ENUM,

	TOKEN_EOS,
	NUM_TOKEN_TYPES,
	UNKNOWN_TOKEN_TYPE
};

enum StorageAccessModifier
{
	ACCESS_MODIFIER_PUBLIC,
	ACCESS_MODIFIER_PRIVATE,
	ACCESS_MODIFIER_PROTECTED
};

struct token
{
	token_type Type;
	char* Text;
	u32 Length;
};

struct tokenizer
{
	char* At;
};

struct enum_value
{
	std::string Name;
	std::string Value;
};

struct class_member
{
	std::string TypeName;
	std::string Name;
	std::string Offset;
	std::string Size;
	std::string AccessModifer;
	std::string IsStatic;
	std::string IsConst;
	std::string IsPointer;
	std::string ArrayLength;
	std::string UIMin;
	std::string UIMax;
	std::string UISpeed;
	std::string Is32BitBool;
};

static bool IsEndOfLine(char C)
{
	return C == '\n' || C == '\r';
}

static bool IsWhitespace(char C)
{
	return C == ' ' || C == '\t' || IsEndOfLine(C);
}

static void EatWhitespace(tokenizer* Tokenizer)
{
	while (true)
	{
		if (IsWhitespace(Tokenizer->At[0]))
			Tokenizer->At++;
		else if (Tokenizer->At[0] == '/' && Tokenizer->At[1] == '/')
		{
			Tokenizer->At += 2;
			while (Tokenizer->At[0] && !IsEndOfLine(Tokenizer->At[0]))
				Tokenizer->At++;
		}
		else if (Tokenizer->At[0] == '/' && Tokenizer->At[1] == '*')
		{
			Tokenizer->At += 2;
			while (Tokenizer->At[0] && !(Tokenizer->At[0] == '*' && Tokenizer->At[1] == '/'))
				Tokenizer->At[0]++;
			if (Tokenizer->At[0] == '*')
				Tokenizer->At += 2;
		}
		else
			break;
	}


}

static bool IsNumber(char C)
{
	bool IsDigit = (C >= '0') && (C <= '9');
	return IsDigit;
}

static bool IsAlpha(char C)
{
	return ((C >= 'a') && (C <= 'z')) ||
		((C >= 'A') && (C <= 'Z'));
}

static bool StringEqual(const char* s1, u32 l1, const char* s2, u32 l2)
{
	if (l1 != l2)
		return false;

	for (u32 i = 0; i < l1; i++)
		if (s1[i] != s2[i])
			return false;

	return true;
}

static token GetToken(tokenizer* Tokenizer)
{
	EatWhitespace(Tokenizer);

	token Token;
	Token.Length = 1;
	Token.Text = Tokenizer->At;

	switch (Tokenizer->At[0])
	{
	case '\0': { Token.Type = TOKEN_EOS; Tokenizer->At++; } break;
	case '(': { Token.Type = TOKEN_OPEN_PAREN; Tokenizer->At++; } break;
	case ')': { Token.Type = TOKEN_CLOSE_PAREN; Tokenizer->At++; } break;
	case '{': { Token.Type = TOKEN_OPEN_BRACE; Tokenizer->At++; } break;
	case '}': { Token.Type = TOKEN_CLOSE_BRACE; Tokenizer->At++; } break;
	case '[': { Token.Type = TOKEN_OPEN_BRACKET; Tokenizer->At++; } break;
	case ']': { Token.Type = TOKEN_CLOSE_BRACKET; Tokenizer->At++; } break;
	case '*': { Token.Type = TOKEN_ASTERISK; Tokenizer->At++; } break;
	case '+': { Token.Type = TOKEN_PLUS; Tokenizer->At++; } break;
	case '%': { Token.Type = TOKEN_MOD; Tokenizer->At++; } break;
	case ':': { Token.Type = TOKEN_COLON; Tokenizer->At++; } break;
	case ';': { Token.Type = TOKEN_SEMICOLON; Tokenizer->At++; } break;
	case '.': { Token.Type = TOKEN_DOT; Tokenizer->At++; } break;
	case ',': { Token.Type = TOKEN_COMMA; Tokenizer->At++; } break;
	case '<': { Token.Type = TOKEN_LESS_THAN; Tokenizer->At++; } break;
	case '>': { Token.Type = TOKEN_GREATER_THAN; Tokenizer->At++; } break;
	case '=': { Token.Type = TOKEN_EQUAL; Tokenizer->At++; } break;
	case '&': { Token.Type = TOKEN_AND; Tokenizer->At++; } break;
	case '|': { Token.Type = TOKEN_PIPE; Tokenizer->At++; } break;
	case '~': { Token.Type = TOKEN_TILDE; Tokenizer->At++; } break;
	case '#': { Token.Type = TOKEN_POUND; Tokenizer->At++; } break;
	case '/': { Token.Type = TOKEN_DIVIDE; Tokenizer->At++; } break;
	case '-': {
		if (Tokenizer->At[1] != '>')
		{
			Token.Type = TOKEN_MINUS;
		}
		else
		{
			Token.Type = TOKEN_ARROW;
			Token.Length = 2;
			Tokenizer->At++;
		}
		Tokenizer->At++;
	} break;
	case '\'': {
		Tokenizer->At++;
		Token.Type = TOKEN_CHAR_LITERAL;
		Token.Text = Tokenizer->At;
		Token.Length = 1;
		Tokenizer->At++;
		Tokenizer->At++;
	} break;
	case '"': {
		Tokenizer->At++;
		Token.Type = TOKEN_STRING_LITERAL;
		Token.Text = Tokenizer->At;
		while (Tokenizer->At[0] != '"' &&
			Tokenizer->At[0] != '\0')
		{
			if (Tokenizer->At[0] == '\\' && Tokenizer->At[1] != '\0')
				Tokenizer->At++;

			Tokenizer->At++;
		}
		Token.Length = Tokenizer->At - Token.Text;
		if (Tokenizer->At[0] == '"')
			Tokenizer->At++;
	} break;
	default: {
		if (IsAlpha(Tokenizer->At[0]) || Tokenizer->At[0] == '_')
		{
			Tokenizer->At++;
			Token.Type = TOKEN_IDENTIFIER;
			while (IsAlpha(Tokenizer->At[0]) || IsNumber(Tokenizer->At[0]) || Tokenizer->At[0] == '_')
				Tokenizer->At++;
			Token.Length = Tokenizer->At - Token.Text;

			if (StringEqual(Token.Text, Token.Length, "class", 5))
				Token.Type = TOKEN_CLASS;
			else if (StringEqual(Token.Text, Token.Length, "struct", 6))
				Token.Type = TOKEN_STRUCT;
			else if (StringEqual(Token.Text, Token.Length, "enum", 4))
				Token.Type = TOKEN_ENUM;
		}
		else if (IsNumber(Tokenizer->At[0]))
		{
			Tokenizer->At++;
			Token.Type = TOKEN_NUMBER_LITERAL;
			bool HasDecimal = false;
			while (true)
			{
				if (IsNumber(Tokenizer->At[0]))
				{
					Tokenizer->At++;
				}
				else if (Tokenizer->At[0] == '.')
				{
					if (HasDecimal)
					{
						//ERROR
						Tokenizer->At++;
					}

					HasDecimal = true;
					Tokenizer->At++;
				}
				else
				{
					break;
				}
			}

			if (Tokenizer->At[0] == 'f')
				Tokenizer->At++;

			Token.Length = Tokenizer->At - Token.Text;
		}
		else
		{
			Tokenizer->At++;
			Token.Type = UNKNOWN_TOKEN_TYPE;
		}
	} break;
	}

	return Token;
}

static char* ReadFileIntoMemoryNullTerminate(const char* Path)
{
	FILE* File = fopen(Path, "rb");
	fseek(File, 0, SEEK_END);
	size_t FileSize = ftell(File);
	fseek(File, 0, SEEK_SET);
	char* FileContents = (char*)malloc(FileSize + 1);
	fread(FileContents, 1, FileSize, File);
	FileContents[FileSize] = 0;
	return FileContents;
	fclose(File);
}

static bool IsEngineFile(const std::string& file)
{
	return file.find("Eunoia.h") != std::string::npos;
}

static void WriteHeader()
{
	if (IsEngine)
	{
		std::string Text = "#include \"../Eunoia.h\"\n\nnamespace Eunoia {\n";
		fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
	}
	else
	{
		std::string Text = "#include <Eunoia\\Eunoia.h>\n#include \"" + InputFileName + ".h\"";
		fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
	}
}

static void WriteGetTypeIDFunction(const std::string& Name, u32 ID)
{
	std::string IDString;

	if (IsEngine)
		IDString = std::to_string(ID);
	else
		IDString = "Eunoia::Metadata::LastEngineTypeID + " + std::to_string(ID);

	std::string Text;
	if (IsEngine)
		Text = "\n\n\ttemplate<>\n\tEU_API metadata_typeid Metadata::GetTypeID < ";
	else
		Text = "\n\n\ttemplate<>\n\tEU_PROJ metadata_typeid Metadata::GetTypeID < ";

	Text += Name + " > () { return " + IDString + "; }";
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteInitMetadataPrimitivesFunction()
{
	const char* Primitives[] = { "char", "u8","u16","u32","s8","s16","s32","r32","r64","bool" };
	const char* PrimitiveTypes[] = { "METADATA_PRIMITIVE_CHAR", "METADATA_PRIMITIVE_U8","METADATA_PRIMITIVE_U16","METADATA_PRIMITIVE_U32",
	"METADATA_PRIMITIVE_S8","METADATA_PRIMITIVE_S16","METADATA_PRIMITIVE_S32","METADATA_PRIMITIVE_R32","METADATA_PRIMITIVE_R64","METADATA_PRIMITIVE_BOOL" };

	std::string Text = "\n\n\tvoid Metadata::InitMetadataPrimitives()\n\t{\n\t\tMetadataInfo info;";
	for (u32 i = 0; i < 9; i++)
	{
		WriteGetTypeIDFunction(Primitives[i], CurrentID);
		Text += "\n\t\tinfo.id = " + std::to_string(CurrentID) + ";\n\t\tinfo.type = METADATA_PRIMITIVE;\n\t\tinfo.primitive = new MetadataPrimitive();\n\t\tinfo.primitive->name = \"" + Primitives[i] +
			"\";\n\t\tinfo.primitive->size = sizeof( " + Primitives[i] + " );\n\t\tinfo.primitive->type = " + PrimitiveTypes[i] +
			";\n\t\t\Metadata::RegisterMetadataInfo(info);\n";
		CurrentID++;
	}
	Text += "\t}";

	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteLastEngineTypeID(u32 ID)
{
	std::string Text = "\n\tconst metadata_typeid Metadata::LastEngineTypeID = " + std::to_string(ID) + ";";
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteConstructMetadataInfoFuntionClass(const std::string& Name, const std::string& BaseClassName, const std::vector<class_member>& Members, u32 ID, const std::string& IsComponent, const std::string& IsSystem, const std::string& IsEvent)
{
	std::string IDString = std::to_string(ID);
	if (!IsEngine)
		IDString = "Eunoia::Metadata::LastEngineTypeID + " + IDString;

	std::string IsEngineText = (IsEngine ? "true" : "false");

	std::string Text = "\n\n\ttemplate<>\n\tMetadataInfo Metadata::ConstructMetadataInfo<" + Name + ">()\n\t{\n\t\tMetadataInfo info;\n\t\tinfo.id = " + IDString +
		";\n\t\tinfo.type = METADATA_CLASS;\n\t\tinfo.cls = Eunoia::Metadata::AllocateClass( " + IsEngineText + " );\n\t\tinfo.cls->name = \"" + Name + "\";\n\t\tinfo.cls->baseClassName = \"" + BaseClassName + "\";\n\t\tinfo.cls->baseClassSize = " + (BaseClassName.empty() ? "0;" : "sizeof( " + BaseClassName + " );")
		+ "\n\t\tinfo.cls->size = sizeof( " + Name + " );\n\t\tinfo.cls->isComponent = " + IsComponent + ";\n\t\tinfo.cls->isSystem = " + IsSystem + ";\n\t\tinfo.cls->isEvent = " + IsEvent +
		";\n\t\tinfo.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< " + Name + " >;" + "\n\t\tinfo.cls->members.SetCapacityAndElementCount( " + std::to_string(Members.size()) + " );";

	std::string MemberVectorInitializationText = "";
	for (u32 i = 0; i < Members.size(); i++)
	{
		const class_member* Member = &Members[i];
		std::string Index = std::to_string(i);
		std::string MemberInitializationText = "\n\n\t\tinfo.cls->members[ " + Index + " ].name = \"" + Members[i].Name + "\";\n\t\tinfo.cls->members[ " + Index + " ].typeName = \"" + Members[i].TypeName +
			"\";\n\t\tinfo.cls->members[ " + Index + " ].typeID = GetTypeID<" + Members[i].TypeName + ">();\n\t\tinfo.cls->members[ " + Index + " ].accessModifier = " + Members[i].AccessModifer +
			";\n\t\tinfo.cls->members[ " + Index + " ].offset = " + Members[i].Offset + ";\n\t\tinfo.cls->members[ " + Index + " ].size = " + Members[i].Size +
			";\n\t\tinfo.cls->members[ " + Index + " ].isStatic = " + Members[i].IsStatic + ";\n\t\tinfo.cls->members[ " + Index + " ].isConst = " + Members[i].IsConst +
			";\n\t\tinfo.cls->members[ " + Index + " ].isPointer = " + Members[i].IsPointer + ";\n\t\tinfo.cls->members[ " + Index + " ].arrayLength = " + Members[i].ArrayLength +
			";\n\t\tinfo.cls->members[ " + Index + " ].uiSliderMin = " + Members[i].UIMin + ";\n\t\tinfo.cls->members[ " + Index + " ].uiSliderMax = " + Members[i].UIMax +
			";\n\t\tinfo.cls->members[ " + Index + " ].uiSliderSpeed = " + Members[i].UISpeed + ";\n\t\tinfo.cls->members[ " + Index + " ].is32BitBool = " + Members[i].Is32BitBool + ";";

		MemberVectorInitializationText += MemberInitializationText;
	}

	std::string FunctionEndText = "\n\n\t\treturn info;\n\t}";
	Text += (MemberVectorInitializationText + FunctionEndText);
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteConstructMetadataInfoFunctionEnum(const std::string Name, const std::vector<enum_value>& Members, u32 ID)
{
	std::string IDString = std::to_string(ID);
	if (!IsEngine)
		IDString = "Eunoia::Metadata::LastEngineTypeID + " + IDString;

	std::string IsEngineText = (IsEngine ? "true" : "false");

	std::string Text = "\n\n\ttemplate<>\n\tMetadataInfo Metadata::ConstructMetadataInfo< " + Name + " >()\n\t{\n\t\tMetadataInfo info;\n\t\tinfo.id = " + IDString +
		";\n\t\tinfo.type = METADATA_ENUM;\n\t\tinfo.enm = Eunoia::Metadata::AllocateEnum( " + IsEngineText + " );\n\t\tinfo.enm->name = \"" + Name + "\";\n\t\tinfo.enm->values.SetCapacityAndElementCount( " +
		std::to_string(Members.size()) + " );";

	std::string ValueVectorInitializationText = "";
	for (u32 i = 0; i < Members.size(); i++)
	{
		const enum_value* value = &Members[i];
		std::string Index = std::to_string(i);
		std::string ValueInitializationText = "\n\t\tinfo.enm->values[ " + Index + " ].name = \"" + value->Name + "\";\n\t\t\info.enm->values[ " + Index + " ].value = " + value->Value + ";";

		ValueVectorInitializationText += ValueInitializationText;
	}

	std::string FunctionEndText = "\n\n\t\treturn info;\n\t}";
	Text += (ValueVectorInitializationText + FunctionEndText);
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteInitMetadataInfosFunction(std::vector<std::string>& Names)
{
	std::string Text = "\n\n\tvoid Metadata::InitMetadataInfos()\n\t{";
	for (u32 i = 0; i < Names.size(); i++)
		Text += "\n\t\tRegisterMetadataInfo( ConstructMetadataInfo< " + Names[i] + " >() );";

	Text += "\n\t}";
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

static void WriteInitProjectMetadataInfosFunction(std::vector<std::string>& Names)
{
	std::string Text = "\n\n\tvoid " + InputFileName + "::RegisterMetadata()\n\t{";
	for (u32 i = 0; i < Names.size(); i++)
		Text += "\n\t\tMetadata::RegisterMetadataInfo( Metadata::ConstructMetadataInfo< " + Names[i] + " >() );";

	Text += "\n\n\t\tMetadata::LastProjectTypeID = Metadata::LastEngineTypeID + " + std::to_string(CurrentID - 1) + ";\n\t}";
	fwrite(Text.c_str(), 1, Text.length(), GeneratedFile);
}

std::string GetDirectory(const char* FilePath)
{
	std::string FilePathStr = FilePath;
	size_t Index = FilePathStr.find_last_of('/');
	size_t BIndex = FilePathStr.find_last_of('\\');
	if (Index == std::string::npos)
		Index = BIndex;

	if (BIndex != std::string::npos && BIndex > Index)
		Index = BIndex;

	FilePathStr = FilePathStr.substr(0, Index + 1);
	return FilePathStr;
}


/*
TODO: Support a case where an enum value has a comma in it
#define TEST(A, B) A + B
enum Test
{
	TEST1 = TEST(0, 1),
};
*/

static void ParseIntrospectableEnum(tokenizer* Tokenizer)
{
	token Token = GetToken(Tokenizer);
	std::string name(Token.Text, Token.Length);
	Token = GetToken(Tokenizer);
	std::vector<enum_value> Values;
	while (Token.Type != TOKEN_CLOSE_BRACE)
	{
		if (Token.Type == TOKEN_IDENTIFIER)
		{
			enum_value Value;
			Value.Name = std::string(Token.Text, Token.Length);
			Value.Value = Value.Name;
			Values.push_back(Value);
		}
		else if (Token.Type == TOKEN_EQUAL)
		{
			while (Token.Type != TOKEN_COMMA && Token.Type != TOKEN_CLOSE_BRACE)
			{
				Token = GetToken(Tokenizer);
			}
		}

		if(Token.Type != TOKEN_CLOSE_BRACE)
			Token = GetToken(Tokenizer);
	}

	WriteGetTypeIDFunction(name, CurrentID);
	WriteConstructMetadataInfoFunctionEnum(name, Values, CurrentID);
	MetaTypes.push_back(name);
	CurrentID++;
}

static void ParseIntrospectable(tokenizer* Tokenizer)
{
	token Token = GetToken(Tokenizer);
	std::string IsComponent = "false";
	std::string IsSystem = "false";
	std::string IsEvent = "false";
	b32 IsClass;
	if (Token.Type != TOKEN_OPEN_PAREN)
		return;
	Token = GetToken(Tokenizer);
	while (Token.Type != TOKEN_CLOSE_PAREN)
	{
		if (Token.Type == TOKEN_IDENTIFIER)
		{
			if (StringEqual(Token.Text, Token.Length, "Component", 9))
				IsComponent = "true";
			else if (StringEqual(Token.Text, Token.Length, "System", 6))
				IsSystem = "true";
			else if (StringEqual(Token.Text, Token.Length, "Event", 5))
				IsEvent = "true";
		}
		Token = GetToken(Tokenizer);
	}

	Token = GetToken(Tokenizer);
	if (Token.Type == TOKEN_STRUCT)
	{
		IsClass = false;
	}
	else if (Token.Type == TOKEN_CLASS)
	{
		IsClass = true;
	}
	else if (Token.Type == TOKEN_ENUM)
	{
		ParseIntrospectableEnum(Tokenizer);
		return;
	}
	else
		return;

	Token = GetToken(Tokenizer);
	if (Token.Type == TOKEN_IDENTIFIER &&
		(StringEqual(Token.Text, Token.Length, "EU_API", 6) ||
			StringEqual(Token.Text, Token.Length, "EU_PROJ", 7)))
	{
		Token = GetToken(Tokenizer);
	}

	std::string Name(Token.Text, Token.Length);
	std::string BaseClassName = "";

	Token = GetToken(Tokenizer);
	if (Token.Type == TOKEN_COLON)
	{
		Token = GetToken(Tokenizer);
		StorageAccessModifier baseAccessModifer = ACCESS_MODIFIER_PRIVATE;
		if (StringEqual(Token.Text, Token.Length, "public", 6))
		{
			baseAccessModifer = ACCESS_MODIFIER_PUBLIC;
			Token = GetToken(Tokenizer);
		}
		else if (StringEqual(Token.Text, Token.Length, "private", 7))
		{
			baseAccessModifer = ACCESS_MODIFIER_PRIVATE;
			Token = GetToken(Tokenizer);
		}
		else if (StringEqual(Token.Text, Token.Length, "protected", 9))
		{
			baseAccessModifer = ACCESS_MODIFIER_PROTECTED;
			Token = GetToken(Tokenizer);
		}

		BaseClassName = std::string(Token.Text, Token.Length);
		Token = GetToken(Tokenizer);
		if (Token.Type == TOKEN_COLON)
		{
			while (Token.Type != TOKEN_LESS_THAN && Token.Type != TOKEN_OPEN_BRACE)
			{
				if (Token.Type == TOKEN_COLON)
				{
					Token = GetToken(Tokenizer);
					continue;
				}

				BaseClassName = std::string(Token.Text, Token.Length);
				Token = GetToken(Tokenizer);
			}
		}

		if (Token.Type == TOKEN_LESS_THAN)
		{
			while (Token.Type != TOKEN_GREATER_THAN)
			{
				Token = GetToken(Tokenizer);
			}

			Token = GetToken(Tokenizer);
		}

		while (Token.Type != TOKEN_OPEN_BRACE)
			Token = GetToken(Tokenizer);
	}

	std::vector<class_member> Members;
	StorageAccessModifier CurrentAccessModifier = IsClass ? ACCESS_MODIFIER_PRIVATE : ACCESS_MODIFIER_PUBLIC;

	while (true)
	{
		Token = GetToken(Tokenizer);

		if (Token.Type == TOKEN_IDENTIFIER)
		{
			if (StringEqual(Token.Text, Token.Length, "public", 6))
			{
				CurrentAccessModifier = ACCESS_MODIFIER_PUBLIC;
				Token = GetToken(Tokenizer);
			}
			else if (StringEqual(Token.Text, Token.Length, "private", 7))
			{
				CurrentAccessModifier = ACCESS_MODIFIER_PRIVATE;
				Token = GetToken(Tokenizer);
			}
			else if (StringEqual(Token.Text, Token.Length, "protected", 9))
			{
				CurrentAccessModifier = ACCESS_MODIFIER_PROTECTED;
				Token = GetToken(Tokenizer);
			}
			else if (StringEqual(Token.Text, Token.Length, Name.c_str(), Name.length()))
			{
				Token = GetToken(Tokenizer);
				if (Token.Type == TOKEN_OPEN_PAREN)
				{
					u32 openParenCount = 1;
					while (true)
					{
						Token = GetToken(Tokenizer);
						if (Token.Type == TOKEN_OPEN_PAREN)
						{
							openParenCount++;
						}
						else if (Token.Type == TOKEN_CLOSE_PAREN)
						{
							openParenCount--;
							if (openParenCount == 0)
								break;
						}
					}
				}
				else
					continue;

				u32 openBraceCount = 0;
				while (true)
				{
					Token = GetToken(Tokenizer);
					if (Token.Type == TOKEN_SEMICOLON)
					{
						break;
					}
					else if (Token.Type == TOKEN_COLON)
					{
						while (true)
						{
							Token = GetToken(Tokenizer);
							if (Token.Type == TOKEN_OPEN_BRACE)
							{
								openBraceCount++;
								break;
							}
						}
					}
					else if (Token.Type == TOKEN_OPEN_BRACE)
					{
						openBraceCount++;
					}
					else if (Token.Type == TOKEN_CLOSE_BRACE)
					{
						openBraceCount--;
						if (openBraceCount == 0)
							break;
					}
				}
			}
			else
			{
				if (StringEqual(Token.Text, Token.Length, "EU_PROPERTY", 11))
				{
					Token = GetToken(Tokenizer);
					if (Token.Type != TOKEN_OPEN_PAREN)
						continue;

					std::string UIMin = "v4(0.0)";
					std::string UIMax = "v4(0.0)";
					std::string UISpeed = "0.1";
					std::string Is32BitBool = "false";
					while (true)
					{
						Token = GetToken(Tokenizer);
						if (Token.Type == TOKEN_CLOSE_PAREN)
							break;

						if (Token.Type == TOKEN_IDENTIFIER && StringEqual(Token.Text, Token.Length, "UIMin", 5))
						{
							GetToken(Tokenizer);
							Token = GetToken(Tokenizer);
							UIMin = "v4(" + std::string(Token.Text, Token.Length);
							while (Token.Type != TOKEN_CLOSE_PAREN)
							{
								Token = GetToken(Tokenizer);
								if (Token.Type == TOKEN_COMMA)
								{
									Token = GetToken(Tokenizer);
									UIMin += ", " + std::string(Token.Text, Token.Length);
								}
							}
							UIMin += ")";
							GetToken(Tokenizer);
						}
						else if (Token.Type == TOKEN_IDENTIFIER && StringEqual(Token.Text, Token.Length, "UIMax", 5))
						{
							GetToken(Tokenizer);
							Token = GetToken(Tokenizer);
							UIMax = "v4(" + std::string(Token.Text, Token.Length);
							while (Token.Type != TOKEN_CLOSE_PAREN)
							{
								Token = GetToken(Tokenizer);
								if (Token.Type == TOKEN_COMMA)
								{
									Token = GetToken(Tokenizer);
									UIMax += ", " + std::string(Token.Text, Token.Length);
								}
							}
							UIMax += ")";
							GetToken(Tokenizer);
						}
						else if (Token.Type == TOKEN_IDENTIFIER && StringEqual(Token.Text, Token.Length, "UISpeed", 7))
						{
							GetToken(Tokenizer);
							Token = GetToken(Tokenizer);
							UISpeed = std::string(Token.Text, Token.Length);
							GetToken(Tokenizer);
						}
						else if (Token.Type == TOKEN_IDENTIFIER && StringEqual(Token.Text, Token.Length, "Bool32", 6))
						{
							Is32BitBool = "true";
						}
					}

					Token = GetToken(Tokenizer);

					bool IsStatic = false;
					bool IsConst = false;
					if (StringEqual(Token.Text, Token.Length, "static", 6))
					{
						IsStatic = true;
						Token = GetToken(Tokenizer);
						if (Token.Type == TOKEN_IDENTIFIER &&
							StringEqual(Token.Text, Token.Length, "const", 5))
						{
							IsConst = true;
							Token = GetToken(Tokenizer);
						}
					}
					else if (StringEqual(Token.Text, Token.Length, "const", 5))
					{
						IsConst = true;
						Token = GetToken(Tokenizer);
						if (Token.Type == TOKEN_IDENTIFIER &&
							StringEqual(Token.Text, Token.Length, "static", 6))
						{
							IsStatic = true;
							Token = GetToken(Tokenizer);
						}
					}

					bool IsPointer = false;

					std::string MemberTypeName = "";
					std::string MemberName = "";
					std::string ArrayLength = "1";
					u32 openTemplateCount = 0;

					while (Token.Type != TOKEN_SEMICOLON && Token.Type != TOKEN_EQUAL)
					{
						if (Token.Type == TOKEN_LESS_THAN)
						{
							openTemplateCount++;
						}
						else if (Token.Type == TOKEN_GREATER_THAN)
						{
							openTemplateCount--;
						}
						else if (Token.Type == TOKEN_ASTERISK)
						{
							if (openTemplateCount == 0)
							{
								IsPointer = true;
								Token = GetToken(Tokenizer);
								continue;
							}
						}
						else if (Token.Type == TOKEN_OPEN_BRACKET)
						{
							Token = GetToken(Tokenizer);
							std::string ArrayLength = "";
							while (Token.Type != TOKEN_CLOSE_BRACKET)
							{
								ArrayLength += std::string(Token.Text, Token.Length);
								Token = GetToken(Tokenizer);
							}
							Token = GetToken(Tokenizer);
						}
						if (!MemberName.empty())
						{
							if (!MemberTypeName.empty())
								MemberTypeName += "";
							MemberTypeName += MemberName;
						}

						MemberName = std::string(Token.Text, Token.Length);
						Token = GetToken(Tokenizer);
					}

					std::string Offset = "0";
					if (!IsStatic)
						Offset = "offsetof( " + Name + ", " + Name + "::" + MemberName + " )";

					std::string Size = "sizeof( " + MemberTypeName + " )";

					std::string AccessModifierString;
					switch (CurrentAccessModifier)
					{
					case ACCESS_MODIFIER_PUBLIC: AccessModifierString = "METADATA_ACCESS_MODIFIER_PUBLIC"; break;
					case ACCESS_MODIFIER_PRIVATE: AccessModifierString = "METADATA_ACCESS_MODIFIER_PRIVATE"; break;
					case ACCESS_MODIFIER_PROTECTED: AccessModifierString = "METADATA_ACCESS_MODIFIER_PROTECTED"; break;
					}

					std::string IsStaticString = IsStatic ? "true" : "false";
					std::string IsConstString = IsConst ? "true" : "false";
					std::string IsPointerString = IsPointer ? "true" : "false";

					class_member Member;
					Member.Name = MemberName;
					Member.TypeName = MemberTypeName;
					Member.IsStatic = IsStaticString;
					Member.IsConst = IsConstString;
					Member.IsPointer = IsPointerString;
					Member.Offset = CurrentAccessModifier == ACCESS_MODIFIER_PUBLIC ? Offset : "EU_U32_MAX";
					Member.Size = Size;
					Member.ArrayLength = ArrayLength;
					Member.AccessModifer = AccessModifierString;
					Member.UIMin = UIMin;
					Member.UIMax = UIMax;
					Member.UISpeed = UISpeed;
					Member.Is32BitBool = Is32BitBool;
					Members.push_back(Member);
				}
			}
		}
		else if (Token.Type == TOKEN_OPEN_BRACE)
		{
			u32 openBraceCount = 0;
			while (true)
			{
				Token = GetToken(Tokenizer);
				if (Token.Type == TOKEN_OPEN_BRACE)
				{
					openBraceCount++;
				}
				else if (Token.Type == TOKEN_CLOSE_BRACE)
				{
					if (openBraceCount != 0)
						openBraceCount--;
					else
						break;
				}
			}
		}
		//End of class/struct
		else if (Token.Type == TOKEN_CLOSE_BRACE)
		{
			Token = GetToken(Tokenizer);
			break;
		}
	}

	WriteGetTypeIDFunction(Name, CurrentID);
	WriteConstructMetadataInfoFuntionClass(Name, BaseClassName, Members, CurrentID, IsComponent, IsSystem, IsEvent);
	CurrentID++;

	MetaTypes.push_back(Name);
}

static void Parse(char* Contents, const std::string& RootFile)
{
	std::string RootPath = GetDirectory(RootFile.c_str());

	tokenizer Tokenizer = {};
	Tokenizer.At = Contents;

	char IncludePath[512];
	bool Parsing = true;

	ParsedFiles.push_back(std::filesystem::absolute(RootFile).generic_string());

	while (Parsing)
	{
		token Token = GetToken(&Tokenizer);
		//printf("%d: %.*s\n", Token.Type, Token.Length, Token.Text);
		switch (Token.Type)
		{
		case TOKEN_EOS: { Parsing = false; } break;
		case UNKNOWN_TOKEN_TYPE: { continue; } break;
		case TOKEN_POUND: {
			Token = GetToken(&Tokenizer);
			if (Token.Type == TOKEN_IDENTIFIER &&
				StringEqual(Token.Text, Token.Length, "include", 7))
			{
				Token = GetToken(&Tokenizer);
				if (Token.Type == TOKEN_STRING_LITERAL)
				{
					std::string RootDir = RootPath;
					std::string IncludeRelDir(Token.Text, Token.Length);
					if (IncludeRelDir.find("/Vendor") != std::string::npos)
						continue;
					std::string IncludeDir = RootDir + IncludeRelDir;

					std::string AbsolutePath = std::filesystem::absolute(IncludeDir).generic_string();
					bool found = false;
					for (u32 i = 0; i < ParsedFiles.size(); i++)
					{
						if (AbsolutePath == ParsedFiles[i])
						{
							found = true;
							break;
						}
					}

					if (found)
						break;

					char* FileContents = ReadFileIntoMemoryNullTerminate(IncludeDir.c_str());
					Parse(FileContents, IncludeDir);
					free(FileContents);
				}
			}
			else if (Token.Type == TOKEN_IDENTIFIER &&
				StringEqual(Token.Text, Token.Length, "define", 6))
			{
				Token = GetToken(&Tokenizer);
				if (Token.Type == TOKEN_IDENTIFIER &&
					StringEqual(Token.Text, Token.Length, "EU_REFLECT", 10))
				{
					Token = GetToken(&Tokenizer);
					break;
				}
			}
		}break;
		case TOKEN_IDENTIFIER: {
			if (StringEqual(Token.Text, Token.Length, "EU_REFLECT", 10))
			{
				ParseIntrospectable(&Tokenizer);
			}
		} break;
		}
	}
}

int main(int argc, char** argv)
{
	CurrentID = 0;
	const char* SrcFile = argv[1];
	const char* DstFile = argv[2];

	InputFileName = SrcFile;
	int Index = InputFileName.find_last_of('/') + 1;
	InputFileName = InputFileName.substr(Index, InputFileName.find_last_of('.') - Index);

	IsEngine = IsEngineFile(SrcFile);

	if (!IsEngine)
		CurrentID = 1;

	char* ParseCode = ReadFileIntoMemoryNullTerminate(SrcFile);

	GeneratedFile = fopen(DstFile, "w");
	WriteHeader();

	if (IsEngine)
	{
		WriteInitMetadataPrimitivesFunction();
	}

	Parse(ParseCode, SrcFile);

	if (IsEngine)
	{
		WriteLastEngineTypeID(CurrentID - 1);
		WriteInitMetadataInfosFunction(MetaTypes);
		fwrite("\n\n}", 1, 3, GeneratedFile);
	}
	else
	{
		WriteInitProjectMetadataInfosFunction(MetaTypes);
	}
}