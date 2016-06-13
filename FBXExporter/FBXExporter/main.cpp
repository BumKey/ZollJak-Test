#include <iostream>
#include "FBXExporter.h"

//#define INPUT_FILE_PATH "FBX Models\\"
#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\goblin\\fbx\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\player\\golem\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\Cyclop\\Cyclop_Soldier\\cyclop_soldier_TM\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\overlord\\models\\overlord\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\Character Pack\\"

#define OUTPUT_FILE_PATH "C:\\Users\\Administrator\\Desktop\\Programming\\Direct3D\\d3d11CodeSet3\\Chapter 25 Character Animation\\SkinnedMesh\\Models\\"

void main()
{
	std::string inFileName;
	std::string outFileName = "goblin.y2k";
	std::ofstream fout(OUTPUT_FILE_PATH + outFileName);
	FBXExporter myExporter;
	myExporter.SetClipNum(10);

	//inFileName = "cyclop_soldier_idle.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "idle", fout);

	//inFileName = "cyclop_soldier_attack_1.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "attack01", fout);

	//inFileName = "cyclop_soldier_attack_2.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "attack02", fout);

	//inFileName = "cyclop_soldier_death.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "death", fout);

	//inFileName = "cyclop_soldier_hit.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "hit", fout);

	//inFileName = "cyclop_soldier_run.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "run", fout);

	//inFileName = "cyclop_soldier_stunned_idle.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "stunned", fout);

	//inFileName = "cyclop_soldier_stunned_idle_hit.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "stunned_hit", fout);

	//inFileName = "cyclop_soldier_walk.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "walk", fout);

	inFileName = "goblin@stand.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "stand01", fout);

	//inFileName = "goblin@attack01.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "attack01", fout);

	//inFileName = "goblin@attack02.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "attack02", fout);

	//inFileName = "goblin@damage.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "damage", fout);

	//inFileName = "goblin@dead.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "dead", fout);

	//inFileName = "goblin@drop down.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "drop_down", fout);

	////inFileName = "goblin@run.fbx";
	////myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	////myExporter.Export(outFileName, "run", fout);

	//inFileName = "goblin@sit up.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "sit_up", fout);

	//inFileName = "goblin@stand_vigilance.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "stand02", fout);

	//inFileName = "goblin@walk.fbx";
	//myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	//myExporter.Export(outFileName, "walk", fout);

	fout.close();
}