#include "KernelProcess.h"
#include "Process.h"
#include "KernelSystem.h"

KernelProcess::KernelProcess(ProcessId pid) {
	m_pid = pid;
}

KernelProcess::~KernelProcess() {
	deInit();
}

ProcessId KernelProcess::getProcessId() const {
	return m_pid;
}

Status KernelProcess::createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessType flags) {
	SegmentEntry* emptySegment = nullptr;
	return createSegmentHelp(startAddress, segmentSize, flags, emptySegment);
}

Status KernelProcess::loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessType flags, void * content) {
	SegmentEntry* emptySegment = nullptr;
	Status status = createSegmentHelp(startAddress, segmentSize, flags, emptySegment);
	
	if (status != OK) {
		return status;
	}

	// u memoriju se ucitava kompletan segment i popunjava se sadrzaj frame-ova
	for (int i = 0; i < segmentSize; i++) {
		PageNum frameNum;
		FrameEntry* frame = m_kernelSystem->getNextFrame(frameNum);
		if (frame == nullptr) {
			return TRAP;
		}
		frame->m_pmtEntry = (emptySegment->m_pmtEntry + i);
		m_kernelSystem->populateFrame(frameNum, (char*)content + i * PAGE_SIZE);
		emptySegment->m_pmtEntry[i].m_isOnDisk = false;
		emptySegment->m_pmtEntry[i].m_valid = true;
		emptySegment->m_pmtEntry[i].m_dirty = true;
		emptySegment->m_pmtEntry[i].m_frame = frameNum;
		emptySegment->m_pmtEntry[i].m_segmentEntry = emptySegment;
	}

	return OK;
}

Status KernelProcess::createSegmentHelp(VirtualAddress startAddress, PageNum segmentSize, AccessType flags, SegmentEntry*& emptySegment) {
	emptySegment = nullptr;
	
	for (int i = 0; i < MAX_NUM_OF_SEG_IN_PROC; i++) {
		if (!m_processEntry->m_SegmentInProcess[i].isUsed) {
			emptySegment = m_processEntry->m_SegmentInProcess + i;
			break;
		}
	}

	if (emptySegment == nullptr) {
		return TRAP;
	}

	VirtualAddress endAddress = startAddress + segmentSize * PAGE_SIZE;

	for (int i = 0; i < MAX_NUM_OF_SEG_IN_PROC; i++) { // da li se segmenti preklapaju medjusobno, adrese
		if (m_processEntry->m_SegmentInProcess[i].isUsed) {
			VirtualAddress iStartAddress = m_processEntry->m_SegmentInProcess[i].m_startAddress;
			VirtualAddress iEndAddress = iStartAddress + m_processEntry->m_SegmentInProcess[i].m_numOfPages * PAGE_SIZE;
			if (startAddress <= iStartAddress && endAddress >= iEndAddress || iStartAddress <= startAddress && iEndAddress >= endAddress) {
				return TRAP;
			}
		}
	}

	// inicijalizacija novog segmenta, uz setovanje svih polja strukture segmenata
	emptySegment->m_pmtEntry = (PMTEntry*)m_kernelSystem->AllocateSpace(segmentSize * PAGE_SIZE);
	
	if (emptySegment->m_pmtEntry == nullptr) {
		return TRAP;
	}

	emptySegment->isUsed = true;
	emptySegment->m_startAddress = startAddress;
	emptySegment->m_numOfPages = segmentSize;

	if (flags == READ_WRITE) {
		emptySegment->m_right = READ | WRITE | READ_WRITE; // 0111, odnosno 7
	}
	else {
		emptySegment->m_right = flags; // 1000, odnosno 8
	}

	// u segmentu setovanje page-ova
	for (int i = 0; i < segmentSize; i++) { // segmentSize je u broju page-ova
		emptySegment->m_pmtEntry[i].m_isOnDisk = false;
		emptySegment->m_pmtEntry[i].m_valid = false;
	}

	return OK;
}

Status KernelProcess::deleteSegment(VirtualAddress startAddress) {
	SegmentEntry* segment = findSegmentByVirtualAddress(startAddress);

	if (segment == nullptr) {
		return TRAP;
	}

	if (segment->m_startAddress != startAddress) {
		return TRAP;
	}

	for (int i = 0; i < segment->m_numOfPages; i++) {
		
		if (segment->m_pmtEntry[i].m_isOnDisk) { // na disku je page koji pripada segmentu koji se brise
			m_kernelSystem->setClusterNotUsed(segment->m_pmtEntry[i].m_locationOnDisk);
		}
		
		if (segment->m_pmtEntry[i].m_valid) {
			m_kernelSystem->m_frameEntry[segment->m_pmtEntry[i].m_frame].m_pmtEntry = nullptr;
		}
	}

	m_kernelSystem->DealocateSpace(segment->m_pmtEntry);
	segment->isUsed = false;
	return OK;
}

SegmentEntry * KernelProcess::findSegmentByVirtualAddress(VirtualAddress address) {
	for (int i = 0; i<MAX_NUM_OF_SEG_IN_PROC; i++) { // prolazi kroz sve segmente i trazi onaj kome data adresa pripada
		if (m_processEntry->m_SegmentInProcess[i].isUsed
			&&
			m_processEntry->m_SegmentInProcess[i].m_startAddress <= address
			&&
			m_processEntry->m_SegmentInProcess[i].m_startAddress + m_processEntry->m_SegmentInProcess[i].m_numOfPages * PAGE_SIZE > address
			) {
			return m_processEntry->m_SegmentInProcess + i;
		}
	}
	return nullptr;
}



Status KernelProcess::pageFault(VirtualAddress address) {
	SegmentEntry* segment = findSegmentByVirtualAddress(address);
	if (segment == nullptr) {
		return TRAP;
	}
	VirtualAddress offsetInSegment = address - segment->m_startAddress;
	VirtualAddress pageNum = offsetInSegment / PAGE_SIZE; // broj stranice u okviru segmenta gde je adresa koja se trazi

	PMTEntry& pmt = segment->m_pmtEntry[pageNum]; // stranica za izbacivanje

	if (pmt.m_valid) {
		return OK; // nije ni trebao da bude page fault
	}

	PageNum frameNum;
	FrameEntry* frame = m_kernelSystem->getNextFrame(frameNum);

	if (frame == nullptr) {
		return TRAP;
	}
	frame->m_pmtEntry = &pmt;

	if (pmt.m_isOnDisk) {
		m_kernelSystem->readFromHardDrive(pmt.m_locationOnDisk, m_kernelSystem->getFrameAddress(frameNum));
	}

	pmt.m_dirty = pmt.m_isOnDisk;
	pmt.m_isOnDisk = false;
	pmt.m_valid = true;
	pmt.m_frame = frameNum;

	return OK;
}

PhysicalAddress KernelProcess::getPhysicalAddress(VirtualAddress address) {
	SegmentEntry* segment = findSegmentByVirtualAddress(address); // nadje se segment na osnovu adrese
	if (segment == nullptr) {
		return 0;
	}
	VirtualAddress offsetInSegment = address - segment->m_startAddress;
	VirtualAddress pageNum = offsetInSegment / PAGE_SIZE; // nadje se page na osnovu offset-a
	VirtualAddress offsetInPage = offsetInSegment % PAGE_SIZE;

	PageNum frameOfPhAddr = segment->m_pmtEntry[pageNum].m_frame; // na osnovu pozicije page-a nadje se frame
	PhysicalAddress physicalAddress = (char*)(m_kernelSystem->getFrameAddress(frameOfPhAddr)) + offsetInPage; // odredjivanje fizicke adrese odakle se cita

	return physicalAddress; // konkretna adresa, odatle se cita
}

void KernelProcess::init(KernelSystem* kernelSystem) {
	m_kernelSystem = kernelSystem;
	m_processEntry = (ProcessEntry*)(m_kernelSystem->AllocateSpace(sizeof(ProcessEntry)));

	// sa sve segmente koji se inicijalno kreiraju setuje da se ne koriste i kom procesu pripadaju
	for (int i = 0; i < MAX_NUM_OF_SEG_IN_PROC; i++) {
		m_processEntry->m_SegmentInProcess[i].isUsed = false;
		m_processEntry->m_SegmentInProcess[i].m_processId = m_pid;
	}
}

void KernelProcess::deInit() {
	for (int i = 0; i < MAX_NUM_OF_SEG_IN_PROC; i++) {
		if (m_processEntry->m_SegmentInProcess[i].isUsed) {
			deleteSegment(m_processEntry->m_SegmentInProcess[i].m_startAddress);
		}
	}

	m_kernelSystem->DealocateSpace(m_processEntry);
}


