#include "Preocess.h"
#include "KernelProcess.h"


Process::Process(ProcessId pid)
{
	KernelProcess* p = new KernelProcess(pid);
}

Process::Process()
{
}

ProcessId Process::getProcessId() const
{
	return ProcessId();
}

Status Process::createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags)
{
	return OK;
}

Status Process::loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void* content)
{
	return OK;
}

Status Process::deleteSegment(VirtualAddress startAddress)
{
	return OK;
}

Status Process::pageFault(VirtualAddress address)
{
	return OK;
}

PhysicalAddress Process::getPhysicalAddress(VirtualAddress address)
{
	return nullptr;
}
