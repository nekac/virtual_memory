#ifndef  _PROCESS_H_
#define  _PROCESS_H_

// File: Process.h 
#include "vm_declarations.h" 

class KernelProcess;
class System;
class KernelSystem;

// apstrakcija jednog procesa; ovo je samo interfejs ka KernelProcess
class Process {
private:
	KernelProcess *pProcess;
	friend class System;
	friend class KernelSystem;

// resenje obezbedjuje podrsku za logicke segmente koje proces moze da alocira i dealocira
// segmenti su poravnati na pocetak stranice (stranice u segmentu imaju ista prava pristupa)
// korisnik sve operacije vrsi na nivou logickih segmenata (segment sadrzi vise stranica)
public:
	Process(ProcessId pid); // kreiranje procesa
	~Process();
	ProcessId getProcessId() const;
	
	Status createSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags); // kreira segment
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize, AccessRight flags, void* content); // ucitava sadrzaj u segment
	Status deleteSegment(VirtualAddress startAddress); // uklanja segment u virtuelnom adresnom prostoru
	Status pageFault(VirtualAddress address); // obrada stranicne greske
	
	PhysicalAddress getPhysicalAddress(VirtualAddress address); // preslikavanje virtuelne u fizicku adresu
};

#endif

