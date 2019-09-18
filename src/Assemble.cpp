// ======================= ZoneTool =======================
// zonetool, a fastfile linker for various
// Call of Duty titles. 
//
// Project: https://github.com/ZoneTool/gsc-asm
// Author: RektInator (https://github.com/RektInator)
// License: GNU GPL v3.0
// ========================================================
#include "stdafx.hpp"

class Assembler
{
private:
#pragma pack(push, 1)
	struct FuncDef
	{
		std::uint32_t size;
		std::uint16_t funcid;
	};
#pragma pack(pop)

	std::vector < std::uint8_t > m_script;
	std::vector < std::uint8_t > m_stack;

	std::size_t m_scriptpos;
	std::size_t	m_stackpos;

	std::string m_name;

	FuncDef* m_curfunc;
	std::uint16_t m_funcid;

public:
	Assembler(std::string name)
	{
		// Reserve data in vector
		m_script.resize(0x100000);
		m_stack.resize(0x100000);

		// we need to clear the vectors
		std::fill(m_script.begin(), m_script.end(), 0);
		std::fill(m_stack.begin(), m_stack.end(), 0);

		// Reset vars
		m_scriptpos = 0;
		m_stackpos = 0;
		m_name = name;
		m_curfunc = nullptr;
		m_funcid = 1650;				// this seems to be used by multiple default scriptfiles, why shouldn't we?
	}

	void InsertFunction()
	{
		FuncDef temp = { 0, 0 };

		// alloc functiondef on the stack
		m_curfunc = WriteStack(temp);
		m_curfunc->funcid = m_funcid;

		// increment funcid for next function
		m_funcid++;
	}

	template <typename T> T* WriteScript(T data)
	{
		T* mem = reinterpret_cast<T*>(m_script.data() + m_scriptpos);
		memcpy(mem, &data, sizeof(T));

		m_scriptpos += sizeof(T);

		// Increment script function size when function is present, and only when we're writing into the script data.
		// Stack data does not count for the function size.
		if (m_curfunc)
		{
			m_curfunc->size += sizeof(T);
		}

		return mem;
	}
	template <typename T> T* WriteStack(T data)
	{
		T* mem = reinterpret_cast<T*>(m_stack.data() + m_stackpos);
		memcpy(mem, &data, sizeof(T));

		m_stackpos += sizeof(T);

		return mem;
	}
	void WriteString(std::string str)
	{
		strcpy(reinterpret_cast<char*>(m_stack.data() + m_stackpos), str.data());
		m_stackpos += str.size() + 1;
	}

	void Save()
	{
		std::string stackfile = m_name + ".cgsc.stack";
		std::string scriptfile = m_name + ".cgsc";

		FILE* fp = fopen(stackfile.data(), "wb");
		fwrite(m_stack.data(), 1, m_stackpos, fp);
		fclose(fp);

		fp = fopen(scriptfile.data(), "wb");
		fwrite(m_script.data(), 1, m_scriptpos, fp);
		fclose(fp);
	}
};

std::vector < std::string > split(std::string &str, char delimiter)
{
	std::vector<std::string> internal;
	std::stringstream ss(str);
	std::string tok;

	while (std::getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}

	return internal;
}

std::unordered_map < std::string, std::function<void(Assembler&, std::vector<std::string>)> > m_assemble;

std::string ReadFile(std::string file)
{
	std::string buffer;

	std::ifstream stream(file, std::ios::binary);
	if (stream.good())
	{
		if (!stream.is_open()) return buffer;

		stream.seekg(0, std::ios::end);
		std::streamsize size = stream.tellg();
		stream.seekg(0, std::ios::beg);

		if (size > -1)
		{
			buffer.clear();
			buffer.resize(static_cast<uint32_t>(size));

			stream.read(const_cast<char*>(buffer.data()), size);
		}

		stream.close();
	}

	return buffer;
}

std::string ToLower(std::string input)
{
	std::string output(input.begin(), input.end());

	for (std::size_t i = 0; i < output.size(); i++)
	{
		output[i] = tolower(input[i]);
	}

	return output;
}

void Assemble(std::string scriptname)
{
	std::string scriptfile = ReadFile(scriptname);

	std::size_t pos;
	while ((pos = scriptfile.find("\t")) != std::string::npos)
	{
		scriptfile = scriptfile.replace(pos, 1, "");
	}
	while ((pos = scriptfile.find("\r")) != std::string::npos)
	{
		scriptfile = scriptfile.replace(pos, 1, "");
	}
	
	Assembler script(scriptname);
	std::vector < std::string > assembly = split(scriptfile, '\n');

	// Scriptfiles always start with the END opcode.
	script.WriteScript<std::uint8_t>(OP_End);

	for (auto &opcode : assembly)
	{
		std::vector < std::string > opcodeargs;
		if (opcode.find(' ') != std::string::npos)
		{
			opcodeargs = split(ToLower(opcode), ' ');
		}
		else
		{
			opcodeargs.push_back(ToLower(opcode));
		}

		if (opcodeargs[0].substr(0, 3) == "fn:")
		{
			// Do this after the OP_End, first opcode does not count as instruction.
			script.InsertFunction();

			// This is not an actual opcode, go to next iteration.
			continue;
		}

		// Do not parse comment lines
		if (opcodeargs[0][0] == '#')
		{
#ifdef _DEBUG
			printf("[DEBUG]: Skipping current line, line starts with a comment.\n");
#endif
			continue;
		}

		if (m_assemble.find(opcodeargs[0]) != m_assemble.end())
		{

			m_assemble[opcodeargs[0]](script, opcodeargs);
		}
		else
		{
			printf("Unknown opcode \"%s\".\n", opcodeargs[0].data());
		}
	}

	script.Save();
}

extern std::unordered_map < std::uint16_t, std::string > builtinmap;
extern std::unordered_map < std::uint16_t, std::string > builtinmethodmap;

std::uint16_t FindFunctionHandle(std::vector < std::string > args)
{
	std::string name = args[1];

	for (auto &func : builtinmap)
	{
		if (ToLower(func.second) == ToLower(name))
		{
			return func.first;
		}
	}

	printf("[ERROR]: Couldn't resolve function \"%s\"!", name.data());
	return 0xFFFF;
}
std::uint16_t FindMethodHandle(std::vector < std::string > args)
{
	std::string name = args[1];

	for (auto &func : builtinmethodmap)
	{
		if (ToLower(func.second) == ToLower(name))
		{
			return func.first;
		}
	}

	printf("[ERROR]: Couldn't resolve method \"%s\"!", name.data());
	return 0xFFFF;
}

void RegisterOpcode(std::string name, std::function<void(Assembler& script, std::vector<std::string> args)> func)
{
	name = ToLower(name);
	m_assemble[name] = func;
}

#define SIMPLE_OP(__OPCODE) \
	RegisterOpcode(#__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
	});

#define OP_1ARG_SCRIPT(__OPCODE,__OPCODE_STR,__OPCODE_SCRIPT_TYPE,__OPCODE_SCRIPT_VALUE) \
	RegisterOpcode(__OPCODE_STR, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(__OPCODE_SCRIPT_VALUE); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(__OPCODE_SCRIPT_VALUE); \
	});

#define OP_1ARG_SCRIPT_CB(__OPCODE,__OPCODE_STR,__OPCODE_SCRIPT_TYPE,__OPCODE_SCRIPT_VALUE) \
	RegisterOpcode(__OPCODE_STR, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(__OPCODE_SCRIPT_VALUE(args)); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(__OPCODE_SCRIPT_VALUE(args)); \
	});

#define OP_1ARG_SCRIPT_INT(__OPCODE,__OPCODE_SCRIPT_TYPE) \
	RegisterOpcode(#__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(std::stoi(args[1])); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<__OPCODE_SCRIPT_TYPE>(std::stoi(args[1])); \
	});

#define OP_1ARG_SCRIPT_FLOAT(__OPCODE) \
	RegisterOpcode(#__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<float>(std::stof(args[1])); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteScript<float>(std::stof(args[1])); \
	});

#define OP_1ARG_STACK_INT(__OPCODE,__OPCODE_SCRIPT_TYPE) \
	RegisterOpcode(#__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteStack<__OPCODE_SCRIPT_TYPE>(std::stoi(args[1])); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteStack<__OPCODE_SCRIPT_TYPE>(std::stoi(args[1])); \
	});

#define OP_1ARG_STACK_FLOAT(__OPCODE) \
	RegisterOpcode(#__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteStack<float>(std::stof(args[1])); \
	}); \
	RegisterOpcode("OP_" #__OPCODE, [](Assembler& script, std::vector<std::string> args) \
	{ \
		script.WriteScript<std::uint8_t>(OP_##__OPCODE); \
		script.WriteStack<float>(std::stof(args[1])); \
	});

void Assembler_Init()
{
	// 1 byte opcodes
	SIMPLE_OP(End);
	SIMPLE_OP(Return);
	SIMPLE_OP(vector);
	SIMPLE_OP(EvalArray);
	SIMPLE_OP(CastFieldObject);
	SIMPLE_OP(size);
	SIMPLE_OP(AddArray);
	SIMPLE_OP(EvalArrayRef);
	SIMPLE_OP(voidCodepos);
	SIMPLE_OP(PreScriptCall);
	SIMPLE_OP(notify);
	SIMPLE_OP(endon);
	SIMPLE_OP(GetUndefined);
	SIMPLE_OP(GetZero);
	SIMPLE_OP(EvalLocalVariableCached0);
	SIMPLE_OP(EvalLocalVariableCached1);
	SIMPLE_OP(EvalLocalVariableCached2);
	SIMPLE_OP(EvalLocalVariableCached3);
	SIMPLE_OP(EvalLocalVariableCached4);
	SIMPLE_OP(EvalLocalVariableCached5);
	SIMPLE_OP(EvalNewLocalArrayRefCached0);
	SIMPLE_OP(ClearArray);
	SIMPLE_OP(EmptyArray);
	SIMPLE_OP(ScriptFunctionCallPointer);
	SIMPLE_OP(ScriptMethodCallPointer);
	SIMPLE_OP(GetSelf);
	SIMPLE_OP(GetLevel);
	SIMPLE_OP(GetGame);
	SIMPLE_OP(GetGameRef);
	SIMPLE_OP(inc);
	SIMPLE_OP(dec);
	SIMPLE_OP(bit_or);
	SIMPLE_OP(bit_ex_or);
	SIMPLE_OP(bit_and);
	SIMPLE_OP(equality);
	SIMPLE_OP(inequality);
	SIMPLE_OP(less);
	SIMPLE_OP(greater);
	SIMPLE_OP(less_equal);
	SIMPLE_OP(waittill);
	SIMPLE_OP(greater_equal);
	SIMPLE_OP(plus);
	SIMPLE_OP(minus);
	SIMPLE_OP(multiply);
	SIMPLE_OP(divide);
	SIMPLE_OP(GetSelfObject);
	SIMPLE_OP(GetAnimTree);
	SIMPLE_OP(EvalLocalVariableRefCached0);
	SIMPLE_OP(SetVariableField);
	SIMPLE_OP(SetLocalVariableFieldCached0);
	SIMPLE_OP(ClearLocalVariableFieldCached0);
	SIMPLE_OP(clearparams);
	SIMPLE_OP(checkclearparams);
	SIMPLE_OP(wait);
	SIMPLE_OP(DecTop);
	SIMPLE_OP(CastBool);
	SIMPLE_OP(BoolNot);
	SIMPLE_OP(BoolComplement);

	// String opcodes
	RegisterOpcode("GetString", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetString);
		script.WriteScript<std::uint16_t>(0);				// Placeholder
		script.WriteString(args[1]);
	});
	RegisterOpcode("OP_GetString", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetString);
		script.WriteScript<std::uint16_t>(0);				// Placeholder
		script.WriteString(args[1]);
	});
	RegisterOpcode("GetAnimation", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetAnimation);
		script.WriteString(args[1]);
		script.WriteString(args[2]);
		script.WriteScript<std::uint8_t>(0);				// Placeholder
	});
	RegisterOpcode("OP_GetAnimation", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetAnimation);
		script.WriteString(args[1]);
		script.WriteString(args[2]);
		script.WriteScript<std::uint8_t>(0);				// Placeholder
	});

	// Vector opcodes
	RegisterOpcode("GetVector", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetVector);
		script.WriteScript<float>(std::stof(args[1]));
		script.WriteScript<float>(std::stof(args[2]));
		script.WriteScript<float>(std::stof(args[3]));
	});
	RegisterOpcode("OP_GetVector", [](Assembler& script, std::vector<std::string> args)
	{
		script.WriteScript<std::uint8_t>(OP_GetVector);
		script.WriteScript<float>(std::stof(args[1]));
		script.WriteScript<float>(std::stof(args[2]));
		script.WriteScript<float>(std::stof(args[3]));
	});

	// Builtin calls
	OP_1ARG_SCRIPT_CB(CallBuiltin0, "Call<0>", std::uint16_t, FindFunctionHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltin1, "Call<1>", std::uint16_t, FindFunctionHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltin2, "Call<2>", std::uint16_t, FindFunctionHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltin3, "Call<3>", std::uint16_t, FindFunctionHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltin4, "Call<4>", std::uint16_t, FindFunctionHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltin5, "Call<5>", std::uint16_t, FindFunctionHandle);

	// Builtin methods
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod0, "Method<0>", std::uint16_t, FindMethodHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod1, "Method<1>", std::uint16_t, FindMethodHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod2, "Method<2>", std::uint16_t, FindMethodHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod3, "Method<3>", std::uint16_t, FindMethodHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod4, "Method<4>", std::uint16_t, FindMethodHandle);
	OP_1ARG_SCRIPT_CB(CallBuiltinMethod5, "Method<5>", std::uint16_t, FindMethodHandle);

	// 2 byte script opcodes
	OP_1ARG_SCRIPT_INT(GetByte, std::int8_t);
	OP_1ARG_SCRIPT_INT(GetNegByte, std::int8_t);
	OP_1ARG_SCRIPT_INT(CreateLocalVariable, std::int8_t);
	OP_1ARG_SCRIPT_INT(RemoveLocalVariables, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalVariableCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalArrayCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalArrayRefCached0, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalArrayRefCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(ScriptThreadCallPointer, std::int8_t);
	OP_1ARG_SCRIPT_INT(ScriptMethodThreadCallPointer, std::int8_t);
	OP_1ARG_SCRIPT_INT(ScriptMethodChildThreadCallPointer, std::int8_t);
	OP_1ARG_SCRIPT_INT(CallBuiltinPointer, std::int8_t);
	OP_1ARG_SCRIPT_INT(CallBuiltinMethodPointer, std::int8_t);
	OP_1ARG_SCRIPT_INT(GetAnimObject, std::int8_t);
	OP_1ARG_SCRIPT_INT(SafeCreateVariableFieldCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(SafeSetVariableFieldCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(SafeSetWaittillVariableFieldCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalVariableRefCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(SetNewLocalVariableFieldCached0, std::int8_t);
	OP_1ARG_SCRIPT_INT(SetLocalVariableFieldCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(ClearLocalVariableFieldCached, std::int8_t);
	OP_1ARG_SCRIPT_INT(EvalLocalVariableObjectCached, std::int8_t);

	// 3 byte script opcodes
	OP_1ARG_SCRIPT_INT(JumpOnFalseExpr, std::int16_t);
	OP_1ARG_SCRIPT_INT(GetUnsignedShort, std::int16_t);
	OP_1ARG_SCRIPT_INT(GetNegUnsignedShort, std::int16_t);
	OP_1ARG_SCRIPT_INT(GetBuiltinFunction, std::int16_t);
	OP_1ARG_SCRIPT_INT(GetBuiltinMethod, std::int16_t);
	// OP_1ARG_SCRIPT_INT(GetString, std::int16_t);
	OP_1ARG_SCRIPT_INT(JumpOnTrueExpr, std::int16_t);
	OP_1ARG_SCRIPT_INT(jumpback, std::int16_t);
	OP_1ARG_SCRIPT_INT(JumpOnFalse, std::int16_t);
	OP_1ARG_SCRIPT_INT(JumpOnTrue, std::int16_t);
	OP_1ARG_SCRIPT_INT(waittillmatch, std::int16_t);

	// 4 byte script opcodes (need to look into those first)

	// 5 byte script opcodes
	OP_1ARG_SCRIPT_INT(GetInteger, std::uint32_t);
	OP_1ARG_SCRIPT_FLOAT(GetFloat);
}
