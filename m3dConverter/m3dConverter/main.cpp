#include "Y2KExporter.h"
#include "xnacollision.h"

#define IN_PATH "M3dFiles\\"
#define OUT_PATH "C:\\Users\\Administrator\\Source\\Repos\\ZollJak-Test\\Scatus Defence\\Scatus Defence\\Models\\"
int main()
{
	std::string inFileName;
	std::string outFileName;
	Y2KExporter exporter;
	
	inFileName = "base.m3d";
	outFileName = "base.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "goblin.y2k";
	outFileName = "goblin.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, true);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "cyclop.y2k";
	outFileName = "cyclop.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, true);
	exporter.ExportY2K(OUT_PATH + outFileName);


	inFileName = "house1.y2k";
	outFileName = "house1.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "pillar1.m3d";
	outFileName = "pillar1.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "pillar2.m3d";
	outFileName = "pillar2.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "pillar5.m3d";
	outFileName = "pillar5.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "pillar6.m3d";
	outFileName = "pillar6.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "rock.m3d";
	outFileName = "rock.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "stairs.m3d";
	outFileName = "stairs.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	inFileName = "tree.m3d";
	outFileName = "tree.y2k";
	exporter.LoadM3D(IN_PATH + inFileName, false);
	exporter.ExportY2K(OUT_PATH + outFileName);

	return 0;
}