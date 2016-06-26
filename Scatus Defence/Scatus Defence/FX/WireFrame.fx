//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
};

struct VertexIn
{
	float3 PosL  : POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

RasterizerState WireFrameRS
{
	FillMode = WireFrame;
	CullMode = back;
	FrontCounterClockWise = false;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

technique11 Tech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetRasterizerState(WireFrameRS);
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
