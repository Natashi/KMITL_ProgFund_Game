sampler g_sSampler : register(s0);

float4x4 g_mViewProj : VIEWPROJECTION;

struct VS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
    float4 diffuse : COLOR0;
	
	float4 i_color : COLOR1;
	float4 i_xypos_xysc : TEXCOORD1;
	float4 i_zmov_uvscr : TEXCOORD2;	//[rotate factor sin, rotate factor cos, u scroll, v scroll]
	float4 i_usdat : TEXCOORD3;
};
struct PS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
	float4 diffuse : COLOR0;
};

PS_INPUT MainVS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
	
	float2 t_scale = input.i_xypos_xysc.zw;

	float4x4 instanceMat = float4x4(
		float4(
			t_scale.x * (input.i_zmov_uvscr.y),
			t_scale.x * (-input.i_zmov_uvscr.x),
			0, 0
		),
		float4(
			t_scale.y * (input.i_zmov_uvscr.x),
			t_scale.y * (input.i_zmov_uvscr.y),
			0, 0
		),
		float4(0, 0, 1, 0),
		float4(input.i_xypos_xysc.xy, 0, 1)
	);
	
    output.position = mul(input.position, instanceMat);
    output.position = mul(output.position, g_mViewProj);
	output.position.z = 1.0f;
	
    output.texUV = input.texUV + input.i_zmov_uvscr.zw;
    output.diffuse = input.diffuse * input.i_color;
	
    return output;
}

float4 MainPS(PS_INPUT input) : COLOR0 {
    return tex2D(g_sSampler, input.texUV) * input.diffuse;
}
float4 MainPS_Untextured(PS_INPUT input) : COLOR0 {
    return input.diffuse;
}

technique Render {
	pass P0 {
		VertexShader = compile vs_2_0 MainVS();
		PixelShader = compile ps_2_0 MainPS();
	}
}
technique RenderUntextured {
	pass P0 {
		VertexShader = compile vs_2_0 MainVS();
		PixelShader = compile ps_2_0 MainPS_Untextured();
	}
}