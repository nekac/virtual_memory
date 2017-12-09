#ifndef  _PMTDATA_H_
#define  _PMTDATA_H_

// File: PMTdata.h 
#include "vm_declarations.h"

#define MAX_NUM_OF_SEG_IN_PROC 100
#define MAX_NUM_OF_PROC 100 // mora da se promeni!!!
#define NO_NEXT_SEG -1

struct SegmentEntry;

struct PMTEntry {
	int m_valid;
	int m_dirty;
	int m_frame;

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

};

struct ProcessEntry {
	bool m_isUsed;
	SegmentEntry m_SegmentInProcess[MAX_NUM_OF_SEG_IN_PROC];

};

struct PageDescriptorStorage {
	size_t m_entrySize;
	int m_next;
	
};

#endif



