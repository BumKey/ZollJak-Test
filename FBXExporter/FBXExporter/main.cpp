#include <iostream>
#include "FBXExporter.h"

#define INPUT_FILE_PATH "FBX Models\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\goblin worrior\\Goblin_model\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\Cyclop\\Cyclop_Soldier\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\overlord\\models\\overlord\\"

#define OUTPUT_FILE_PATH "Output\\"

void main()
{
	std::string inFileName;
	std::string outFileName = "cyclop_tangent01.y2k";
	std::ofstream fout(OUTPUT_FILE_PATH + outFileName);
	FBXExporter myExporter;
	myExporter.SetClipNum(9);

	inFileName = "cyclop_soldier_attack_1.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "attack01", fout, false);

	inFileName = "cyclop_soldier_attack_2.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "attack02", fout, true);

	inFileName = "cyclop_soldier_death.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "death", fout, true);

	inFileName = "cyclop_soldier_hit.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "hit", fout, true);

	inFileName = "cyclop_soldier_idle.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "idle", fout, true);

	inFileName = "cyclop_soldier_run.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "run", fout, true);

	inFileName = "cyclop_soldier_stunned_idle.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "stunned", fout, true);

	inFileName = "cyclop_soldier_stunned_idle_hit.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "stunned_hit", fout, true);

	inFileName = "cyclop_soldier_walk.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "walk", fout, true);

	//inFileName = "goblin@run.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "run", fout, false);

	//inFileName = "goblin@attack01.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "attack01", fout, true);

	fout.close();
}