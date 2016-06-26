#include "Y2KExporter.h"
#include "xnacollision.h"

#define IN_PATH "M3dFiles\\"
#define OUT_PATH "C:\\Users\\Administrator\\Source\\Repos\\ZollJak-Test\\Scatus Defence\\Scatus Defence\\Models\\"
int main()
{
	std::string inFileName;
	std::string outFileName;
	Y2KExporter exporter;

	inFileName = "cyclop.y2k";
	outFileName = "cyclop.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, true);
	exporter.ExportY2K(OUT_PATH + outFileName);

	return 0;
}