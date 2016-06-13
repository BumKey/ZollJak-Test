#pragma once
#include "MathHelper.h"
#include <vector>
#include <algorithm>

namespace Vertex
{
	struct Basic
	{
		Basic() { ZeroMemory(this, sizeof(Basic)); }

		XMFLOAT3 Position;
		XMFLOAT2 Tex;
	    XMFLOAT4 Tangent;
		XMFLOAT3 Normal;
	};


	struct VertexBlendingInfo
	{
		unsigned int BlendingIndex;
		double BlendingWeight;

		VertexBlendingInfo() :
			BlendingIndex(0),
			BlendingWeight(0.0)
		{}

		bool operator < (const VertexBlendingInfo& rhs)
		{
			return (BlendingWeight > rhs.BlendingWeight);
		}
	};

	struct Skinned : public Basic
	{
		std::vector<VertexBlendingInfo> VertexBlendingInfos;

		void SortBlendingInfoByWeight()
		{
			std::sort(VertexBlendingInfos.begin(), VertexBlendingInfos.end());
		}

		void operator=(const Skinned& rhs)
		{
			Position = rhs.Position;
			Normal = rhs.Normal;
			Tex = rhs.Tex;
			Tangent = rhs.Tangent;
			VertexBlendingInfos = rhs.VertexBlendingInfos;
		}
	};
}