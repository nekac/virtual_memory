#ifndef  _KERNELSYSTEM_H_
#define  _KERNELSYSTEM_H_

// File: KernelSystem.h 
#include "part.h"
#include "PMTdata.h"
#include "System.h"

class KernelSystem
{
private:
	
	friend KernelProcess;

	PhysicalAddress m_processVMSpace;
	PageNum m_processVMSpaceSize;

	PhysicalAddress m_pmtSpace;
	PageNum m_pmtSpaceSize;

	Partition* m_partition;

	FrameEntry *m_frameEntry;

	ClusterNo m_numOfClusters;
	size_t m_numOfBitVectors;
	unsigned long *m_bitVector;

	
	PageDescriptorStorage *m_storageEmptySpace; // prvi slobodan u prostoru

	ProcessId m_nextProcessId;

	int nextFrame;

public:
	KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace,  PageNum pmtSpaceSize,  Partition* partition);
	~KernelSystem();

	Process* createProcess();
	Time periodicJob();
	Status access(ProcessId pid, VirtualAddress address, AccessType type);
	FrameEntry* getNextFrame(int& frameNumber);
	ClusterNo getFirstEmptyCluster();
	void populateFrame(int frameNumber, void* data);
	void setClusterUsed(ClusterNo num);
	void setClusterNotUsed(ClusterNo num);

	void* AllocateSpace(size_t pmtEntryNum);
	bool canBeMerged(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* curr);
	void findToMerge(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* &prev, PageDescriptorStorage* &curr);
	void DealocateSpace(void* storageEmptySpace);

};

#endif
