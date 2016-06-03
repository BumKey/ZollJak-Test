#include "FBXExporter.h"

#pragma region Constructor
FBXExporter::FBXExporter() : mFbxMgr(nullptr), mScene(nullptr), 
mHasAnimation(true), mOverWrite(false), 
mPrevCtrlPointCount(0), mPrevTriangleCount(0)
{
	QueryPerformanceFrequency(&mCPUFreq);

	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	mFbxMgr = FbxManager::Create();
	assert(mFbxMgr);

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(mFbxMgr, IOSROOT);
	mFbxMgr->SetIOSettings(ios);

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	mScene = FbxScene::Create(mFbxMgr, "My Scene");
	assert(mScene);
}
FBXExporter::~FBXExporter()
{
	mScene->Destroy();
	mFbxMgr->Destroy();

	mTriangles.clear();

	mVertices.clear();

	mBones.clear();

	for (auto itr = mMaterials.begin(); itr != mMaterials.end(); ++itr)
	{
		delete itr->second;
	}
	mMaterials.clear();
}
#pragma endregion

#pragma region Public method
bool FBXExporter::LoadFile(const std::string& fileName)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	QueryPerformanceCounter(&start);

	FbxImporter* importer = FbxImporter::Create(mFbxMgr, "");

	// Initialize the importer by providing a filename.
	assert(importer->Initialize(fileName.c_str(), -1, mFbxMgr->GetIOSettings()));
	assert(importer->Import(mScene));

	importer->Destroy();

	// Convert scene's AxisSystem to DirectX
	FbxAxisSystem sceneAxisSysytem = mScene->GetGlobalSettings().GetAxisSystem();
	if (sceneAxisSysytem != FbxAxisSystem::DirectX)
		FbxAxisSystem::DirectX.ConvertScene(mScene);

	// Set system's Unit to Centimeter
	FbxSystemUnit sceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
	if (sceneSystemUnit.GetScaleFactor() != 1.0f)
		FbxSystemUnit::cm.ConvertScene(mScene);

	// Converts mesh, NURBS and patch into triangle mesh
	FbxGeometryConverter gc(mFbxMgr);
	if (gc.Triangulate(mScene, true) == false)
		std::cout << "Not all meshes is trianglated" << std::endl;

	QueryPerformanceCounter(&end);
	std::cout << "\n\nFBX File Loaded: " << fileName << std::endl;

	return true;
}

void FBXExporter::Export(const std::string& fileName, const std::string& clipName, std::ofstream& fout, bool overWrite)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;

	mClipName = clipName;
	mOverWrite = overWrite;

	std::cout << "Exporting to... " << fileName << "\n\n";

	QueryPerformanceCounter(&start);
	ProcessBoneHierachy(mScene->GetRootNode());
	QueryPerformanceCounter(&end);
	std::cout << "Processing BoneHierachy: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	if(mBones.empty())
		mHasAnimation = false;

	QueryPerformanceCounter(&start);
	ProcessGeometry(mScene->GetRootNode());
	QueryPerformanceCounter(&end);
	std::cout << "Processing Geometry: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	if (overWrite == false)
	{
		QueryPerformanceCounter(&start);
		FinalProcedure();
		QueryPerformanceCounter(&end);
		std::cout << "Optimization: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
		std::cout << "\n\n";

		WriteMesh(fout);
	}

	if(mHasAnimation)
		WriteAnimation(fout);

	std::cout << "\n\nExport Done!\n";
	mBones.clear();
}
#pragma endregion 

#pragma region Geometry
void FBXExporter::ProcessGeometry(FbxNode* node)
{
	if (node->GetNodeAttribute())
	{
		switch (node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			FbxMesh* mesh = node->GetMesh();
			ProcessControlPoints(mesh);
			if (mHasAnimation)
				ReadSkinnedData(mesh);
		
			if (mOverWrite == false) {
				ProcessMesh(mesh);
				ProcessMaterials(node);
			}

			mPrevCtrlPointCount += mesh->GetControlPointsCount();
			mPrevTriangleCount += mesh->GetPolygonCount();
			break;
		}
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		ProcessGeometry(node->GetChild(i));
	}
}

void FBXExporter::ProcessControlPoints(FbxMesh* mesh)
{
	UINT ctrlPointCount = mesh->GetControlPointsCount();
	for (UINT i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint ctrlPoint;
		XMFLOAT3 position;
		position.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
		position.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
		position.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);
		ctrlPoint.Position = position;
		mControlPoints[mPrevCtrlPointCount + i] = ctrlPoint;
	}
}

void FBXExporter::ProcessMesh(FbxMesh* mesh)
{
	UINT currtriangleCount = mesh->GetPolygonCount();
	int polyVertex = 0;
	mTriangles.reserve(mPrevTriangleCount + currtriangleCount);

	for (UINT i = 0; i < currtriangleCount; ++i)
	{
		Triangle triangle;
		mTriangles.push_back(triangle);

		for (UINT j = 0; j < 3; ++j)
		{
			UINT ctrlPointIndex = mPrevCtrlPointCount + mesh->GetPolygonVertex(i, j);
			mIndices.push_back(ctrlPointIndex);
			ReadNormal(mesh, ctrlPointIndex, polyVertex);
			ReadUV(mesh, i, j);

			// Copy the blending info from each control point
			//for (UINT i = 0; i < ctrlPoint.BlendingInfo.size(); ++i)
			//{
			//	Vertex::VertexBlendingInfo currBlendingInfo;
			//	currBlendingInfo.BlendingIndex = ctrlPoint.BlendingInfo[i].mBlendingIndex;
			//	currBlendingInfo.BlendingWeight = ctrlPoint.BlendingInfo[i].mBlendingWeight;
			//	vertex.VertexBlendingInfos.push_back(currBlendingInfo);
			//}
			//// Sort the blending info so that later we can remove
			//// duplicated vertices
			//vertex.SortBlendingInfoByWeight();
			 
			++polyVertex;
		}
	}
}


void FBXExporter::ReadUV(FbxMesh* mesh, UINT triangleIndex, UINT positionInTriangle)
{
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);
	UINT ctrlPointIndex = mPrevCtrlPointCount + mesh->GetPolygonVertex(triangleIndex, positionInTriangle);
	UINT uvIndex = mesh->GetTextureUVIndex(triangleIndex, positionInTriangle);

	XMFLOAT2 uv;
	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			uv.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			uv.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(ctrlPointIndex);
			uv.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			uv.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			uv.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(uvIndex).mData[0]);
			uv.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(uvIndex).mData[1]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}
	UVInfo uvInfo;
	uvInfo.UV = uv;
	uvInfo.TriangleIndex = mPrevTriangleCount + triangleIndex;
	uvInfo.PositionInTriangle = positionInTriangle;

	std::vector<UVInfo> uvInfos = mControlPoints[ctrlPointIndex].UVInfos;
	auto iter = std::find(uvInfos.begin(), uvInfos.end(), uvInfo);
	if(iter == uvInfos.end())
		mControlPoints[ctrlPointIndex].UVInfos.push_back(uvInfo);
}

void FBXExporter::ReadNormal(FbxMesh* mesh, int ctrlPointIndex, int inpolyVertex)
{
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

	XMFLOAT3 outNormal;
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(ctrlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inpolyVertex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inpolyVertex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inpolyVertex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inpolyVertex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}

	std::vector<XMFLOAT3> normals = mControlPoints[ctrlPointIndex].Normals;
	auto iter = std::find(normals.begin(), normals.end(), outNormal);
	if(iter == normals.end())
		mControlPoints[ctrlPointIndex].Normals.push_back(outNormal);

}

void FBXExporter::ReadBinormal(FbxMesh* mesh, int ctrlPointIndex, int inpolyVertex, XMFLOAT3& outBinormal)
{
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(0);
	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(ctrlPointIndex);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inpolyVertex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inpolyVertex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inpolyVertex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inpolyVertex);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;;
		}
		break;
	}

}

void FBXExporter::ReadTangent(FbxMesh* mesh, int ctrlPointIndex, int inpolyVertex, XMFLOAT4& outTangent)
{
	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(0);
	switch (vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(ctrlPointIndex);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[3]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inpolyVertex).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inpolyVertex).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inpolyVertex).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inpolyVertex).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inpolyVertex);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[3]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}
}

#pragma endregion : Abount extracting geometry data

#pragma region Animation
void FBXExporter::ProcessBoneHierachy(FbxNode * rootNode)
{
	for (int childIndex = 0; childIndex < rootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* childNode = rootNode->GetChild(childIndex);
		ProcessBoneHierachy(childNode, 0, 0, -1);
	}
}

// Notice : It reads unnecessary bones too, so we remove it later.
void FBXExporter::ProcessBoneHierachy(FbxNode* node, int inDepth, int myIndex, int inParentIndex)
{
	if (node->GetNodeAttribute())
	{
		FbxNodeAttribute::EType type = node->GetNodeAttribute()->GetAttributeType();
		switch (type)
		{
		case FbxNodeAttribute::eMesh:
		case FbxNodeAttribute::eSkeleton:
			Bone bone;
			bone.ParentIndex = inParentIndex;
			bone.Name = node->GetName();
			bone.Node = node;

			ReadKeyframes(node, bone);
			
			mBones.push_back(bone);
			break;
		}
	}
	UINT childCount = node->GetChildCount();
	for (int i = 0; i < childCount; i++)
		ProcessBoneHierachy(node->GetChild(i), inDepth + 1, mBones.size(), myIndex);
}

void FBXExporter::ReadSkinnedData(FbxMesh* mesh)
{
	// Notice : It must procceed to right order.
	// though it repeat the same for_statement
	// its necessary to remove unnecessary bones and to apply right parent index.
	if (mesh->GetDeformerCount() > 0)
	{
	
		ReadBoneOffset(mesh);
		RemoveUnSkinnedBonesFromHierachy();
		if (mOverWrite == false)
			ReadBoneWeightsAndIndices(mesh);
	}
}

void FBXExporter::ReadBoneOffset(FbxMesh * mesh)
{
	UINT numDeformers = mesh->GetDeformerCount();
	for (unsigned int deformerIndex = 0; deformerIndex < numDeformers; ++deformerIndex)
	{
		FbxSkin* skin = FbxCast<FbxSkin>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		unsigned int numClusters = skin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numClusters; ++clusterIndex)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);
			FbxNode* bone = cluster->GetLink();
			std::string boneName = bone->GetName();
			int boneIndex = FindBoneIndexUsingName(boneName);

			FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(mesh->GetNode());
			FbxAMatrix meshTransform;
			FbxAMatrix bindPoseTransform;

			cluster->GetTransformMatrix(meshTransform);	// The transformation of the mesh at binding time
			cluster->GetTransformLinkMatrix(bindPoseTransform);	// The transformation of the link(bone) at binding time from bone local space to model local space
			mBones[boneIndex].BoneOffset = bindPoseTransform.Inverse() * meshTransform * geometryTransform;
		}
	}
}

void FBXExporter::RemoveUnSkinnedBonesFromHierachy()
{
	for (UINT i = 1; i < mBones.size(); ++i)
	{
		// For Goblin
		/*if (mBones[i].BoneOffset == FbxAMatrix())
		{
			for (UINT j = i + 1; j < mBones.size(); ++j) {
				if (mBones[j].ParentIndex >= i)
					--mBones[j].ParentIndex;
			}

			mBones.erase(mBones.begin() + i);
			--i;
		}*/

		// For model that has several meshes(weapons, sheild, etc..)
		if (mBones[i].ParentIndex == -1)
			mBones[i].ParentIndex = 0;
	}
}

void FBXExporter::ReadBoneWeightsAndIndices(FbxMesh * mesh)
{
	UINT numDeformers = mesh->GetDeformerCount();
	for (unsigned int deformerIndex = 0; deformerIndex < numDeformers; ++deformerIndex)
	{
		FbxSkin* skin = FbxCast<FbxSkin>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		unsigned int numClusters = skin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numClusters; ++clusterIndex)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);
			FbxNode* bone = cluster->GetLink();
			std::string boneName = bone->GetName();
			int boneIndex = FindBoneIndexUsingName(boneName);

			// Associate each Bone with the control points it affects
			UINT numBlendingIndices = cluster->GetControlPointIndicesCount();
			for (UINT i = 0; i < numBlendingIndices; ++i)
			{
				BlendingIndexWeightPair blendingIndexWeightPair;
				blendingIndexWeightPair.mBlendingIndex = boneIndex;
				blendingIndexWeightPair.mBlendingWeight = cluster->GetControlPointWeights()[i];
				mControlPoints[mPrevCtrlPointCount + cluster->GetControlPointIndices()[i]].BlendingInfo.push_back(blendingIndexWeightPair);
			}
		}
	}
	// Some of the control points only have less than 4 Bones affecting them.
	// For a normal renderer, there are usually 4 Bones
	// I am adding more dummy Bones if there isn't enough
	BlendingIndexWeightPair blendingIndexWeightPair;
	blendingIndexWeightPair.mBlendingIndex = 0;
	blendingIndexWeightPair.mBlendingWeight = 0;
	for (UINT i = 0; i < mesh->GetControlPointsCount(); ++i)
	{
		for (UINT j = mControlPoints[mPrevCtrlPointCount + i].BlendingInfo.size(); j < 4; ++j)
		{
			mControlPoints[mPrevCtrlPointCount + i].BlendingInfo.push_back(blendingIndexWeightPair);
		}
	}
}
void FBXExporter::ReadKeyframes(FbxNode * node, Bone& bone)
{
	FbxAnimStack* currAnimStack = mScene->GetSrcObject<FbxAnimStack>(0);
	FbxTakeInfo* takeInfo = mScene->GetTakeInfo(currAnimStack->GetName());
	FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

	UINT startFrameCount = start.GetFrameCount(FbxTime::eFrames24);
	UINT endFrameCount = end.GetFrameCount(FbxTime::eFrames24);
	float animationLength = (end.GetMilliSeconds() - start.GetMilliSeconds());

	//mAnimationStack[stackIndex]->BakeLayers(mScene->GetAnimationEvaluator(),
	//	takeInfo->mLocalTimeSpan.GetStart(),
	//	takeInfo->mLocalTimeSpan.GetStop(),
	//	FbxTime::eFrames24);
	
	for (UINT t = startFrameCount; t <= endFrameCount; ++t)
	{
		float timePos = ((float)t / endFrameCount - startFrameCount) * animationLength;

		FbxTime currTime;
		currTime.SetFrame(t, FbxTime::eFrames24);
		FbxAMatrix localTransform = node->EvaluateLocalTransform(currTime);
		FbxVector4		S = localTransform.GetS();
		FbxQuaternion	Q = localTransform.GetQ();
		FbxVector4		T = localTransform.GetT();

		Keyframe keyframe;
		keyframe.TimePos = timePos;

		keyframe.Scale.x = S.mData[0];
		keyframe.Scale.y = S.mData[1];
		keyframe.Scale.z = S.mData[2];

		keyframe.RotationQuat.x = Q.mData[0];
		keyframe.RotationQuat.y = Q.mData[1];
		keyframe.RotationQuat.z = Q.mData[2];
		keyframe.RotationQuat.w = Q.mData[3];

		keyframe.Translation.x = T.mData[0];
		keyframe.Translation.y = T.mData[1];
		keyframe.Translation.z = T.mData[2];

		bone.Keyframes.push_back(keyframe);
	}
}

UINT FBXExporter::FindBoneIndexUsingName(const std::string& inBoneName)
{
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].Name == inBoneName)
		{
			return i;
		}
	}
	std::cout << "Skeleton information in FBX file is corrupted." << std::endl;
}

#pragma endregion : Abount extracting animation data.

#pragma region Material
void FBXExporter::ProcessMaterials(FbxNode* node)
{
	AssociateMaterialToMesh(node);

	UINT materialCount = node->GetMaterialCount();
	for (UINT i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);
		UINT uniqueMatID = surfaceMaterial->GetUniqueID();

		ProcessMaterialAttribute(surfaceMaterial, uniqueMatID);
		ProcessMaterialTexture(surfaceMaterial, mMaterials[uniqueMatID]);
	}
	
}

void FBXExporter::AssociateMaterialToMesh(FbxNode* node)
{
	static UINT meshCount(0);
	FbxLayerElementArrayTemplate<int>* materialIndices;
	FbxGeometryElement::EMappingMode materialMappingMode;
	FbxMesh* mesh = node->GetMesh();
	if(mesh->GetElementMaterial())
	{
		materialIndices = &(mesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = mesh->GetElementMaterial()->GetMappingMode();

		if(materialIndices)
		{
			UINT matIndexCount = materialIndices->GetCount();
			UINT meshPolyCount = mesh->GetPolygonCount();
			switch(materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (matIndexCount == meshPolyCount)
				{
					for (UINT i = 0; i < meshPolyCount; ++i)
					{
						UINT materialIndex = materialIndices->GetAt(i);
						mTriangles[mPrevTriangleCount + i].SubsetID = materialIndex + mMaterials.size();
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				UINT materialIndex = materialIndices->GetAt(0);
				for (UINT i = 0; i < meshPolyCount; ++i)
				{
					mTriangles[mPrevTriangleCount + i].SubsetID = materialIndex + mMaterials.size();
				}
			}
			break;

			default:
				std::cout<< "Invalid mapping mode for material\n" << std::endl;
			}
		}
	}
	++meshCount;
}

void FBXExporter::ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, UINT inMaterialIndex)
{
	FbxDouble3 double3;
	FbxDouble double1;
	if (inMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		Material* currMaterial = new Material();

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Ambient;
		currMaterial->Ambient.x = static_cast<float>(double3.mData[0]);
		currMaterial->Ambient.y = static_cast<float>(double3.mData[1]);
		currMaterial->Ambient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Diffuse;
		currMaterial->Diffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->Diffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->Diffuse.z = static_cast<float>(double3.mData[2]);

		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Specular;
		currMaterial->Specular.x = static_cast<float>(double3.mData[0]);
		currMaterial->Specular.y = static_cast<float>(double3.mData[1]);
		currMaterial->Specular.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Emissive;
		currMaterial->Emissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->Emissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->Emissive.z = static_cast<float>(double3.mData[2]);

		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Reflection;
		currMaterial->Reflection.x = static_cast<float>(double3.mData[0]);
		currMaterial->Reflection.y = static_cast<float>(double3.mData[1]);
		currMaterial->Reflection.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->TransparencyFactor;
		currMaterial->TransparencyFactor = double1;

		// Shininess
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Shininess;
		currMaterial->Shininess = double1;

		// Specular Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->SpecularFactor;
		currMaterial->SpecularPower = double1;


		// Reflection Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->ReflectionFactor;
		currMaterial->ReflectionFactor = double1;

		mMaterials[inMaterialIndex] = currMaterial;
	}
	else if (inMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		Material* currMaterial = new Material();
		FbxSurfaceLambert* lambert = static_cast<FbxSurfaceLambert*>(inMaterial);

		// Amibent Color
		double3 = lambert->Ambient;
		currMaterial->Ambient.x = static_cast<float>(double3.mData[0]);
		currMaterial->Ambient.y = static_cast<float>(double3.mData[1]);
		currMaterial->Ambient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = lambert->Diffuse;
		currMaterial->Diffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->Diffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->Diffuse.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = lambert->Emissive;
		currMaterial->Emissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->Emissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->Emissive.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = lambert->TransparencyFactor;
		currMaterial->TransparencyFactor = double1;

		mMaterials[inMaterialIndex] = currMaterial;
	}
}

void FBXExporter::ProcessMaterialTexture(FbxSurfaceMaterial * inMaterial, Material * ioMaterial)
{
	UINT textureIndex = 0;
	FbxProperty property;

	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = inMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
		if (property.IsValid())
		{
			UINT textureCount = property.GetSrcObjectCount<FbxTexture>();
			for (UINT i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if (layeredTexture)
				{
					std::cout << "Layered Texture is currently unsupported" << std::endl;;
				}
				else
				{
					FbxTexture* texture = property.GetSrcObject<FbxFileTexture>(i);
					if (texture)
					{
						std::string textureType = property.GetNameAsCStr();
						FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

						if (fileTexture)
						{
							if (textureType == "DiffuseColor")
							{
								ioMaterial->DiffuseMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "SpecularColor")
							{
								ioMaterial->SpecularMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "NormalMap")
							{
								ioMaterial->NormalMapName = fileTexture->GetRelativeFileName();
							}
						}
					}
				}
			}
		}
	}
}
#pragma endregion : About extracting material data.

#pragma region FinalProcedure
void FBXExporter::FinalProcedure()
{
	// First get a list of unique vertices
	//std::vector<Vertex::Skinned> uniqueVertices;
	//for (UINT i = 0; i < mTriangles.size(); ++i)
	//{
	//	for (UINT j = 0; j < 3; ++j)
	//	{
	//		// If current vertex has not been added to
	//		// our unique vertex list, then we add it
	//		if (FindVertex(mVertices[i * 3 + j], uniqueVertices) == -1)
	//		{
	//			uniqueVertices.push_back(mVertices[i * 3 + j]);
	//		}
	//	}
	//}

	UINT ctrlPointSize = mControlPoints.size();
	mVertices.resize(ctrlPointSize);

	for (UINT i = 0; i < ctrlPointSize; ++i)
	{
		const CtrlPoint ctrlPoint = mControlPoints[i];
		mVertices[i].Position = ctrlPoint.Position;

		XMFLOAT3 normal;
		for (auto n : ctrlPoint.Normals)
			normal = normal + n;

		XMVECTOR vNormal = XMVector3Normalize(XMLoadFloat3(&normal));
		XMStoreFloat3(&normal, vNormal);
		mVertices[i].Normal = normal;

		// Copy the blending info from each control point
		for (UINT j = 0; j < ctrlPoint.BlendingInfo.size(); ++j)
		{
			Vertex::VertexBlendingInfo blendingInfo;
			blendingInfo.BlendingIndex = ctrlPoint.BlendingInfo[j].mBlendingIndex;
			blendingInfo.BlendingWeight = ctrlPoint.BlendingInfo[j].mBlendingWeight;
			mVertices[i].VertexBlendingInfos.push_back(blendingInfo);
		}

		// Push back new vertex If it has different UV
		// Other information is same.
		
		for (UINT j = 0; j < ctrlPoint.UVInfos.size(); ++j)
		{
			UVInfo uvInfo = ctrlPoint.UVInfos[j];
			if (mIndices[uvInfo.TriangleIndex * 3 + uvInfo.PositionInTriangle] == i)
			{
				mVertices[i].Tex = uvInfo.UV;
			}
			else {
				mIndices[uvInfo.TriangleIndex * 3 + uvInfo.PositionInTriangle] = mVertices.size();

				Vertex::Skinned vertex;
				vertex.Position = mVertices[i].Position;
				vertex.Normal = mVertices[i].Normal;
				vertex.VertexBlendingInfos = mVertices[i].VertexBlendingInfos;

				vertex.Tex = ctrlPoint.UVInfos[j].UV;
				mVertices.push_back(vertex);
			}
		}
	}
	// Now we update the index buffer
	//for (UINT i = 0; i < mTriangles.size(); ++i)
	//{
	//	for (UINT j = 0; j < 3; ++j)
	//	{
	//		mTriangles[i].Indices[j] = FindVertex(mVertices[i * 3 + j], uniqueVertices);
	//	}
	//}

	//for (UINT i = 0; i < mIndices.size(); i += 3)
	//{
	//	for (UINT j = 0; j < 3; j++)
	//	{
	//		XMFLOAT3 vertex0, vertex1, vertex2;
	//		XMVECTOR e0, e1;
	//		vertex0 = mVertices[i + j].Position;
	//		vertex1 = mVertices[i + (j + 1) % 3].Position;
	//		vertex2 = mVertices[i + (j + 2) % 3].Position;

	//		e0 = XMLoadFloat3(&(vertex1 - vertex0));
	//		e1 = XMLoadFloat3(&(vertex2 - vertex0));

	//		XMFLOAT2 uv0, uv1, uv2;
	//		XMVECTOR d_uv0, d_uv1;
	//		uv0 = mVertices[i+ j].Tex;
	//		uv1 = mVertices[i + (j + 1) % 3].Tex;
	//		uv2 = mVertices[i + (j + 2) % 3].Tex;

	//		d_uv0 = XMLoadFloat2(&(uv1 - uv0));
	//		d_uv1 = XMLoadFloat2(&(uv2 - uv0));

	//		XMVECTOR a = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	//		XMVECTOR b = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	//		XMMATRIX UV(d_uv0, d_uv1, a, b);
	//		XMMATRIX E(e0, e1, a, b);

	//		/*	FLOAT det = d_uv0 *d_uv1.y - d_uv0.y*d_uv1.x;
	//		FLOAT Tx = (d_uv1.y * e0.x - d_uv0.y*e1.x)/det;
	//		FLOAT Ty = (d_uv1.y * e0.y - d_uv0.y*e1.y)/det;
	//		FLOAT Tz = (d_uv1.y * e0.z - d_uv0.y*e1.z)/det;
	//		FLOAT length = sqrtf(Tx*Tx + Ty*Ty + Tz*Tz);*/

	//		XMVECTOR det;
	//		XMMATRIX result = XMMatrixInverse(&det, UV)*E;
	//		if (XMVectorGetX(det) == 0.0f)
	//			continue;
	//		FLOAT Tx = result._21;
	//		FLOAT Ty = result._22;
	//		FLOAT Tz = result._23;

	//		// Binormal
	//		XMFLOAT3 binormal = Float3Normalize(XMFLOAT3(Tx, Ty, Tz));
	//		mVertices[i + j].Binormals.push_back(binormal);
	//	}
	//}

	// Calculate tangent
	//for (UINT i = 0; i < mTriangles.size(); ++i)
	//{
	//	for (UINT j = 0; j < 3; j++)
	//	{
	//		XMFLOAT3 vertex0, vertex1, vertex2;
	//		XMVECTOR e0, e1;
	//		vertex0 = uniqueVertices[mTriangles[i].Indices[j]].Position;
	//		vertex1 = uniqueVertices[mTriangles[i].Indices[(j + 1) % 3]].Position;
	//		vertex2 = uniqueVertices[mTriangles[i].Indices[(j + 2) % 3]].Position;

	//		e0 = XMLoadFloat3(&(vertex1 - vertex0));
	//		e1 = XMLoadFloat3(&(vertex2 - vertex0));

	//		XMFLOAT2 uv0, uv1, uv2;
	//		XMVECTOR d_uv0, d_uv1;
	//		uv0 = uniqueVertices[mTriangles[i].Indices[j]].Tex;
	//		uv1 = uniqueVertices[mTriangles[i].Indices[(j + 1) % 3]].Tex;
	//		uv2 = uniqueVertices[mTriangles[i].Indices[(j + 2) % 3]].Tex;

	//		d_uv0 = XMLoadFloat2(&(uv1 - uv0));
	//		d_uv1 = XMLoadFloat2(&(uv2 - uv0));

	//		XMVECTOR a = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	//		XMVECTOR b = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	//		XMMATRIX UV(d_uv0, d_uv1, a, b);
	//		XMMATRIX E(e0, e1, a, b);

	//		/*	FLOAT det = d_uv0 *d_uv1.y - d_uv0.y*d_uv1.x;
	//		FLOAT Tx = (d_uv1.y * e0.x - d_uv0.y*e1.x)/det;
	//		FLOAT Ty = (d_uv1.y * e0.y - d_uv0.y*e1.y)/det;
	//		FLOAT Tz = (d_uv1.y * e0.z - d_uv0.y*e1.z)/det;
	//		FLOAT length = sqrtf(Tx*Tx + Ty*Ty + Tz*Tz);*/

	//		XMVECTOR det;
	//		XMMATRIX result = XMMatrixInverse(&det, UV)*E;
	//		if (XMVectorGetX(det) == 0.0f)
	//			continue;
	//		FLOAT Tx = result._21;
	//		FLOAT Ty = result._22;
	//		FLOAT Tz = result._23;

	//		// Binormal
	//		XMFLOAT3 binormal = Float3Normalize(XMFLOAT3(Tx, Ty, Tz));
	//		uniqueVertices[mTriangles[i].Indices[j]].Binormals.push_back(binormal);
	//	}
	//}

	//for (auto& iter : uniqueVertices)
	//{
	//	// Get the tangent value to cross normal and Binormal
	//	XMFLOAT3 tangentU, binormal(0.0f, 0.0f, 0.0f);
	//	for (auto b : iter.Binormals)
	//	{
	//		binormal.x += b.x;
	//		binormal.y += b.y;
	//		binormal.z += b.z;
	//	}
	//	binormal.x /= iter.Binormals.size();
	//	binormal.y /= iter.Binormals.size();
	//	binormal.z /= iter.Binormals.size();

	//	XMVECTOR vBi = XMLoadFloat3(&binormal);
	//	XMVECTOR w0 = XMVector3Normalize(XMLoadFloat3(&iter.Normal));
	//	XMVECTOR w2 = XMVector3Normalize(XMVector3Cross(w0, vBi));
	//	XMVECTOR vTan = XMVector3Cross(w2, w0);
	//	XMStoreFloat3(&tangentU, vTan);
	//	iter.TangentU.x = tangentU.x;
	//	iter.TangentU.y = tangentU.y;
	//	iter.TangentU.z = tangentU.z;
	//	iter.TangentU.w = 1.0f;
	//}

	//mVertices.clear();
	//mVertices = uniqueVertices;
	//uniqueVertices.clear();

	// Now we sort the triangles by materials to reduce 
	// shader's workload
	std::sort(mTriangles.begin(), mTriangles.end());

	// Make SubsetTable 
	UINT count(0);
	std::unordered_map<UINT, UINT> material_TriangleCount;

	for (auto iterT : mTriangles)
		++material_TriangleCount[iterT.SubsetID];

	mSubsets.resize(mMaterials.size());
	

	for (UINT i = 0; i < mSubsets.size(); ++i)
	{
		mSubsets[i].ID = i;
		mSubsets[i].FaceCount = material_TriangleCount[i];
		if (i + 1<mSubsets.size())
			mSubsets[i + 1].FaceStart = mSubsets[i].FaceStart + mSubsets[i].FaceCount;
	}

	// If model doesn't have normal map
	for (auto iterM : mMaterials)
	{
		if (iterM.second->DiffuseMapName == "")
			iterM.second->DiffuseMapName = "None";

		if (iterM.second->NormalMapName == "")
			iterM.second->NormalMapName = "None";
	}
}

int FBXExporter::FindVertex(const Vertex::Skinned& inTargetVertex, const std::vector<Vertex::Skinned>& uniqueVertices)
{
	for (UINT i = 0; i < uniqueVertices.size(); ++i)
	{
		if (inTargetVertex == uniqueVertices[i])
		{
			return i;
		}
	}

	return -1;
}
#pragma endregion  : Final procedure to export y2k file format.

#pragma region File
void FBXExporter::WriteMesh(std::ostream& fout)
{
	using namespace std;
	if (mHasAnimation && mOverWrite == false)
	{
		// Notice: �� ����ũ�� ����
		fout << "***************Y2K-File-Header***************" << endl;
		fout << "#Materials: " << mMaterials.size() << endl;
		fout << "#Vertices: " << mVertices.size() << endl;
		fout << "#Triangles: " << mTriangles.size() << endl;
		fout << "#Bones: " << mBones.size() << endl;
		fout << "#AnimationClips: " << mClipNum << endl << endl;	
	}
	else
	{
		fout << "***************KPU-File-Header***************" << endl;
		fout << "#Materials: "	<< mMaterials.size() << endl;
		fout << "#Vertices: "	<< mVertices.size() << endl;
		fout << "#Triangles: "  << mTriangles.size() << endl;
		fout << "#Bones: "		<< 0 << endl;		// �� ������ �ʿ����. ���� �ִϸ��̼� ������
		fout << "#AnimationClips: " << 0 << endl << endl;
	}

	fout << "***************Materials*********************" << endl;
	for (auto iterM : mMaterials)
	{
		fout << "Ambient: "		<< iterM.second->Ambient << endl;
		fout << "Diffuse: "		<< iterM.second->Diffuse << endl;
		fout << "Specular: "	<< iterM.second->Specular << endl;
		fout << "SpecPower: "	<< iterM.second->SpecularPower << endl;
		fout << "Reflectivity: " << iterM.second->Reflection << endl;
		fout << "AlphaClip: "	<< false << endl;
		fout << "Effect: "		<< "Normal" << endl;
		fout << "DiffuseMap: "	<< iterM.second->DiffuseMapName << endl;
		fout << "NormalMap: "	<< iterM.second->NormalMapName << endl << endl;;
	}

	fout << "***************SubsetTable*******************" << endl;
	for (auto iterS : mSubsets)
	{
		fout << "SubsetID: "	<< iterS.ID			<< " ";
		fout << "VertexStart: " << 0				<< " ";
		fout << "VetexCount: "	<< 0				<< " ";
		fout << "FaceStart: "	<< iterS.FaceStart	<< " ";
		fout << "FaceCount: "	<< iterS.FaceCount  << endl;
	}
	fout << endl;

	if (mHasAnimation)
	{
		fout << "***************Vertices**********************" << endl;
		for (auto vertex : mVertices)
		{
			// Remeber this order.
			fout << "Pos: "		<< vertex.Position << endl;
			fout << "Tangent: " << vertex.TangentU << endl;
			fout << "Normal: "	<< vertex.Normal << endl;
			fout << "Tex: "		<< vertex.Tex.x << " " << 1.0f - vertex.Tex.y << endl;
			fout << "BlendWeights: " << vertex.VertexBlendingInfos[0].BlendingWeight << " " <<
				vertex.VertexBlendingInfos[1].BlendingWeight << " " <<
				vertex.VertexBlendingInfos[2].BlendingWeight << " " <<
				vertex.VertexBlendingInfos[3].BlendingWeight << endl;
			fout << "BlendIndices: " << vertex.VertexBlendingInfos[0].BlendingIndex << " " <<
				vertex.VertexBlendingInfos[1].BlendingIndex << " " <<
				vertex.VertexBlendingInfos[2].BlendingIndex << " " <<
				vertex.VertexBlendingInfos[3].BlendingIndex << endl;
			fout << endl;
		}
	}
	else
	{
		fout << "***************Vertices**********************" << endl;
		for (auto vertex : mVertices)
		{
			// Remeber this order.
			fout << "Pos: " << vertex.Position << endl;
			fout << "Tangent: " << vertex.TangentU << endl;
			fout << "Normal: " << vertex.Normal << endl;
			fout << "Tex: " << vertex.Tex.x << " " << 1.0f - vertex.Tex.y << endl;
			fout << endl;
		}
	}
	fout << "***************Triangles*********************" << endl;
	UINT count(0);
	for (UINT i = 0; i < mIndices.size(); ++i)
	{
		fout << mIndices[i] << " ";
		++count;
		if (count >= 3)
		{
			fout << endl;
			count = 0;
		}
	}

	fout << endl;
}

void FBXExporter::WriteAnimation(std::ostream& fout)
{
	using namespace std;
	if (mOverWrite == false)
	{
		fout << "***************BoneOffsets*******************" << endl;
		for (UINT i = 0; i < mBones.size(); ++i)
		{
			fout << "BoneOffset" << i << " ";
			Utilities::WriteMatrix(fout, mBones[i].BoneOffset, true);
		}

		fout << endl;
		fout << "***************BoneHierarchy*****************" << endl;
		for (UINT i = 0; i < mBones.size(); ++i)
			fout << "ParentIndexOfBone" << i << " " << mBones[i].ParentIndex << endl;

		fout << endl;
		fout << "***************AnimationClips****************" << endl;
	}
	// mAnimationName.erase(mAnimationName.find(" "), 1)

	fout << "AnimationClip " << mClipName << endl;

	fout << "{" << endl;
	for (UINT bonIndex = 0; bonIndex < mBones.size(); ++bonIndex)
	{
		fout << "\tBone" << bonIndex << " #KeyFrames: " << mBones[bonIndex].Keyframes.size() << endl;
		fout << "\t{" << endl;
		for (auto iterK : mBones[bonIndex].Keyframes)
		{
			fout << "\t\t";
			fout << "Time: " << iterK.TimePos / 1000.0f << " ";
			fout << "Pos: " << iterK.Translation << " ";
			fout << "Scale: " << iterK.Scale << " ";
			fout << "Quat: " << iterK.RotationQuat << endl;
		}
			
		fout << "\t}" << endl << endl;
	}
	fout << "}" << endl;
}

#pragma endregion : File Exporting