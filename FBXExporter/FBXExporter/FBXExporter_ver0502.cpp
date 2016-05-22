#include "FBXExporter.h"

#pragma region Constructor
FBXExporter::FBXExporter() : mFbxMgr(nullptr), mScene(nullptr), mHasAnimation(false)
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
}
FBXExporter::~FBXExporter()
{
	mScene->Destroy();
	mFbxMgr->Destroy();

	mTriangles.clear();

	mVertices.clear();

	mBones.clear();

	for (auto itr = mMaterialLookUp.begin(); itr != mMaterialLookUp.end(); ++itr)
	{
		delete itr->second;
	}
	mMaterialLookUp.clear();
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

	mHasAnimation = HasAnimation();

	QueryPerformanceCounter(&end);
	std::cout << "Loading FBX File: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	return true;
}

void FBXExporter::Export(const std::string& fileName)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	
	QueryPerformanceCounter(&start);
	ReadBoneHierachy();

	std::cout << "\n\n\n\nExporting Model:" << fileName << "\n";
	QueryPerformanceCounter(&end);
	std::cout << "Processing Skeleton Hierarchy: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	QueryPerformanceCounter(&start);
	ProcessNodes(mScene->GetRootNode());
	QueryPerformanceCounter(&end);
	std::cout << "Processing Geometry: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	QueryPerformanceCounter(&start);
	Optimize();
	QueryPerformanceCounter(&end);
	std::cout << "Optimization: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
	//PrintMaterial();
	std::cout << "\n\n";

	std::string outFileName = OUTPUT_FILE_PATH + fileName + ".y2k";
	std::ofstream fout(outFileName);
	WriteMesh(fout);

	if(mHasAnimation)
		WriteAnimation(fout);

	std::cout << "\n\nExport Done!\n";
}
#pragma endregion 

#pragma region Parsing from FBX root node
void FBXExporter::ProcessNodes(FbxNode* node)
{
	if (node->GetNodeAttribute())
	{
		switch (node->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessControlPoints(node);
			if (mHasAnimation)
				ProcessAnimations(node);
			ProcessMesh(node);
			AssociateMaterialToMesh(node);
			ProcessMaterials(node);
			break;

		default:
			break;
		}
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		ProcessNodes(node->GetChild(i));
	}
}
#pragma endregion Parsing Entry Point

#pragma region Mesh
void FBXExporter::ProcessMesh(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();

	UINT triangleCount = mesh->GetPolygonCount();
	int vertexCounter = 0;
	mTriangles.reserve(triangleCount);

	bool bTan(true), bBinormal(true);
	if (mesh->GetElementBinormalCount() != 1) {
		std::cout << "This mesh has no binormal value" << std::endl;
		bBinormal = false;
	}
	if (mesh->GetElementTangentCount() == 0) {
		std::cout << "This mesh has no tangent value" << std::endl;
		bTan = false;
	}
	

	for (UINT i = 0; i < triangleCount; ++i)
	{
	
		Triangle currTriangle;
		mTriangles.push_back(currTriangle);

		for (UINT j = 0; j < 3; ++j)
		{
			XMFLOAT3 normal;
			XMFLOAT4 tangent;
			XMFLOAT2 UV;
			ZeroMemory(&tangent, sizeof(XMFLOAT4));

			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];


			ReadNormal(mesh, ctrlPointIndex, vertexCounter, normal);
			if (bTan) ReadTangent(mesh, ctrlPointIndex, vertexCounter, tangent);
			for (int k = 0; k < 1; ++k)		// Set UV depend on Diffuse map
			{
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), k, UV);
			}

			Vertex::Skinned temp;
			temp.Position = currCtrlPoint->Position;
			temp.Normal = normal;
			temp.TangentU = tangent;
			temp.Tex = UV;
			// Copy the blending info from each control point
			for (UINT i = 0; i < currCtrlPoint->BlendingInfo.size(); ++i)
			{
				Vertex::VertexBlendingInfo currBlendingInfo;
				currBlendingInfo.BlendingIndex = currCtrlPoint->BlendingInfo[i].mBlendingIndex;
				currBlendingInfo.BlendingWeight = currCtrlPoint->BlendingInfo[i].mBlendingWeight;
				temp.VertexBlendingInfos.push_back(currBlendingInfo);
			}
			// Sort the blending info so that later we can remove
			// duplicated vertices
			temp.SortBlendingInfoByWeight();

			mVertices.push_back(temp);
			mTriangles.back().mIndices.push_back(vertexCounter);
			++vertexCounter;
		}
	}

	// Now mControlPoints has served its purpose
	// We can free its memory
	for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		delete itr->second;
	}
	mControlPoints.clear();
}

void FBXExporter::ProcessControlPoints(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();
	UINT ctrlPointCount = mesh->GetControlPointsCount();
	for(UINT i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* currCtrlPoint = new CtrlPoint();
		XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->Position = currPosition;
		mControlPoints[i] = currCtrlPoint;
	}
}


void FBXExporter::ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV)
{
	if (inUVLayer >= 2 || inMesh->GetElementUVCount() <= inUVLayer)
	{
		std::cout << "Invalid UV Layer Number" << std::endl;
	}
	FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(inUVLayer);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
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
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}
}

void FBXExporter::ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	FbxGeometryElementNormal* vertexNormal = inMesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
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

void FBXExporter::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
{
	FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
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

void FBXExporter::ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT4& outTangent)
{
	FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
	switch (vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
			outTangent.w = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
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

#pragma endregion : Parsing Mesh Data

#pragma region Animation
bool FBXExporter::HasAnimation()
{
	// 현재 원테이크만 지원하므로 '0'
	mAnimationStack = mScene->GetSrcObject<FbxAnimStack>(0);
	FbxString animationName = mAnimationStack->GetName();
	if (animationName.IsEmpty() == false)
	{
		mAnimationName = animationName.Buffer();
		return true;
	}

	return false;

}
void FBXExporter::ReadBoneHierachy()
{
	FbxNode* rootNode = mScene->GetRootNode();
	for (int childIndex = 0; childIndex < rootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* childBone = rootNode->GetChild(childIndex);
		ReadBoneHierachy(childBone, 0, 0, -1);
	}
}

void FBXExporter::ReadBoneHierachy(FbxNode* node, int inDepth, int myIndex, int inParentIndex)
{
	if (node->GetNodeAttribute() &&  node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone bone;
		bone.ParentIndex = inParentIndex;
		bone.Name = node->GetName();
		mBones.push_back(bone);

		//ReadKeyframes(node, myIndex);
	}
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		ReadBoneHierachy(node->GetChild(i), inDepth + 1, mBones.size(), myIndex);
	}
}

void FBXExporter::ProcessAnimations(FbxNode* node)
{
	FbxSkin* skin = reinterpret_cast<FbxSkin*>(node->GetGeometry()->GetDeformer(0, FbxDeformer::eSkin));
	FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(node);

	for (UINT i = 0; i < skin->GetClusterCount(); ++i)
	{
		FbxCluster* cluster		= skin->GetCluster(i);
		FbxNode*	bone		= cluster->GetLink();
		std::string boneName	= bone->GetName();
		UINT		boneIndex	= FindBoneIndexUsingName(boneName);

		ReadBoneOffset(cluster, boneIndex, geometryTransform);
		ReadBlendingIndexWeightPair(cluster, boneIndex);
		ReadKeyframes(bone, boneIndex);
	}
		
}

UINT FBXExporter::FindBoneIndexUsingName(const std::string& inBoneName)
{
	for(UINT i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].Name == inBoneName)
		{
			return i;
		}
	}
	std::cout << "Skeleton information in FBX file is corrupted." << std::endl;
}

void FBXExporter::ReadBoneOffset(FbxCluster* cluster, UINT boneIndex, FbxAMatrix geoMatrix)
{
	FbxAMatrix transformMatrix;
	FbxAMatrix transformLinkMatrix;
	FbxAMatrix globalBindposeInverseMatrix;

	// The transformation of the mesh at binding time
	cluster->GetTransformMatrix(transformMatrix);
	cluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(Bone) at binding time from Bone space to world space
	globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geoMatrix;

	// Update the information in mSkeleton 
	mBones[boneIndex].GlobalBindposeInverse = globalBindposeInverseMatrix;
}
void FBXExporter::ReadBlendingIndexWeightPair(FbxCluster* cluster, UINT boneIndex)
{
	// Associate each Bone with the control points it affects
	UINT numBlendingIndices = cluster->GetControlPointIndicesCount();
	for (UINT i = 0; i < numBlendingIndices; ++i)
	{
		BlendingIndexWeightPair blendingIndexWeightPair;
		blendingIndexWeightPair.mBlendingIndex = boneIndex;
		blendingIndexWeightPair.mBlendingWeight = cluster->GetControlPointWeights()[i];
		mControlPoints[cluster->GetControlPointIndices()[i]]->BlendingInfo.push_back(blendingIndexWeightPair);
	}

	// Some of the control points only have less than 4 Bones affecting them.
	// For a normal renderer, there are usually 4 Bones
	// I am adding more dummy Bones if there isn't enough
	BlendingIndexWeightPair blendingIndexWeightPair;
	blendingIndexWeightPair.mBlendingIndex = 0;
	blendingIndexWeightPair.mBlendingWeight = 0;
	for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		for (UINT i = itr->second->BlendingInfo.size(); i <= 4; ++i)
		{
			itr->second->BlendingInfo.push_back(blendingIndexWeightPair);
		}
	}
}
void FBXExporter::ReadKeyframes(FbxNode * bone, UINT boneIndex)
{
	FbxString animStackName = mAnimationStack->GetName();
	UINT layerCount = mAnimationStack->GetMemberCount<FbxAnimLayer>();

	for (UINT i = 0; i < layerCount; ++i)
	{
		FbxAnimLayer* animLayer = mAnimationStack->GetMember<FbxAnimLayer>(i);

		//get the animation layers
		const FbxAnimCurve* translationLayer = bone->LclTranslation.GetCurve(animLayer, false);
		const FbxAnimCurve* rotationLayer = bone->LclRotation.GetCurve(animLayer, false);
		const FbxAnimCurve* scalingLayer = bone->LclScaling.GetCurve(animLayer, false);


		FbxTakeInfo* takeInfo = mScene->GetTakeInfo(animStackName);
		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
		UINT animationLength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;

		std::map<float, FbxTime> keyTimes;
		const auto setKeyTimes = [&keyTimes](const FbxAnimCurve* pCurve)
		{
			UINT numKeys = pCurve ? pCurve->KeyGetCount() : 0;
			for (UINT k = 0; k < numKeys; ++k)
			{
				FbxTime time = pCurve->KeyGet(k).GetTime();
				time.SetFrame(k, FbxTime::eFrames30);
				const float timeMS = time.GetMilliSeconds();
				keyTimes[timeMS] = time;
			}
		};

		setKeyTimes(translationLayer);
		setKeyTimes(rotationLayer);
		setKeyTimes(scalingLayer);

		FbxAnimEvaluator* evaluator = mScene->GetAnimationEvaluator();
		for (auto iter : keyTimes)
		{
			Keyframe keyframe;
			keyframe.TimePos = iter.second.GetMilliSeconds();

			keyframe.Translation.x = evaluator->GetNodeLocalTranslation(bone, iter.second).mData[0];
			keyframe.Translation.y = evaluator->GetNodeLocalTranslation(bone, iter.second).mData[1];
			keyframe.Translation.z = evaluator->GetNodeLocalTranslation(bone, iter.second).mData[2];

			keyframe.Scale.x = evaluator->GetNodeLocalScaling(bone, iter.second).mData[0];
			keyframe.Scale.y = evaluator->GetNodeLocalScaling(bone, iter.second).mData[1];
			keyframe.Scale.z = evaluator->GetNodeLocalScaling(bone, iter.second).mData[2];

			XMFLOAT3 rot;
			rot.x = evaluator->GetNodeLocalRotation(bone, iter.second).mData[0];
			rot.y = evaluator->GetNodeLocalRotation(bone, iter.second).mData[1];
			rot.z = evaluator->GetNodeLocalRotation(bone, iter.second).mData[2];

			XMFLOAT4 f4Quat;
			XMVECTOR vQuat = XMQuaternionRotationRollPitchYaw(rot.x, rot.y, rot.z);
			XMStoreFloat4(&f4Quat, vQuat);

			keyframe.RotationQuat = f4Quat;

			mBones[boneIndex].KeyFrames.push_back(keyframe);
		}
	}
}
#pragma endregion : Parsing Animation Data

#pragma region Material
void FBXExporter::ProcessMaterials(FbxNode* node)
{
	UINT materialCount = node->GetMaterialCount();

	for (UINT i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);
		UINT uniqueMatID = surfaceMaterial->GetUniqueID();

		ProcessMaterialAttribute(surfaceMaterial, uniqueMatID);
		ProcessMaterialTexture(surfaceMaterial, mMaterialLookUp[uniqueMatID]);
	}
}

void FBXExporter::AssociateMaterialToMesh(FbxNode* node)
{
	FbxLayerElementArrayTemplate<int>* materialIndices;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	FbxMesh* mesh = node->GetMesh();

	if(mesh->GetElementMaterial())
	{
		materialIndices = &(mesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = mesh->GetElementMaterial()->GetMappingMode();

		if(materialIndices)
		{
			switch(materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (materialIndices->GetCount() == mTriangles.size())
				{
					for (UINT i = 0; i < mTriangles.size(); ++i)
					{
						UINT materialIndex = materialIndices->GetAt(i);
						mTriangles[i].mMaterialIndex = materialIndex;
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				UINT materialIndex = materialIndices->GetAt(0);
				for (UINT i = 0; i < mTriangles.size(); ++i)
				{
					mTriangles[i].mMaterialIndex = materialIndex;
				}
			}
			break;

			default:
				std::cout<< "Invalid mapping mode for material\n" << std::endl;
			}
		}
	}
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

		mMaterialLookUp[inMaterialIndex] = currMaterial;
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

		mMaterialLookUp[inMaterialIndex] = currMaterial;
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
							else if (textureType == "Bump")
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
#pragma endregion : Parsing Material Data

#pragma region Optimize
void FBXExporter::Optimize()
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
			mTriangles[i].mIndices[j] = FindVertex(mVertices[i * 3 + j], uniqueVertices);
		}
	}

	mVertices.clear();
	mVertices = uniqueVertices;
	uniqueVertices.clear();

	// Now we sort the triangles by materials to reduce 
	// shader's workload
	std::sort(mTriangles.begin(), mTriangles.end());

	// Make SubsetTable from mSubsetMap
	UINT count(0);
	std::unordered_map<UINT, UINT> material_TriangleCount;

	for (auto iterT : mTriangles)
		++material_TriangleCount[iterT.mMaterialIndex];

	mSubsets.resize(mMaterialLookUp.size());
	

	for (UINT i = 0; i < mSubsets.size(); ++i)
	{
		mSubsets[i].ID = i;
		mSubsets[i].FaceCount = material_TriangleCount[i];
		if (i + 1<mSubsets.size())
			mSubsets[i + 1].FaceStart = mSubsets[i].FaceStart + mSubsets[i].FaceCount;
	}

	// If model doesn't have normal map
	for (auto iterM : mMaterialLookUp)
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
#pragma endregion  : Final Procedure Of Exporting

#pragma region File
void FBXExporter::WriteMesh(std::ostream& fout)
{
	using namespace std;
	if (mHasAnimation)
	{
		fout << "***************KPU-File-Header***************" << endl;
		fout << "#Materials: "  << mMaterialLookUp.size() << endl;
		fout << "#Vertices: "   << mVertices.size() << endl;
		fout << "#Triangles: "  << mTriangles.size() << endl;
		fout << "#Bones: "		<< mBones.size() << endl;
		fout << "#AnimationClips: " << 1 << endl << endl;	// 현재는  원 테이크만 지원
	}
	else
	{
		fout << "***************KPU-File-Header***************" << endl;
		fout << "#Materials: "	<< mMaterialLookUp.size() << endl;
		fout << "#Vertices: "	<< mVertices.size() << endl;
		fout << "#Triangles: "  << mTriangles.size() << endl;
		fout << "#Bones: "		<< 0 << endl;		// 이 정보는 필요없다. 모델이 애니메이션 없으면
		fout << "#AnimationClips: " << 0 << endl << endl;
	}

	fout << "***************Materials*********************" << endl;
	for (auto iterM : mMaterialLookUp)
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
		fout << mTriangles[i].mIndices[0] << " " << mTriangles[i].mIndices[1] << " " << mTriangles[i].mIndices[2] << endl;

	fout << endl;
}

void FBXExporter::WriteAnimation(std::ostream& fout)
{
	using namespace std;
	fout << "***************BoneOffsets*******************" << endl;
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		fout << "BoneOffset" << i << " ";
	/*	FbxVector4 translation = mBones[i].GlobalBindposeInverse.GetT();
		FbxVector4 rotation = mBones[i].GlobalBindposeInverse.GetR();
		translation.Set(translation.mData[0], translation.mData[1], translation.mData[2]);
		rotation.Set(rotation.mData[0], rotation.mData[1], rotation.mData[2]);
		mBones[i].GlobalBindposeInverse.SetT(translation);
		mBones[i].GlobalBindposeInverse.SetR(rotation);*/
		FbxMatrix out = mBones[i].GlobalBindposeInverse;

		Utilities::WriteMatrix(fout, out, true);		// 값이 이상하게 나오면 체크해볼 것
	}

	fout << endl;
	fout << "***************BoneHierarchy*****************" << endl;
	for (UINT i = 0; i < mBones.size(); ++i)
		fout << "ParentIndexOfBone" << i << " " << mBones[i].ParentIndex << endl;

	fout << endl;
	fout << "***************AnimationClips****************" << endl;
	fout << "AnimationClip " << mAnimationName.erase(mAnimationName.find(" "), 1) << endl;
	fout << "{" << endl;
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		fout << "\tBone" << i << " #KeyFrames: " << mBones[i].KeyFrames.size() << endl;
		fout << "\t{" << endl;
		for (auto iterK : mBones[i].KeyFrames)
		{
			fout << "\t\t";
			fout << "Time: "	<< iterK.TimePos / 1000		<< " ";
			fout << "Pos: "		<< iterK.Translation		<< " ";
			fout << "Scale: "	<< iterK.Scale				<< " ";
			fout << "Quat: "	<< iterK.RotationQuat		<< endl;
		}
		fout << "\t}" << endl << endl;
	}
	fout << "}" << endl;


	/*fout << "\t\t<animation name='" << mAnimationName << "' length='" << mAnimationLength << "'>\n";
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		fout << "\t\t\t" << "<track id = '" << i << "' name='" << mBones[i].mName << "'>\n";
		Keyframe* walker = mBones[i].mAnimation;
		while (walker)
		{
			fout << "\t\t\t\t" << "<frame num='" << walker->mFrameNum - 1 << "'>\n";
			fout << "\t\t\t\t\t";
			FbxVector4 translation = walker->mGlobalTransform.GetT();
			FbxVector4 rotation = walker->mGlobalTransform.GetR();
			translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
			rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
			walker->mGlobalTransform.SetT(translation);
			walker->mGlobalTransform.SetR(rotation);
			FbxMatrix out = walker->mGlobalTransform;
			Utilities::WriteMatrix(fout, out.Transpose(), true);
			fout << "\t\t\t\t" << "</frame>\n";
			walker = walker->mNext;
		}
		fout << "\t\t\t" << "</track>\n";
	}*/
}
#pragma endregion : File Exporting