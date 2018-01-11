#include "KernelSystem.h"
#include "part.h"
#include "PMTdata.h"
#include "Process.h"
#include "KernelProcess.h"
#include <intrin.h>
#include <string.h>

// processVMSpace - pokazivac na pocetak prostora u memoriji za smestanje stranica procesa
// processVMSpaceSize - velicina tog prostora za stranice
// pmtSpace - pokazivac na pocetak prostora za smestanje tabela preslikavanja
// pmtSpaceSize - velicina tog prostora za tabele
// partition - pokazivac na particiju koja sluzi za zamenu

KernelSystem::KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition) {
	m_processVMSpace = processVMSpace;
	m_processVMSpaceSize = processVMSpaceSize;
	m_pmtSpace = pmtSpace;
	m_pmtSpaceSize = pmtSpaceSize;
	m_partition = partition;

	m_nextProcessId = 0;

	// *** PROSTOR ZA DESKRIPTORE SVAKOG OD FRAME-OVA ***

	m_frameEntry = (FrameEntry*)pmtSpace;

	for (PageNum i = 0; i < processVMSpaceSize; i++) {
		m_frameEntry[i].m_pmtEntry = nullptr;
	}

	size_t totalPMT_size = pmtSpaceSize * PAGE_SIZE; // ceo "mali" prostor koji je dodeljen, mnozi se jer je u page-ovima izrazen
	size_t totalPMT_size_afterFrameAlloc = totalPMT_size - sizeof(FrameEntry) * processVMSpaceSize;

	// *** PROSTOR ZA BIT VEKTOR ***

	m_numOfClusters = partition->getNumOfClusters();
	m_numOfBitVectors = m_numOfClusters / (sizeof(unsigned long) * 8); // svaki klaster je predstavljen jednim bitom u okviru vektora
	m_numOfBitVectors += (m_numOfBitVectors % (sizeof(unsigned long) * 8)) ? 1 : 0;
	m_bitVector = (unsigned long*)(m_frameEntry + processVMSpaceSize); // pomeren pokazivac na prostor ispod frame prostora

	for (int i = 0; i < m_numOfBitVectors; i++) {
		m_bitVector[i] = 0;
	}

	size_t totalPMT_size_afterBitvectorAlloc = totalPMT_size_afterFrameAlloc - m_numOfBitVectors * sizeof(unsigned long);

	// *** OSTATAK PROSTORA ZA DESKRIPTORE PROCESA I PMTENTRY-E ***

	m_storageEmptySpace = (PageDescriptorStorage*)(m_bitVector + m_numOfBitVectors);
	((PageDescriptorStorage*)(m_storageEmptySpace))->m_next = nullptr;
	((PageDescriptorStorage*)(m_storageEmptySpace))->m_entrySize = totalPMT_size_afterBitvectorAlloc;
	m_nextFrame = 0;

}

KernelSystem::~KernelSystem() {

}

Process* KernelSystem::createProcess() {
	Process *newProcess = new Process(++m_nextProcessId);
	newProcess->pProcess->init(this);
	m_allProc[newProcess->getProcessId()] = newProcess->pProcess; // popunjava mapu procesa koji su trenutno u sistemu aktivi

	return newProcess;
}

Time KernelSystem::periodicJob() {
	return 0;
}

Status KernelSystem::access(ProcessId pid, VirtualAddress address, AccessType type) {
	auto it = m_allProc.find(pid);
	if (it == m_allProc.end()) {
		return TRAP;
	}

	SegmentEntry* segment = (*it).second->findSegmentByVirtualAddress(address);

	if (!(segment->m_right & type)) {
		return TRAP;
	}

	VirtualAddress offsetInSegment = address - segment->m_startAddress;
	VirtualAddress pageNum = offsetInSegment / PAGE_SIZE;

	if (!(segment->m_pmtEntry[pageNum].m_valid)) {
		return PAGE_FAULT;
	}

	segment->m_pmtEntry[pageNum].m_dirty = segment->m_pmtEntry[pageNum].m_dirty || type == READ_WRITE || type == WRITE;

	return OK;
}

void* KernelSystem::AllocateSpace(size_t size) {
	PageDescriptorStorage* prev = nullptr;
	PageDescriptorStorage* curr = m_storageEmptySpace;

	size = size + sizeof(PageDescriptorStorage); // velicina jednog ProcessEntry-a + onaj mali opisni deo

	while (curr != nullptr && curr->m_entrySize < size) { // ako prvi ne odgovara po velicini za alokaciju ide kroz listu i trazi
		prev = curr;
		curr = curr->m_next;
	}

	if (curr == nullptr) return nullptr; // ako nema dovoljno prostora za zauzimanje, iz prethodne while petlje je to dobijeno

	if (curr->m_entrySize > (size + sizeof(PageDescriptorStorage))) {
		PageDescriptorStorage* newSpace = (PageDescriptorStorage*)(((char*)curr) + size); // char zbog velicine od 1B, lakse se krecemo kroz podatke

		if (prev) {
			prev->m_next = newSpace;
		}
		else {
			m_storageEmptySpace = newSpace;
		}

		newSpace->m_next = curr->m_next;
		newSpace->m_entrySize = curr->m_entrySize - size;
		curr->m_entrySize = size;
	}
	else {
		if (prev) {
			prev->m_next = curr->m_next;
		}
		else {
			m_storageEmptySpace = curr->m_next;
		}
	}

	return curr + 1; // pomeri da tacno pokaze na sledeci slobodan
}

void KernelSystem::populateFrame(int frameNumber, void * data) {
	memcpy(static_cast<char*>(m_processVMSpace) + PAGE_SIZE*frameNumber, data, PAGE_SIZE);
}

void KernelSystem::DealocateSpace(void* storageEmptySpace) {
	PageDescriptorStorage* spaceToReturn = ((PageDescriptorStorage*)storageEmptySpace) - 1; // pomeri na kraj onog iznad
	PageDescriptorStorage* prev = nullptr;
	PageDescriptorStorage* curr = nullptr;

	findToMerge(spaceToReturn, prev, curr); // ako je prilikom izbacivanja moguce prelancavanje

	while (curr != nullptr) {
		if (prev) {
			prev->m_next = curr->m_next;
		}
		else {
			m_storageEmptySpace = curr->m_next;
		}

		size_t newSize = curr->m_entrySize + spaceToReturn->m_entrySize;

		if (spaceToReturn > curr) {
			spaceToReturn = curr;
		}

		spaceToReturn->m_entrySize = newSize;

		findToMerge(spaceToReturn, prev, curr);
	}

	spaceToReturn->m_next = m_storageEmptySpace;
	m_storageEmptySpace = spaceToReturn; // novi poslednji u listi slobodan prostor

}

void KernelSystem::findToMerge(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* &prev, PageDescriptorStorage* &curr) {
	prev = nullptr;
	curr = m_storageEmptySpace;

	while (curr != nullptr && !(canBeMerged(spaceToReturn, curr) || canBeMerged(curr, spaceToReturn))) {
		prev = curr;
		curr = curr->m_next;
	}
}

bool KernelSystem::canBeMerged(PageDescriptorStorage* spaceToReturn, PageDescriptorStorage* curr) {
	return curr == (PageDescriptorStorage*)((char*)spaceToReturn + spaceToReturn->m_entrySize);
}

FrameEntry* KernelSystem::getNextFrame(PageNum &frameNumber) {
	frameNumber = m_nextFrame;
	FrameEntry* result = m_frameEntry + m_nextFrame;

	if (result->m_pmtEntry != nullptr && result->m_pmtEntry->m_valid && result->m_pmtEntry->m_dirty) {
		ClusterNo freeCluster = getFirstEmptyCluster();

		if (freeCluster >= m_partition->getNumOfClusters()) {
			return nullptr;
		}

		m_partition->writeCluster(freeCluster, (char*)(m_processVMSpace)+m_nextFrame*PAGE_SIZE);
		setClusterUsed(freeCluster);
		result->m_pmtEntry->m_dirty = false;
		result->m_pmtEntry->m_isOnDisk = true;
	}

	if (result->m_pmtEntry != nullptr && result->m_pmtEntry->m_valid) {
		result->m_pmtEntry->m_valid = false;
	}

	m_nextFrame = (m_nextFrame + 1) % m_processVMSpaceSize; // FIFO algoritam trazenja free frame-a

	return result;
}

ClusterNo KernelSystem::getFirstEmptyCluster() {
	unsigned long index;
	unsigned char isNonzero;
	ClusterNo result = 0;

	for (unsigned i = 0; i < m_numOfBitVectors; i++) {
		unsigned long mask = *(reinterpret_cast<unsigned*>(m_bitVector) + i);
		isNonzero = _BitScanForward(&index, mask);
		if (isNonzero) {
			result = i * sizeof(unsigned) * 4 + index;
			break;
		}
	}

	return result;
}


void KernelSystem::setClusterNotUsed(ClusterNo cluster_no) {
	size_t wordIndex = cluster_no / (sizeof(unsigned long) * 8);
	size_t bitIndex = cluster_no - wordIndex * sizeof(unsigned long) * 8;
	reinterpret_cast<unsigned long*>(m_bitVector)[wordIndex] &= ~(1 << bitIndex);
}

void KernelSystem::setClusterUsed(ClusterNo cluster_no) {
	size_t wordIndex = cluster_no / (sizeof(unsigned long) * 8);
	size_t bitIndex = cluster_no - wordIndex * sizeof(unsigned long) * 8;
	reinterpret_cast<unsigned long*>(m_bitVector)[wordIndex] |= 1 << bitIndex;
}

PhysicalAddress KernelSystem::getFrameAddress(PageNum index) {
	return (char*)m_processVMSpace + index * PAGE_SIZE;
}

void KernelSystem::readFromHardDrive(ClusterNo cluster_no, PhysicalAddress location) {
	m_partition->readCluster(cluster_no, (char*)location);
	setClusterNotUsed(cluster_no);
}
