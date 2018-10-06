#pragma once

struct StaticFunctionTag;
class VirtualMachine;


namespace PapyrusATInstanceData
{
	bool RegisterPapyrus(VirtualMachine *vm);
	void RegisterFuncs(VirtualMachine* vm);
}

