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
		std::vector<XMFLOAT3> Binormals;
		XMFLOAT4 TangentU;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;

		bool operator==(const Basic& rhs) const
		{
			uint32_t position;
			uint32_t normal;
			uint32_t tex;
			uint32_t tan;

			XMVectorEqualR(&position, XMLoadFloat3(&(Position)), XMLoadFloat3(&rhs.Position));
			XMVectorEqualR(&normal, XMLoadFloat3(&(Normal)), XMLoadFloat3(&rhs.Normal));
			XMVectorEqualR(&tex, XMLoadFloat2(&Tex), XMLoadFloat2(&rhs.Tex));
			XMVectorEqualR(&tan, XMLoadFloat4(&TangentU), XMLoadFloat4(&rhs.TangentU));

			return XMComparisonAllTrue(position) && XMComparisonAllTrue(normal) 
				&& XMComparisonAllTrue(tex) && XMComparisonAllTrue(tan);
		}
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

		bool operator==(const Skinned& rhs) const
		{
			bool sameBlendingInfo = true;

			// We only compare the blending info when there is blending info
			if (!(VertexBlendingInfos.empty() && rhs.VertexBlendingInfos.empty()))
			{
				// Each vertex should only have 4 index-weight blending info pairs
				// 한 fbx파일에 본이 있는 메쉬와 아닌 메쉬가 같이 있을 수도 있다.
				for (UINT i = 0; i < 4; ++i)
				{
					if (VertexBlendingInfos[i].BlendingIndex != rhs.VertexBlendingInfos[i].BlendingIndex ||
						abs(VertexBlendingInfos[i].BlendingWeight - rhs.VertexBlendingInfos[i].BlendingWeight) > 0.01)
					{
						sameBlendingInfo = false;
						break;
					}
				}
			}
			bool result = Basic::operator==(static_cast<Basic>(rhs));
			return  result && sameBlendingInfo;
		}

		void operator=(const Skinned& rhs)
		{
			Position = rhs.Position;
			Normal = rhs.Normal;
			Tex = rhs.Tex;
			TangentU = rhs.TangentU;
			VertexBlendingInfos = rhs.VertexBlendingInfos;
		}
	};
}