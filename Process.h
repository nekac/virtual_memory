#pragma once

// File: Process.h 
#include "vm_declarations.h" 

class KernelProcess;
class System;
class KernelSystem;

// apstrakcija jednog procesa; ovo je samo interfejs ka KernelProcess
class Process {

	KernelProcess *pProcess;
	friend class System;
	friend class KernelSystem;

// resenje obezbedjuje podrsku za logicke segmente koje proces moze da alocira i dealocira
// segmenti su poravnati na pocetak stranice (stranice u segmentu imaju ista prava pristupa)
// korisnik sve operacije vrsi na nivou logickih segmenata (segment sadrzi vise stranica)
public:
	Process(ProcessId pid);
	~Process();
	ProcessId getProcessId() const;

	Status createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessType flags); // kreira segment
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessType flags, void* content); // ucitava sadrzaj u segment
	Status deleteSegment(VirtualAddress startAddress); // uklanja segment u virtuelnom adresnom prostoru
	Status pageFault(VirtualAddress address); // obrada stranicne greske, kada nije pronadjena adresa
	
	PhysicalAddress getPhysicalAddress(VirtualAddress address); // preslikavanje virtuelne u fizicku adresu
};

