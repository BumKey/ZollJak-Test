#include "Y2KExporter.h"
#include "xnacollision.h"

#define IN_PATH "C:\\Users\\Administrator\\Desktop\\"
#define OUT_PATH "C:\\Users\\Administrator\\Source\\Repos\\ZollJak-Test\\Scatus Defence\\Scatus Defence\\Models\\"

int main()
{
	std::string inFileName;
	std::string outFileName;
	Y2KExporter exporter;

	inFileName = "sample.m3d";
	outFileName = "sample.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	return 0;
}