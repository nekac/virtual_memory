#include "part.h"
#include <iostream>
#include "vm_declarations.h"
#include "System.h"
#include "Preocess.h"

int main()
{
	Partition* part = new Partition("p1.ini");
	std::cout << part->getNumOfClusters();

	auto pages = new char[4 * PAGE_SIZE];
	auto pmtSpace = new char[300 * PAGE_SIZE];

	auto system = new System(pages, 10000, pmtSpace, 300, part);

	Process* p1 = system->createProcess();
	Process* p2 = system->createProcess();
	Process* p3 = system->createProcess();
	Process* p4 = system->createProcess();

	p1->createSegment(0x1000, 4, WRITE);
	p1->createSegment(0x5000, 4, READ);

	p2->createSegment(0x1000, 4, WRITE);
	p2->createSegment(0x5000, 4, READ);
					
	p3->createSegment(0x1000, 4, WRITE);
	p3->createSegment(0x5000, 4, READ);
					
	p4->createSegment(0x1000, 4, WRITE);
	p4->createSegment(0x5000, 4, READ);

	delete p1;
	delete p2;
	delete p3;
	delete p4;

	delete system;

	delete part;
}
