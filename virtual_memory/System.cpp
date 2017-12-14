#include "System.h"
#include "KernelSystem.h"

// sve metode su interfejs ka onima iz KernelSystem klase
System::System(PhysicalAddress processVMSpace, PageNum processVMSpaceSize, PhysicalAddress pmtSpace, PageNum pmtSpaceSize, Partition * partition){
	pSystem = new KernelSystem(processVMSpace, processVMSpaceSize, pmtSpace, pmtSpaceSize, partition);
}

System::~System(){
	delete pSystem;
}

Process * System::createProcess(){
	return pSystem->createProcess();
}

Time System::periodicJob(){
	return pSystem->periodicJob();
}

Status System::access(ProcessId pid, VirtualAddress address, AccessType type){
	return pSystem->access(pid, address, type);
}
