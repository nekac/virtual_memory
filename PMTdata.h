#ifndef  _PMTDATA_H_
#define  _PMTDATA_H_

// File: PMTdata.h 
#include "vm_declarations.h"
#include "part.h"

#define MAX_NUM_OF_SEG_IN_PROC 100 // uvedeno maksimalan broj segmenata u procesu 
#define NO_NEXT_SEG -1

struct SegmentEntry;

struct PMTEntry {
	int m_valid; // da li se logicka stranica nalazi u memoriji
	int m_dirty; // da li je page koji je u frame-u koriscen vec
	PageNum m_frame; // broj frame-a u nizu koji je u "velikoj" memoriji
	int m_isOnDisk; // da li je trazeni frame uopste u adresnom prostoru diska (da li je na disku)
	ClusterNo m_locationOnDisk; // u kom klasteru je na disku
	SegmentEntry *m_segmentEntry; // segment kome taj page pripada
};

struct SegmentEntry {
	int m_right; // prava pristupa segmentu
	PMTEntry *m_pmtEntry; // pokazivac na niz page-ova tog segmenta
	ProcessId m_processId; // kom procesu pripada segment
	bool isUsed; // koriscenost tog segmenta 
	VirtualAddress m_startAddress; // adresa odakle pocinje niz page-ova
	PageNum m_numOfPages; // broj page-ova u segmentu
};

struct ProcessEntry {
	SegmentEntry m_SegmentInProcess[MAX_NUM_OF_SEG_IN_PROC]; // niz segmenata procesa
};

struct FrameEntry {
	PMTEntry *m_pmtEntry; // page koji je ucitan u dati frame
};

struct PageDescriptorStorage { // opis elementa liste u "maloj" memoriji
	size_t m_entrySize;
	PageDescriptorStorage* m_next;
};

#endif