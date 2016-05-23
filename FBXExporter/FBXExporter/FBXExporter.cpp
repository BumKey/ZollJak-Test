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
	UINT triangleCount = mesh->GetPolygonCount();
	int vertexCounter = 0;
	mTriangles.reserve(mPrevTriangleCount + triangleCount);

	for (UINT i = 0; i < triangleCount; ++i)
	{
		Triangle triangle;
		mTriangles.push_back(triangle);

		for (UINT j = 0; j < 3; ++j)
		{
			XMFLOAT3 normal;
			XMFLOAT4 tangent;
			XMFLOAT2 UV;
			ZeroMemory(&tangent, sizeof(XMFLOAT4));

			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
			CtrlPoint ctrlPoint = mControlPoints[mPrevCtrlPointCount + ctrlPointIndex];

			ReadNormal(mesh, ctrlPointIndex, vertexCounter, normal);
			for (int k = 0; k < 1; ++k)		// Set UV depend on Diffuse map
			{
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), k, UV);
			}

			Vertex::Skinned vertex;
			vertex.Position = ctrlPoint.Position;
			vertex.Normal = normal;
			vertex.Tex = UV;
			// Copy the blending info from each control point
			for (UINT i = 0; i < ctrlPoint.BlendingInfo.size(); ++i)
			{
				Vertex::VertexBlendingInfo currBlendingInfo;
				currBlendingInfo.BlendingIndex = ctrlPoint.BlendingInfo[i].mBlendingIndex;
				currBlendingInfo.BlendingWeight = ctrlPoint.BlendingInfo[i].mBlendingWeight;
				vertex.VertexBlendingInfos.push_back(currBlendingInfo);
			}
			// Sort the blending info so that later we can remove
			// duplicated vertices
			vertex.SortBlendingInfoByWeight();

			mVertices.push_back(vertex);
			mTriangles.back().Indices.push_back(vertexCounter);
			++vertexCounter;
		}
	}
}


void FBXExporter::ReadUV(FbxMesh* mesh, int ctrlPointIndex, int UVIndex, int UVlayerNum, XMFLOAT2& uv)
{
	if (UVlayerNum >= 2 || mesh->GetElementUVCount() <= UVlayerNum)
	{
		std::cout << "Invalid UV Layer Number" << std::endl;
	}
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(UVlayerNum);

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
			uv.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(UVIndex).mData[0]);
			uv.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(UVIndex).mData[1]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}
}

void FBXExporter::ReadNormal(FbxMesh* mesh, int ctrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);
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
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
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
}

void FBXExporter::ReadBinormal(FbxMesh* mesh, int ctrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
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
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
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

void FBXExporter::ReadTangent(FbxMesh* mesh, int ctrlPointIndex, int inVertexCounter, XMFLOAT4& outTangent)
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
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
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
	for (UINT i = mPrevCtrlPointCount; i < mControlPoints.size(); ++i)
	{
		for (UINT j = mControlPoints[i].BlendingInfo.size(); j <= 4; ++j)
		{
			mControlPoints[i].BlendingInfo.push_back(blendingIndexWeightPair);
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
					FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
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
	std::vector<Vertex::Skinned> uniqueVertices;
	for (UINT i = 0; i < mTriangles.size(); ++i)
	{
		for (UINT j = 0; j < 3; ++j)
		{
			// If current vertex has not been added to
			// our unique vertex list, then we add it
			if (FindVertex(mVertices[i * 3 + j], uniqueVertices) == -1)
			{
				uniqueVertices.push_back(mVertices[i * 3 + j]);
			}
		}
	}

	// Now we update the index buffer
	for (UINT i = 0; i < mTriangles.size(); ++i)
	{
		for (UINT j = 0; j < 3; ++j)
		{
			mTriangles[i].Indices[j] = FindVertex(mVertices[i * 3 + j], uniqueVertices);
		}
	}

	// Calculate Tagent
	for (UINT i = 0; i < mTriangles.size(); ++i)
	{
		for (UINT j = 0; j < 3; j++)
		{
			XMFLOAT3 vertex0, vertex1, vertex2;
			XMVECTOR e0, e1;
			vertex0 = uniqueVertices[mTriangles[i].Indices[j]].Position;
			vertex1 = uniqueVertices[mTriangles[i].Indices[(j+1)%3]].Position;
			vertex2 = uniqueVertices[mTriangles[i].Indices[(j+2)%3]].Position;

			e0 = XMLoadFloat3(&(vertex1 - vertex0));
			e1 = XMLoadFloat3(&(vertex2 - vertex0));

			XMFLOAT2 uv0, uv1, uv2, d_uv0, d_uv1;
			uv0 = uniqueVertices[mTriangles[i].Indices[j]].Tex;
			uv1 = uniqueVertices[mTriangles[i].Indices[(j+1)%3]].Tex;
			uv2 = uniqueVertices[mTriangles[i].Indices[(j+2)%3]].Tex;

			d_uv0 = uv1 - uv0;
			d_uv1 = uv2 - uv0;

			FLOAT det = d_uv0.x*d_uv1.y - d_uv0.y*d_uv1.x;
			assert(det != 0);
			FLOAT a = d_uv1.y / det;
			FLOAT b = -d_uv0.y / det;
			FLOAT c = -d_uv1.x / det;
			FLOAT d = d_uv0.x / det;
			
			XMMATRIX UV(
				a, b, 0.0f, 0.0f,
				c, d, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f);

			XMVECTOR zeroVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			XMMATRIX E(e0, e1, zeroVector, zeroVector);

			XMMATRIX result = UV*E;
			XMVECTOR vBinormal = XMLoadFloat3(&XMFLOAT3(result(0, 1), result(1, 1), result(2, 1)));
			XMVector3Normalize(vBinormal);

			// Binormal
			XMFLOAT3 binormal;
			XMStoreFloat3(&binormal, vBinormal);
			uniqueVertices[mTriangles[i].Indices[j]].Binormals.push_back(binormal);
		}
	}

	for (auto& iter : uniqueVertices)
	{
		// Get the tangent value to cross normal and Binormal
		XMFLOAT3 tangentU, binormal(0.0f, 0.0f, 0.0f);
		for (auto b : iter.Binormals)
		{
			binormal.x += b.x;
			binormal.y += b.y;
			binormal.z += b.z;
		}
		binormal.x /= iter.Binormals.size();
		binormal.y /= iter.Binormals.size();
		binormal.z /= iter.Binormals.size();

		XMVECTOR vNo = XMVector3Normalize(XMLoadFloat3(&iter.Normal));
		XMVECTOR vBi = XMLoadFloat3(&binormal);
		XMVECTOR vTan = XMVector3Cross(vNo, vBi);
		XMStoreFloat3(&tangentU, XMVector3Normalize(vTan));
		iter.TangentU.x = tangentU.x;
		iter.TangentU.y = tangentU.y;
		iter.TangentU.z = tangentU.z;
		iter.TangentU.w = 1.0f;
	}

	mVertices.clear();
	mVertices = uniqueVertices;
	uniqueVertices.clear();

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
		// Notice: 원 테이크만 지원
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
		fout << "#Bones: "		<< 0 << endl;		// 이 정보는 필요없다. 모델이 애니메이션 없으면
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
	for (UINT i = 0; i < mTriangles.size(); ++i)
		fout << mTriangles[i].Indices[0] << " " << mTriangles[i].Indices[1] << " " << mTriangles[i].Indices[2] << endl;

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