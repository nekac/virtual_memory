#ifndef  _KERNELPROCESS_H_
#define  _KERNELPROCESS_H_

// File: KernelProcess.h 
#include "vm_declarations.h"

class KernelSystem;
struct ProcessEntry;
struct SegmentEntry;

class KernelProcess
{
private:
	ProcessId m_pid;
	KernelSystem* m_kernelSystem;
	ProcessEntry* m_processEntry;

public:
	// metode kao u Process klasi, ovde se implementiraju
	KernelProcess(ProcessId pid);
	~KernelProcess();
	ProcessId getProcessId() const;
	Status createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags);
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void* content);
	Status deleteSegment(VirtualAddress startAddress);
	Status pageFault(VirtualAddress address);
	PhysicalAddress getPhysicalAddress(VirtualAddress address);

	// dodatne metode
	Status createSegmentHelp(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, SegmentEntry*& emptySegment);
	SegmentEntry* findSegmentByVirtualAddress(VirtualAddress address);
	void init(KernelSystem*);
	void deInit();

};

#endif


