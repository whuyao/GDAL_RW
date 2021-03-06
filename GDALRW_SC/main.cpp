#include <iostream>
#include <gdal_priv.h>
#include <fstream>
#include <omp.h>
#include "GDALRead.h"
#include "GDALWrite.h"
using namespace std;

int main(int argc, char *argv[])
{
	if (argc<3)
	{
		cout<<"please input I/O filename. exit."<<endl;
		return -1;
	}

	//register
	GDALAllRegister();
	//OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	//test_data
	CGDALRead* pread = new CGDALRead;
	if (!pread->loadFrom(argv[1]))
	{
		cout<<"load error!"<<endl;
	}
	
	cout<<"load success!"<<endl;

	int cols = pread->cols();
	int rows = pread->rows();
	double geoTransform[6];
	double* pgeo = pread->geotransform();
	for (int kk=0; kk<6; kk++)
	{
		geoTransform[kk] = pgeo[kk];
	}

	char projref[2048];
	strcpy(projref, pread->projectionRef());

	//
	cout<<"samples = "<<pread->cols()<<endl;
	cout<<"lines = "<<pread->rows()<<endl;
	cout<<pread->bandnum()<<endl;
	cout<<pread->datalength()<<endl;
	cout<<pread->projectionRef()<<endl;

	//
	unsigned char* pdata = new unsigned char[pread->rows()*pread->cols()];

	int i, j, k;
	
//#pragma omp parallel for private(j), num_threads(omp_get_max_threads())
	for (i=0; i<pread->rows(); i++)
	{
		for (j=0; j<pread->cols(); j++)
		{
			pdata[0*pread->rows()*pread->cols()+i*pread->cols()+j] = *(unsigned char*)pread->read(i, j, 0);
		}
	}

	cout<<"read success!"<<endl;

	pread->close();
	delete pread;

	
//
	CGDALWrite pwrite;
	bool brlt = pwrite.init(argv[2], rows, cols, 1, \
		geoTransform, projref, GDT_Float32, 0);
	if (!brlt)
	{
		cout<<"write init error!"<<endl;
		return -2;
	}

	float _val = 0;

//#pragma omp parallel for private(j, k, _val), num_threads(omp_get_max_threads())
	for (i=0; i<pwrite.rows(); i++)
	{
		for (j=0; j<pwrite.cols(); j++)
		{
			for (k=0; k<1; k++)
			{
				 _val = (float)pdata[k*pwrite.rows()*pwrite.cols()+i*pwrite.cols()+j]/2.0;
				 pwrite.write(i, j, k, &_val);
			}
		}
	}

	cout<<"write success!"<<endl;

	pwrite.close();

	delete []pdata;

	return 0;
}