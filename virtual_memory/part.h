#ifndef  _PART_H_
#define  _PART_H_

// File: part.h 
typedef unsigned long ClusterNo;
const unsigned long ClusterSize = 1024;

class PartitionImpl;

class Partition {
private:
	PartitionImpl *myImpl;

public:
	Partition(const char *); // kreiranje objekta particije, parametar je naziv konfiguracionog fajla koji sadrzi informacije
	virtual ClusterNo getNumOfClusters() const; // vraca broj klastera koji pripadaju particiji

	virtual int readCluster(ClusterNo, char *buffer); // cita zadati klaster sa diska i u slucaju uspeha vraca 1; u suprotnom 0
	virtual int writeCluster(ClusterNo, const char *buffer); // upisuje zadati klaster iz memorije na disk i u slucaju uspeha vraca 1; u suprotnom 0

	virtual ~Partition();
};

#endif