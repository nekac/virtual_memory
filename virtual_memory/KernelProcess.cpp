#include "KernelProcess.h"


KernelProcess::KernelProcess(ProcessId pid){
	m_pid = pid;
}


KernelProcess::~KernelProcess()
{

}

ProcessId KernelProcess::getProcessId() const
{
	return ProcessId();
}

Status KernelProcess::createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags)
{
	return OK;
}

Status KernelProcess::loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void * content)
{
	return OK;
}

Status KernelProcess::deleteSegment(VirtualAddress startAddress)
{
	return OK;
}

Status KernelProcess::pageFault(VirtualAddress address)
{
	return OK;
}

PhysicalAddress KernelProcess::getPhysicalAddress(VirtualAddress address)
{
	return PhysicalAddress();
}

void KernelProcess::init(KernelSystem* kernelSystem){
	m_kernelSystem = kernelSystem;
}

