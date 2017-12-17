#ifndef  _PMTDATA_H_
#define  _PMTDATA_H_

// File: PMTdata.h 
#include "vm_declarations.h"

#define MAX_NUM_OF_SEG_IN_PROC 100 // uvedeno maksimalan broj segmenata u procesu 
#define NO_NEXT_SEG -1

struct SegmentEntry;

struct PMTEntry {
	int m_valid;
	int m_dirty;
	PageNum m_frame;
	int m_isOnDisk;
	ClusterNo m_locationOnDisk;
	SegmentEntry *m_segmentEntry;
	// ostale informacije
};

struct FrameEntry {
	PMTEntry *m_pmtEntry;
	// ostale informacije
};

struct SegmentEntry {
	AccessRight m_right;
	PMTEntry *m_pmtEntry;
	ProcessId m_processId;
	bool isUsed;
	VirtualAddress m_startAddress;
	PageNum m_numOfPages;
};

struct ProcessEntry {
	SegmentEntry m_SegmentInProcess[MAX_NUM_OF_SEG_IN_PROC];
};

struct PageDescriptorStorage {
	size_t m_entrySize;
	PageDescriptorStorage* m_next;
	
};

#endif