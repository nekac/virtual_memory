#ifndef  _SYSTEM_H_
#define  _SYSTEM_H_

// File: System.h 
#include "vm_declarations.h" 

class Partition;
class Process;
class KernelProcess;
class KernelSystem;

// klasa koja rukuje prostorom u memoriji; ovo je interfejs ka KernelSystem
class System {
private:
	KernelSystem *pSystem; // implementacija sistema
	friend class Process;
	friend class KernelProcess;

public:
	// processVMSpace - pokazivac na pocetak prostora u memoriji za smestanje stranica procesa
	// processVMSpaceSize - velicina tog prostora za stranice
	// pmtSpace - pokazivac na pocetak prostora za smestanje tabela preslikavanja
	// pmtSpaceSize - velicina tog prostora za tabele
	// partition - pokazivac na particiju koja sluzi za zamenu
	System(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition* partition);
	~System();

	Process* createProcess();
	Time periodicJob(); // kroz FIFO implementaciju nije bitna, ne poziva se

	// simulacija hardverskog posla
	// proces koji pristupa adresi; kojoj virtuelnoj adresi se pristupa; koji je tip pristupa
	Status access(ProcessId pid, VirtualAddress address, AccessType type);

};

#endif

