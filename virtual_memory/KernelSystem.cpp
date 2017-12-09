#include "KernelSystem.h"
#include "part.h"
#include "PMTdata.h"
#include "Preocess.h"
#include "KernelProcess.h"

// processVMSpace - pokazivac na pocetak prostora u memoriji za smestanje stranica procesa
// processVMSpaceSize - velicina tog prostora za stranice
// pmtSpace - pokazivac na pocetak prostora za smestanje tabela preslikavanja
// prtSpaceSize - velicina tog prostora za tabele
// partition - pokazivac na particiju koja sluzi za zamenu

KernelSystem::KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition) {
	m_processVMSpace = processVMSpace;
	m_processVMSpaceSize = processVMSpaceSize;
	m_pmtSpace = pmtSpace;
	m_pmtSpaceSize = pmtSpaceSize;
	m_partition = partition;

	m_nextProcessId = 0;

	// prostor za stranice svakog od procesa

	m_frameEntry = (FrameEntry*) pmtSpace;
	for(PageNum i = 0; i<processVMSpaceSize; i++){
		m_frameEntry[i].m_pmtEntry = nullptr;
	}

	size_t totalPMT_size = pmtSpaceSize * PAGE_SIZE;
	size_t totalPMT_size_afterFrameAlloc = totalPMT_size - sizeof(FrameEntry) * processVMSpaceSize;

	// prostor za bit vektore (izracunavanje)

	m_numOfClusters = partition->getNumOfClusters();

	m_numOfBitVectors = m_numOfClusters / (sizeof(unsigned) * 8); // num of unsigned words (4B words)
	m_numOfBitVectors += (m_numOfBitVectors % (sizeof(unsigned) * 8)) ? 1 : 0;
	
	m_bitVector = (unsigned*)(m_frameEntry + processVMSpaceSize);

	for(int i=0; i < m_numOfBitVectors; i++){
		m_bitVector[i] = 0;
	}

	// prostor za ulaze svakog od procesa

	size_t totalPMT_size_afterBitvectorAlloc = totalPMT_size_afterFrameAlloc - m_numOfBitVectors * sizeof(unsigned);

	m_processEntry = (ProcessEntry*)(m_bitVector + m_numOfBitVectors);

	for (int i=0; i<MAX_NUM_OF_PROC; i++){
		m_processEntry[i].m_isUsed = false;
	}

	// prostor za ulaze u PMT

	size_t totalPMT_size_afterProcessAlloc = totalPMT_size_afterBitvectorAlloc - MAX_NUM_OF_PROC * sizeof(PMTEntry);

	m_pmtEntry = (PMTEntry*)(m_processEntry + MAX_NUM_OF_PROC);
	m_firstFreePageDescriptorStorage = 0;

	((PageDescriptorStorage*)(m_pmtEntry))->m_next = NO_NEXT_SEG;
	((PageDescriptorStorage*)(m_pmtEntry))->m_entrySize = totalPMT_size_afterProcessAlloc / sizeof(PMTEntry);

}

KernelSystem::~KernelSystem()
{

}

Process* KernelSystem::createProcess()
{
	Process *newProcess = new Process(++m_nextProcessId);
	newProcess->pProcess->init(this);
	return nullptr;
}

Time KernelSystem::periodicJob()
{
	return 0;
}

Status KernelSystem::access(ProcessId pid, VirtualAddress address, AccessType type)
{
	return OK;
}
