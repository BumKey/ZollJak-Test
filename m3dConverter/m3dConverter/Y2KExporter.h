#pragma once
#include "LoadM3D.h"
#include <fstream>

class Y2KExporter
{
public:
	Y2KExporter();
	~Y2KExporter();

public:
	void LoadM3D(const std::string& inFileName, bool hasAnim);
	void ExportY2K(const std::string& outFileName);

private:
	bool mHasAnim;
	M3DLoader mLoader;

	std::vector<USHORT> mIndices;
	std::vector<Subset> mSubsets;
	std::vector<Material> mMats;
	SkinnedData mSkinnedData;

	std::vector<Vertex::PosNormalTexTan> mBasicVertices;
	std::vector<Vertex::PosNormalTexTanSkinned> mSkinnedVertices;
};

