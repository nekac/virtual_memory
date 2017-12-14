#include "KernelSystem.h"
#include "part.h"
#include "PMTdata.h"
#include "Preocess.h"
#include "KernelProcess.h"
#include <intrin.h>
#include <string.h>

// processVMSpace - pokazivac na pocetak prostora u memoriji za smestanje stranica procesa
// processVMSpaceSize - velicina tog prostora za stranice
// pmtSpace - pokazivac na pocetak prostora za smestanje tabela preslikavanja
// prtSpaceSize - velicina tog prostora za tabele
// partition - pokazivac na particiju koja sluzi za zamenu

KernelSystem::KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition){
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

	m_numOfBitVectors = m_numOfClusters / (sizeof(unsigned long) * 8); // num of unsigned words (4B words)
	m_numOfBitVectors += (m_numOfBitVectors % (sizeof(unsigned long) * 8)) ? 1 : 0;
	
	m_bitVector = (unsigned long*)(m_frameEntry + processVMSpaceSize);

	for(int i=0; i < m_numOfBitVectors; i++){
		m_bitVector[i] = 0;
	}

	// prostor za ulaze svakog od procesa

	size_t totalPMT_size_afterBitvectorAlloc = totalPMT_size_afterFrameAlloc - m_numOfBitVectors * sizeof(unsigned long);

	// prostor za ulaze u PMT

	m_storageEmptySpace = (PageDescriptorStorage*)(m_bitVector + m_numOfBitVectors);

	((PageDescriptorStorage*)(m_storageEmptySpace))->m_next = nullptr;
	((PageDescriptorStorage*)(m_storageEmptySpace))->m_entrySize = totalPMT_size_afterBitvectorAlloc;

	nextFrame = 0;

}

KernelSystem::~KernelSystem(){

}

Process* KernelSystem::createProcess(){
	Process *newProcess = new Process(++m_nextProcessId);
	newProcess->pProcess->init(this);
	return newProcess;
}

Time KernelSystem::periodicJob(){
	return 0;
}

// TODO!
Status KernelSystem::access(ProcessId pid, VirtualAddress address, AccessType type){
	return OK;
}

FrameEntry * KernelSystem::getNextFrame(int &frameNumber){
	frameNumber = nextFrame;
	FrameEntry* result = m_frameEntry + nextFrame;

	if(result->m_pmtEntry != nullptr && result->m_pmtEntry->m_valid && result->m_pmtEntry->m_dirty){
		ClusterNo freeCluster = getFirstEmptyCluster();
		if(freeCluster >= m_partition->getNumOfClusters()){
			return nullptr;
		}
		m_partition->writeCluster(freeCluster, (char*)(m_processVMSpace) + nextFrame*PAGE_SIZE);
		setClusterUsed(freeCluster);
		result->m_pmtEntry->m_dirty = false;
		result->m_pmtEntry->m_isOnDisk = true;
	}

	result->m_pmtEntry->m_valid = false;
	nextFrame = (nextFrame + 1) % m_processVMSpaceSize;
	return result;
}

ClusterNo KernelSystem::getFirstEmptyCluster(){
	unsigned long index;
	unsigned char isNonzero;
	ClusterNo result = 0;

	for (unsigned i = 0; i < m_numOfBitVectors; i++){
		unsigned long mask = *(reinterpret_cast<unsigned*>(m_bitVector) + i);
		isNonzero = _BitScanForward(&index, mask);
		if (isNonzero){
			result = i * sizeof(unsigned) * 4 + index;
			break;
		}
	}
	
	return result;

}

void KernelSystem::populateFrame(int frameNumber, void * data){
	memcpy(((char*)(m_processVMSpace)+nextFrame*frameNumber), data, PAGE_SIZE);
}

void KernelSystem::setClusterNotUsed(ClusterNo cluster_no){
	size_t wordIndex = cluster_no / (sizeof(unsigned long) * 8);
	size_t bitIndex = cluster_no - wordIndex * sizeof(unsigned long) * 8;

	reinterpret_cast<unsigned long*>(m_bitVector)[wordIndex] &= ~(1 << bitIndex);

}

void KernelSystem::setClusterUsed(ClusterNo cluster_no){
	size_t wordIndex = cluster_no / (sizeof(unsigned long) * 8);
	size_t bitIndex = cluster_no - wordIndex * sizeof(unsigned long) * 8;

	reinterpret_cast<unsigned long*>(m_bitVector)[wordIndex] |= 1 << bitIndex;
}

void* KernelSystem::AllocateSpace(size_t pmtEntrySize){
	PageDescriptorStorage* prev = nullptr;
	PageDescriptorStorage* curr = m_storageEmptySpace;

	pmtEntrySize = pmtEntrySize + sizeof(PageDescriptorStorage);

	while (curr!=nullptr && curr->m_entrySize<pmtEntrySize)
	{
		prev = curr;
		curr = curr->m_next;
	}

	if (curr == nullptr) return nullptr; // ako nema dovoljno prostora

	if(curr->m_entrySize > (pmtEntrySize + sizeof(PageDescriptorStorage))){
		PageDescriptorStorage* newSpace = (PageDescriptorStorage*)(((char*)curr) + pmtEntrySize); // char zbog velicine od 1B
		if(prev){
			prev->m_next = newSpace;
		} else {
			m_storageEmptySpace = newSpace;
		}

		newSpace->m_next = curr->m_next;
		newSpace->m_entrySize = curr->m_entrySize - pmtEntrySize;
		curr->m_entrySize = pmtEntrySize;
	} else {
		if (prev) {
			prev->m_next = curr->m_next;
		}
		else {
			m_storageEmptySpace = curr->m_next;
		}
	}

	return curr + 1;

}

bool KernelSystem::canBeMerged(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* curr){
	return curr == (PageDescriptorStorage*)((char*)spaceToReturn + spaceToReturn->m_entrySize);
}

void KernelSystem::findToMerge(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* &prev, PageDescriptorStorage* &curr){
	prev = nullptr;
	curr = m_storageEmptySpace;

	while (curr != nullptr && !(canBeMerged(spaceToReturn, curr) || canBeMerged(curr, spaceToReturn)))
	{
		prev = curr;
		curr = curr->m_next;
	}
}

void KernelSystem::DealocateSpace(void* storageEmptySpace){
	PageDescriptorStorage* spaceToReturn = ((PageDescriptorStorage*)storageEmptySpace)-1;

	PageDescriptorStorage* prev = nullptr;
	PageDescriptorStorage* curr = nullptr;

	findToMerge(spaceToReturn, prev, curr);

	while (curr != nullptr){
		if(prev){
			prev->m_next = curr->m_next;
		} else {
			m_storageEmptySpace = curr->m_next;
		}

		size_t newSize = curr->m_entrySize + spaceToReturn->m_entrySize;

		if(spaceToReturn>curr){
			spaceToReturn = curr;
		}

		spaceToReturn->m_entrySize = newSize;

		findToMerge(spaceToReturn, prev, curr);
	}

	spaceToReturn->m_next = m_storageEmptySpace;
	m_storageEmptySpace = spaceToReturn;
	
}
