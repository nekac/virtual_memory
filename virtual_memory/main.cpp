//#include "part.h"
//#include <iostream>
//#include "vm_declarations.h"
//#include "System.h"
//#include "Preocess.h"
//
//int main()
//{
//	Partition* part = new Partition("p1.ini");
//	std::cout << part->getNumOfClusters();
//
//	auto pages = new char[4 * PAGE_SIZE];
//	auto pmtSpace = new char[300 * PAGE_SIZE];
//
//	auto system = new System(pages, 10000, pmtSpace, 300, part);
//
//	Process* p1 = system->createProcess();
//	Process* p2 = system->createProcess();
//	Process* p3 = system->createProcess();
//	Process* p4 = system->createProcess();
//
//	p1->createSegment(0x1000, 4, WRITE);
//	p1->createSegment(0x5000, 4, READ);
//
//	p2->createSegment(0x1000, 4, WRITE);
//	p2->createSegment(0x5000, 4, READ);
//					
//	p3->createSegment(0x1000, 4, WRITE);
//	p3->createSegment(0x5000, 4, READ);
//					
//	p4->createSegment(0x1000, 4, WRITE);
//	p4->createSegment(0x5000, 4, READ);
//
//	delete p1;
//	delete p2;
//	delete p3;
//	delete p4;
//
//	delete system;
//
//	delete part;
//}
//
#include <chrono>
#include <cstdint>
#include <memory>
#include <iostream>
#include <thread>
#include "System.h"
#include "part.h"
#include "vm_declarations.h"
#include "ProcessTest.h"
#include "SystemTest.h"

#define VM_SPACE_SIZE (10000)
#define PMT_SPACE_SIZE (30000)
#define N_PROCESS (20)
#define PERIODIC_JOB_COST (1)

PhysicalAddress alignPointer(PhysicalAddress address) {
	uint64_t addr = reinterpret_cast<uint64_t> (address);

	addr += PAGE_SIZE;
	addr = addr / PAGE_SIZE * PAGE_SIZE;

	return reinterpret_cast<PhysicalAddress> (addr);
}

int main() {
	Partition part("p1.ini");

	uint64_t size = (VM_SPACE_SIZE + 2) * PAGE_SIZE;
	PhysicalAddress vmSpace = (PhysicalAddress) new char[size];
	PhysicalAddress alignedVmSpace = alignPointer(vmSpace);

	size = (PMT_SPACE_SIZE + 2) * PAGE_SIZE;
	PhysicalAddress pmtSpace = (PhysicalAddress) new char[size];
	PhysicalAddress alignedPmtSpace = alignPointer(pmtSpace);

	System system(alignedVmSpace, VM_SPACE_SIZE, alignedPmtSpace, PMT_SPACE_SIZE, &part);
	SystemTest systemTest(system, alignedVmSpace, VM_SPACE_SIZE);
	ProcessTest* process[N_PROCESS];
	std::thread *threads[N_PROCESS];

	std::mutex globalMutex;

	for (int i = 0; i < N_PROCESS; i++) {
		process[i] = new ProcessTest(system, systemTest);
	}

	for (int i = 0; i < N_PROCESS; i++) {
		std::cout << "Create process " << i << std::endl;
		threads[i] = new std::thread(&ProcessTest::run, process[i]);
	}

	Time time;
	while ((time = system.periodicJob())) {
		std::this_thread::sleep_for(std::chrono::microseconds(time));

		std::lock_guard<std::mutex> guard(systemTest.getGlobalMutex());

		std::cout << "Doing periodic job\n";

		std::this_thread::sleep_for(std::chrono::microseconds(PERIODIC_JOB_COST));

		bool finished = true;
		for (int i = 0; i < N_PROCESS; i++) {
			finished = finished && process[i]->isFinished();
		}

		if (finished) {
			break;
		}
	}

	for (int i = 0; i < N_PROCESS; i++) {
		threads[i]->join();
		delete threads[i];
		delete process[i];
	}

	delete[] vmSpace;
	delete[] pmtSpace;

	std::cout << "Test finished\n";
}