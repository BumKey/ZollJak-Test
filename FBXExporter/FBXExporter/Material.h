#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Vertex.h"

struct Material
{
	Material() { ZeroMemory(this, sizeof(Material)); }

	std::string mName;

	XMFLOAT3 Ambient;
	XMFLOAT3 Diffuse;
	XMFLOAT3 Emissive;
	XMFLOAT3 Specular;
	XMFLOAT3 Reflection;
	double SpecularPower;
	double Shininess;
	double ReflectionFactor;
	double TransparencyFactor;

	std::string DiffuseMapName;
	std::string EmissiveMapName;
	std::string GlossMapName;
	std::string NormalMapName;
	std::string SpecularMapName;
};

//class LambertMaterial : public Material
//{
//public:
//
//	void WriteToStream(std::ostream& inStream)
//	{
//		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
//		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
//		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;
//
//		if (!mDiffuseMapName.empty())
//		{
//			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
//		}
//
//		if (!mNormalMapName.empty())
//		{
//			inStream << "NormalMap: " << mNormalMapName << std::endl;
//		}
//	}
//};

//class PhongMaterial : public Material
//{
//public:
//	XMFLOAT3 mSpecular;
//	XMFLOAT3 mReflection;
//	double mSpecularPower;
//	double mShininess;
//	double mReflectionFactor;
//
//	void WriteToStream(std::ostream& inStream)
//	{
//		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
//		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
//		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;
//		inStream << "Specular: " << mSpecular.x << " " << mSpecular.y << " " << mSpecular.z << std::endl;
//		inStream << "SpecPower: " << mSpecularPower << std::endl;
//		inStream << "Reflectivity: " << mReflection.x << " " << mReflection.y << " " << mReflection.z << std::endl;
//		
//		if (!mDiffuseMapName.empty())
//		{
//			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
//		}
//
//		if(!mNormalMapName.empty())
//		{
//			inStream << "NormalMap: " << mNormalMapName << std::endl;
//		}
//	}
//};