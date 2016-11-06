#include <iostream>
#include "FBXExporter.h"

//#define INPUT_FILE_PATH "FBX Models\\"
#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Desktop\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\goblin\\fbx\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\hammer warrior\\hammer_warrior_LOD0_TM\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\player\\golem\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\Cyclop\\Cyclop_Soldier\\cyclop_soldier_TM\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\overlord\\models\\overlord\\"
//#define INPUT_FILE_PATH "C:\\Users\\Administrator\\Documents\\Test\\Assets\\Character Pack\\"

#define OUTPUT_FILE_PATH "C:\\Users\\Administrator\\Desktop\\"

void main()
{
	std::string inFileName;
	std::string outFileName = "sample.m3d";
	std::ofstream fout(OUTPUT_FILE_PATH + outFileName);
	FBXExporter myExporter;
	//myExporter.SetClipNum(10);

	//FrameInfo frameInfo;
	//frameInfo[std::string("idle")] = std::make_pair(149, 187);
	//frameInfo[std::string("walk")] = std::make_pair(60, 114);
	//frameInfo[std::string("run")] = std::make_pair(115, 145);
	//frameInfo[std::string("damage")] = std::make_pair(638, 657);
	//frameInfo[std::string("attack01")] = std::make_pair(188, 240);
	//frameInfo[std::string("attack02")] = std::make_pair(241, 290);
	//frameInfo[std::string("attack03")] = std::make_pair(291, 347);

	//myExporter.AllInOneTakeMode(frameInfo);

	inFileName = "sample.fbx";
	myExporter.LoadFile(INPUT_FILE_PATH + inFileName);
	myExporter.Export(outFileName, "", fout);

	fout.close();
}