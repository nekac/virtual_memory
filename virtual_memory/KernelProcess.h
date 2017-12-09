#ifndef  _KERNELPROCESS_H_
#define  _KERNELPROCESS_H_

// File: KernelProcess.h 
#include "Preocess.h"
#include "KernelSystem.h"

class KernelProcess
{
private:
	ProcessId m_pid;
	KernelSystem* m_kernelSystem;

public:
	KernelProcess(ProcessId pid);
	~KernelProcess();

	ProcessId getProcessId() const;

	Status createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags);
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void* content);
	Status deleteSegment(VirtualAddress startAddress); 
	Status pageFault(VirtualAddress address); 

	PhysicalAddress getPhysicalAddress(VirtualAddress address);

	void init(KernelSystem*);

};


#endif


