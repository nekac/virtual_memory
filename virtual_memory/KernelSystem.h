#ifndef  _KERNELSYSTEM_H_
#define  _KERNELSYSTEM_H_

#include "part.h"
#include "PMTdata.h"
#include "System.h"

class KernelSystem
{
public:
	KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace,  PageNum pmtSpaceSize,  Partition* partition);
	~KernelSystem();
	Process* createProcess();
	Time periodicJob();
	Status access(ProcessId pid, VirtualAddress address, AccessType type);

private:
	PhysicalAddress m_processVMSpace;
	PageNum m_processVMSpaceSize;
	PhysicalAddress m_pmtSpace;
	PageNum m_pmtSpaceSize;
	Partition* m_partition;

	FrameEntry *m_frameEntry;

	ClusterNo m_numOfClusters;
	size_t m_numOfBitVectors;
	unsigned *m_bitVector;
	ProcessEntry *m_processEntry;

	int m_firstFreePageDescriptorStorage; // prvi slobodan u prostoru
	PMTEntry *m_pmtEntry;

	ProcessId m_nextProcessId;
};

#endif
