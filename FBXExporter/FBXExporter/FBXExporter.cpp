#include "FBXExporter.h"

#pragma region Constructor
FBXExporter::FBXExporter() : mFbxMgr(nullptr), mScene(nullptr), 
mHasAnimation(true), mClipNum(0), mExportingCount(0),
mPrevCtrlPointCount(0), mPrevTriangleCount(0), mMeshCount(0)
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

void FBXExporter::Export(const std::string& fileName, const std::string& clipName, std::ofstream& fout)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;

	++mExportingCount;
	mClipName = clipName;

	if (!mScene->GetCurrentAnimationStack())
		mHasAnimation = false;

	std::cout << "Exporting to... " << fileName << "\n\n";

	if (mHasAnimation)
	{
		QueryPerformanceCounter(&start);
		ProcessBoneHierachy(mScene->GetRootNode());
		QueryPerformanceCounter(&end);
		std::cout << "Processing BoneHierachy: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
	}

	QueryPerformanceCounter(&start);
	ProcessGeometry(mScene->GetRootNode());
	QueryPerformanceCounter(&end);
	std::cout << "Processing Geometry: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
	
	if (mHasAnimation)
	{
		QueryPerformanceCounter(&start);
		ReadKeyframes();
		QueryPerformanceCounter(&end);
		std::cout << "Processing Keyframes: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
	}

	if (mExportingCount == 1)
	{
		QueryPerformanceCounter(&start);
		FinalProcedure();
		QueryPerformanceCounter(&end);
		std::cout << "FinalProcedure: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
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
			ProcessMesh(mesh);
			ProcessMaterials(node);
			
			if (mHasAnimation)
				ReadSkinnedData(mesh);

			mPrevCtrlPointCount += mesh->GetControlPointsCount();
			mPrevTriangleCount += mesh->GetPolygonCount();
			++mMeshCount;

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
	assert(mesh->GenerateTangentsDataForAllUVSets());

	int polyVertex = 0;
	UINT currtriangleCount = mesh->GetPolygonCount();
	mTriangles.reserve(mPrevTriangleCount + currtriangleCount);
	for (UINT i = 0; i < currtriangleCount; ++i)
	{
		Triangle triangle;

		for (UINT j = 0; j < 3; ++j)
		{
			UINT ctrlPointIndex = mPrevCtrlPointCount + mesh->GetPolygonVertex(i, j);
			mControlPoints[ctrlPointIndex].PolygonVertex.push_back(XMFLOAT2(mPrevTriangleCount + i,j));
			mIndices.push_back(ctrlPointIndex);
			ReadNormal(mesh, ctrlPointIndex, polyVertex, triangle.Data[j]);
			ReadTangent(mesh, ctrlPointIndex, polyVertex, triangle.Data[j]);
			ReadUV(mesh, i, j, triangle.Data[j]);
	 
			++polyVertex;
		}

		mTriangles.push_back(triangle);
	}
}


void FBXExporter::ReadUV(FbxMesh* mesh, const UINT triangleIndex, const UINT positionInTriangle, Triangle::SurfaceData& data)
{
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);
	UINT ctrlPointIndex = mesh->GetPolygonVertex(triangleIndex, positionInTriangle);
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

	data.UV = uv;

	//std::vector<UVInfo>& uvInfos = mControlPoints[mPrevCtrlPointCount + ctrlPointIndex].UVInfos;
	//// 추가되는 UV 같은 값이라도 삼각형이 다르면 그 정보 추가해줘야 함.
	//auto &iter = std::find(uvInfos.begin(), uvInfos.end(), uvInfo);
	//if (iter == uvInfos.end())	// uv같은 값이 없다면
	//	uvInfos.push_back(uvInfo);
	//else
	//{
	//	// 있다면 삼각형 정보 추가
	//	iter->TriangleIndex.push_back(mPrevTriangleCount + triangleIndex);
	//	iter->PositionInTriangle.push_back(positionInTriangle);
	//}
	
}

void FBXExporter::ReadNormal(FbxMesh* mesh, int ctrlPointIndex, int indexCounter, Triangle::SurfaceData& data)
{
	FbxGeometryElementNormal* normal = mesh->GetElementNormal(0);

	XMFLOAT3 outNormal;
	switch (normal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (normal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(normal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(normal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(normal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = normal->GetIndexArray().GetAt(ctrlPointIndex);
			outNormal.x = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (normal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(normal->GetDirectArray().GetAt(indexCounter).mData[0]);
			outNormal.y = static_cast<float>(normal->GetDirectArray().GetAt(indexCounter).mData[1]);
			outNormal.z = static_cast<float>(normal->GetDirectArray().GetAt(indexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = normal->GetIndexArray().GetAt(indexCounter);
			outNormal.x = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(normal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}

	data.Normal = outNormal;
}

void FBXExporter::ReadTangent(FbxMesh * mesh, int ctrlPointIndex, int indexCounter, Triangle::SurfaceData & data)
{
	FbxGeometryElementTangent* tangent = mesh->GetElementTangent(0);

	XMFLOAT4 outTangent;
	switch (tangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (tangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
			outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
			outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
			outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(ctrlPointIndex).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = tangent->GetIndexArray().GetAt(ctrlPointIndex);
			outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[2]);
			outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[3]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (tangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(indexCounter).mData[0]);
			outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(indexCounter).mData[1]);
			outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(indexCounter).mData[2]);
			outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(indexCounter).mData[3]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = tangent->GetIndexArray().GetAt(indexCounter);
			outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[2]);
			outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[3]);
		}
		break;

		default:
			std::cout << "Invalid Reference" << std::endl;
		}
		break;
	}

	data.Tangent = outTangent;
}

#pragma endregion : Abount extracting geometry data

#pragma region Animation
void FBXExporter::ProcessBoneHierachy(FbxNode * rootNode)
{
	// First get the all bones
	for (int childIndex = 0; childIndex < rootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* childNode = rootNode->GetChild(childIndex);
		GetAllBones(childNode);
	}

	// Update parentIndex using node's parent
	for (auto& iterB : mBones)
	{
		std::string parentName = iterB.Node->GetParent()->GetName();
		int parentIndex = FindBoneIndexUsingName(parentName);
		iterB.ParentIndex = parentIndex;
	}

	// Update order
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].ParentIndex >= i &&
			mBones[i].ParentIndex > 0)
		{
			if (i + 1 < mBones.size())
			{
				Bone temp = mBones[i];
				mBones[i] = mBones[i + 1];
				mBones[i + 1] = temp;
			}
		}
	}

	// Fix the wrong data
	for (UINT i = 1; i < mBones.size(); ++i)
	{
		// For model that has several meshes(weapons, sheild, etc..)
		if (mBones[i].ParentIndex == -1)
			mBones[i].ParentIndex = 0;
	}
}

void FBXExporter::GetAllBones(FbxNode* node)
{
	if (node->GetNodeAttribute())
	{
		FbxNodeAttribute::EType type = node->GetNodeAttribute()->GetAttributeType();
		switch (type)
		{
		case FbxNodeAttribute::eSkeleton:
			Bone bone;
			bone.Name = node->GetName();
			bone.Node = node;

			mBones.push_back(bone);
			
			UINT childCount = node->GetChildCount();
			for (int i = 0; i < childCount; i++)
				GetAllBones(node->GetChild(i));
			break;
		}
	}

}

void FBXExporter::ReadSkinnedData(FbxMesh* mesh)
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
			mBones[boneIndex].Node = bone;
			mBones[boneIndex].Name = boneName;

			// Associate each Bone with the control points it affects
			UINT numBlendingIndices = cluster->GetControlPointIndicesCount();
			for (UINT i = 0; i < numBlendingIndices; ++i)
			{
				UINT ind = cluster->GetControlPointIndices()[i];
				BlendingIndexWeightPair blendingIndexWeightPair;
				blendingIndexWeightPair.mBlendingIndex = boneIndex;
				blendingIndexWeightPair.mBlendingWeight = cluster->GetControlPointWeights()[i];
				mControlPoints[mPrevCtrlPointCount + ind].BlendingInfo.push_back(blendingIndexWeightPair);
			}
		}
	}
}

void FBXExporter::RemoveUnskinnedBones()
{
	for (UINT i = 1; i < mBones.size();)
	{
		if (mBones[i].BoneOffset.IsIdentity())
			mBones.erase(mBones.begin() + i);
		else
			++i;
	}
}

void FBXExporter::ReadKeyframes()
{
	FbxAnimStack* currAnimStack = mScene->GetSrcObject<FbxAnimStack>(0);
	FbxTakeInfo* takeInfo = mScene->GetTakeInfo(currAnimStack->GetName());
	FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

	int startFrameCount = start.GetFrameCount(FbxTime::eFrames24);
	int endFrameCount = end.GetFrameCount(FbxTime::eFrames24);
	float animationLength = (end.GetMilliSeconds() - start.GetMilliSeconds());

	for (auto& iterB : mBones)
	{
		for (int t = startFrameCount; t <= endFrameCount; ++t)
		{
			float timePos = ((float)t / endFrameCount - startFrameCount) * animationLength;

			FbxTime currTime;
			currTime.SetFrame(t, FbxTime::eFrames24);
			FbxAMatrix localTransform;
			localTransform	= iterB.Node->EvaluateLocalTransform(currTime);
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

			iterB.Keyframes.push_back(keyframe);
		}
	}
}

int FBXExporter::FindBoneIndexUsingName(const std::string& boneName)
{
	for (UINT i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].Name == boneName)
		{
			return i;
		}
	}

	return -1;
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
}

void FBXExporter::ProcessMaterialAttribute(FbxSurfaceMaterial* material, UINT materialIndex)
{
	FbxDouble3 double3;
	FbxDouble double1;
	if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		Material* currMaterial = new Material();

		// Amibent Color
		double3 = FbxCast<FbxSurfacePhong>(material)->Ambient;
		currMaterial->Ambient.x = static_cast<float>(double3.mData[0]);
		currMaterial->Ambient.y = static_cast<float>(double3.mData[1]);
		currMaterial->Ambient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = FbxCast<FbxSurfacePhong>(material)->Diffuse;
		currMaterial->Diffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->Diffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->Diffuse.z = static_cast<float>(double3.mData[2]);

		// Specular Color
		double3 = FbxCast<FbxSurfacePhong>(material)->Specular;
		currMaterial->Specular.x = static_cast<float>(double3.mData[0]);
		currMaterial->Specular.y = static_cast<float>(double3.mData[1]);
		currMaterial->Specular.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = FbxCast<FbxSurfacePhong>(material)->Emissive;
		currMaterial->Emissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->Emissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->Emissive.z = static_cast<float>(double3.mData[2]);

		// Reflection
		double3 = FbxCast<FbxSurfacePhong>(material)->Reflection;
		currMaterial->Reflection.x = static_cast<float>(double3.mData[0]);
		currMaterial->Reflection.y = static_cast<float>(double3.mData[1]);
		currMaterial->Reflection.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = FbxCast<FbxSurfacePhong>(material)->TransparencyFactor;
		currMaterial->TransparencyFactor = double1;

		// Shininess
		double1 = FbxCast<FbxSurfacePhong>(material)->Shininess;
		currMaterial->Shininess = double1;

		// Specular Factor
		double1 = FbxCast<FbxSurfacePhong>(material)->SpecularFactor;
		currMaterial->SpecularPower = double1;


		// Reflection Factor
		double1 = FbxCast<FbxSurfacePhong>(material)->ReflectionFactor;
		currMaterial->ReflectionFactor = double1;

		mMaterials[materialIndex] = currMaterial;
	}
	else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		Material* currMaterial = new Material();
		FbxSurfaceLambert* lambert = FbxCast<FbxSurfaceLambert>(material);

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

		mMaterials[materialIndex] = currMaterial;
	}
}

void FBXExporter::ProcessMaterialTexture(FbxSurfaceMaterial * material, Material * outMaterial)
{
	UINT textureIndex = 0;
	FbxProperty property;

	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = material->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
		if (property.IsValid())
		{
			UINT textureCount = property.GetSrcObjectCount<FbxTexture>();
			for (UINT i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if (layeredTexture)
				{
					std::string textureType = property.GetNameAsCStr();
					FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(layeredTexture);

					if (fileTexture)
					{
						if (textureType == "DiffuseColor")
						{
							outMaterial->DiffuseMapName = fileTexture->GetRelativeFileName();
						}
						else if (textureType == "SpecularColor")
						{
							outMaterial->SpecularMapName = fileTexture->GetRelativeFileName();
						}
						else if (textureType == "NormalMap")
						{
							outMaterial->NormalMapName = fileTexture->GetRelativeFileName();
						}
					}
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
								outMaterial->DiffuseMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "SpecularColor")
							{
								outMaterial->SpecularMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "NormalMap")
							{
								outMaterial->NormalMapName = fileTexture->GetRelativeFileName();
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
	// Some of the control points only have less than 4 Bones affecting them.
	// For a normal renderer, there are usually 4 Bones
	// I am adding more dummy Bones if there isn't enough
	BlendingIndexWeightPair blendingIndexWeightPair;
	blendingIndexWeightPair.mBlendingIndex = 0;
	blendingIndexWeightPair.mBlendingWeight = 0;
	for (auto& iterC : mControlPoints)
	{
		for (UINT j = iterC.second.BlendingInfo.size(); j < 4; ++j)
			iterC.second.BlendingInfo.push_back(blendingIndexWeightPair);
	}

	// Caculate Binormal by looping triangles
	/*for (UINT i = 0; i < mTriangles.size(); ++i)
	//{
	//	XMFLOAT3 vertex0, vertex1, vertex2;
	//	XMVECTOR e0, e1;
	//	vertex0 = mControlPoints[mIndices[i * 3]].Position;
	//	vertex1 = mControlPoints[mIndices[i * 3 + 1]].Position;
	//	vertex2 = mControlPoints[mIndices[i * 3 + 2]].Position;

	//	e0 = XMLoadFloat3(&(vertex1 - vertex0));
	//	e1 = XMLoadFloat3(&(vertex2 - vertex0));

	//	XMFLOAT2 uv0, uv1, uv2;
	//	XMVECTOR d_uv0, d_uv1;
	//	uv0 = mTriangles[i].Data[0].UV;
	//	uv1 = mTriangles[i].Data[1].UV;
	//	uv2 = mTriangles[i].Data[2].UV;

	//	d_uv0 = XMLoadFloat2(&(uv1 - uv0));
	//	d_uv1 = XMLoadFloat2(&(uv2 - uv0));

	//	XMVECTOR a = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	//	XMVECTOR b = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	//	XMMATRIX UV(d_uv0, d_uv1, a, b);
	//	XMMATRIX E(e0, e1, a, b);

	//	XMVECTOR det;
	//	XMMATRIX result = XMMatrixInverse(&det, UV)*E;
	//	if (XMVectorGetX(det) == 0.0f)
	//		result = XMMatrixIdentity();

	//	XMFLOAT3 tangent = XMFLOAT3(result(0, 0), result(1, 0), result(2, 0));
	//	XMFLOAT3 binormal = XMFLOAT3(result(0, 1), result(1, 1), result(2, 1));
	//	for (UINT j = 0; j < 3; ++j) {
	//		mTriangles[i].Data[j].Tangent = Float3Normalize(tangent);
	//		mTriangles[i].Data[j].Binormal = Float3Normalize(binormal);
	//	}
	} */

	UINT ctrlPointSize = mControlPoints.size();
	mVertices.resize(ctrlPointSize);

	for (UINT i = 0; i < ctrlPointSize; ++i)
	{
		CtrlPoint ctrlPoint = mControlPoints[i];
		mVertices[i].Position = ctrlPoint.Position;

		// Copy the blending info from each control point
		for (UINT j = 0; j < ctrlPoint.BlendingInfo.size(); ++j)
		{
			Vertex::VertexBlendingInfo blendingInfo;
			blendingInfo.BlendingIndex = ctrlPoint.BlendingInfo[j].mBlendingIndex;
			blendingInfo.BlendingWeight = ctrlPoint.BlendingInfo[j].mBlendingWeight;
			mVertices[i].VertexBlendingInfos.push_back(blendingInfo);
		}

		XMFLOAT3 normal(0.0f, 0.0f, 0.0f);
		//XMFLOAT3 binormal(0.0f, 0.0f, 0.0f);
		XMFLOAT4 tangent(0.0f, 0.0f, 0.0f, 0.0f);
		for (UINT j = 0; j < ctrlPoint.PolygonVertex.size(); ++j)
		{
			UINT t = ctrlPoint.PolygonVertex[j].x;
			UINT p = ctrlPoint.PolygonVertex[j].y;
			UINT index = t * 3 + p;
			
			normal = normal + mTriangles[t].Data[p].Normal;
			// binormal = binormal + mTriangles[t].Data[p].Binormal;
			tangent = tangent + mTriangles[t].Data[p].Tangent;
		}

		normal = Float3Normalize(normal);
		// binormal = Float3Normalize(binormal);
		tangent = Float4Normalize(tangent);

		XMVECTOR v0, v1, v2;
		XMVECTOR w0, w1, w2;
		XMVECTOR projW0V1, projW0V2, projW1V2, prep;

		v0 = XMLoadFloat3(&normal);
		v1 = XMLoadFloat4(&tangent);
		//v2 = XMLoadFloat3(&binormal);

		w0 = v0;
		XMVector3ComponentsFromNormal(&projW0V1, &prep, v1, w0);

		w1 = v1 - projW0V1;
		//XMVector3ComponentsFromNormal(&projW0V2, &prep, v2, w0);
		//XMVector3ComponentsFromNormal(&projW1V2, &prep, v2, w1);

		//w2 = v2 - projW0V2 - projW1V2;

		XMVector3Normalize(w0);
		XMVector3Normalize(w1);
		//XMVector3Normalize(w2);

		XMStoreFloat3(&mVertices[i].Normal, w0);

		XMStoreFloat4(&tangent, w1);
		//int handedness = XMVectorGetX(XMVector3Dot(XMVector3Cross(w0, w2), w1)) < 0 ? -1 : 1;

		mVertices[i].Tangent = tangent;

		// Push back new vertex If it has different UV
		// Other information is same.
		for (UINT j = 0; j < ctrlPoint.PolygonVertex.size(); ++j)
		{
			UINT t = ctrlPoint.PolygonVertex[j].x;
			UINT p = ctrlPoint.PolygonVertex[j].y;
			UINT index = t * 3 + p;
			XMFLOAT2 uv = mTriangles[t].Data[p].UV;

			if (j == 0)
				mVertices[i].Tex = uv;
			else if (mVertices[i].Tex != uv)
			{
				bool generateVertex = true;
				for (int k = j - 1; k > 0; --k)
				{
					UINT prev_t = ctrlPoint.PolygonVertex[k].x;
					UINT prev_p = ctrlPoint.PolygonVertex[k].y;
					XMFLOAT2 prev_uv = mTriangles[prev_t].Data[prev_p].UV;
					if (uv == prev_uv)
					{
						generateVertex = false;
						mIndices[index] = mIndices[prev_t * 3 + prev_p];
						k = 0; // break the for loop
					}
				}
				if (generateVertex)
				{
					mIndices[index] = mVertices.size();

					Vertex::Skinned vertex;
					vertex.Position = mVertices[i].Position;
					vertex.Normal = mVertices[i].Normal;
					vertex.Tangent = mVertices[i].Tangent;
					vertex.VertexBlendingInfos = mVertices[i].VertexBlendingInfos;

					vertex.Tex = uv;
					mVertices.push_back(vertex);
				}
			}
		}
	}

	// Calculate tangent by cross the normal and binormal
	//for (auto& iterC : mControlPoints)
	//{
	//	// Get the tangent value to cross normal and Binormal
	//	XMFLOAT3 normal, tangent, binormal;
	//	for (auto iterP : iterC.second.PolygonVertex)
	//	{
	//		UINT t = iterP.x;
	//		UINT p = iterP.y;
	//		normal = mTriangles[t].Data[p].Normal;
	//		binormal = mTriangles[t].Data[p].Binormal;

	//		XMVECTOR vBi = XMVector3Normalize(XMLoadFloat3(&binormal));
	//		XMVECTOR w0 = XMVector3Normalize(XMLoadFloat3(&normal));
	//		XMVECTOR w2 = XMVector3Normalize(XMVector3Cross(w0, vBi));
	//		XMVECTOR vTan = XMVector3Cross(w2, w0);
	//		XMStoreFloat3(&tangentU, vTan);

	//		UINT index = t * 3 + p;
	//		mVertices[mIndices[index]].Normals.push_back(normal);

	//		XMFLOAT3 tangent;
	//		tangent.x = isnan(tangentU.x) ? 0.0f : tangentU.x;
	//		tangent.y = isnan(tangentU.y) ? 0.0f : tangentU.y;
	//		tangent.z = isnan(tangentU.z) ? 0.0f : tangentU.z;
	//		mVertices[mIndices[index]].Tangents.push_back(tangent);
	//	}
	//}

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
	for (auto& iterM : mMaterials)
	{
		if (iterM.second->DiffuseMapName == "")
			iterM.second->DiffuseMapName = "None";
		else
		{
			size_t t = iterM.second->DiffuseMapName.find("\\", 1);
			while (t < iterM.second->DiffuseMapName.size()) {
				iterM.second->DiffuseMapName.erase(0, t+1);
				t = iterM.second->DiffuseMapName.find("\\", 1);
			}
		}

		if (iterM.second->NormalMapName == "")
			iterM.second->NormalMapName = "Cyclop_Normals.png";
		else
		{
			size_t t = iterM.second->NormalMapName.find("\\", 1);
			if (t < iterM.second->NormalMapName.size())
				iterM.second->NormalMapName.erase(0, t+1);
		}

		if (iterM.second->SpecularPower == 0.0f)
			iterM.second->SpecularPower = 1.0f;
	}
}

#pragma endregion  : Final procedure to export y2k file format.

#pragma region File
void FBXExporter::WriteMesh(std::ostream& fout)
{
	using namespace std;
	if (mHasAnimation)
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

	fout << "***************Vertices**********************" << endl;
	for (auto vertex : mVertices)
	{
		// Remeber this order.
		fout << "Pos: " << vertex.Position << endl;
		fout << "Tangent: " << vertex.Tangent << endl;
		fout << "Normal: " << vertex.Normal << endl;
		fout << "Tex: " << vertex.Tex.x << " " << 1.0f - vertex.Tex.y << endl;
		if (mHasAnimation)
		{
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
	if (mExportingCount == 1)
	{
		fout << "***************BoneOffsets*******************" << endl;
		for (UINT i = 0; i < mBones.size(); ++i)
		{
			fout << "BoneOffset<" << i << ">: ";
			Utilities::WriteMatrix(fout, mBones[i].BoneOffset, true);
		}

		fout << endl;
		fout << "***************BoneHierarchy*****************" << endl;
		for (UINT i = 0; i < mBones.size(); ++i) {
			// replace space to "_"
			std::string boneName = mBones[i].Name;
			size_t t = boneName.find(" ");
			while (t < boneName.size()) {
				boneName.erase(t, 1);
				boneName.insert(t, "_");
				t = boneName.find(" ");
			}
			fout << "ParentIndexOfBone<" << boneName << "> " << mBones[i].ParentIndex << endl;
		}

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