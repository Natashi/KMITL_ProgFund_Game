sampler g_sSampler : register(s0);

float4x4 g_mViewProj : VIEWPROJECTION;

struct VS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
    float4 diffuse : COLOR0;
	
	float4 i_color : COLOR1;
	float4 i_xypos_xysc : TEXCOORD1;
	float4 i_xyzang : TEXCOORD2;
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

	float2 ax = float2(sin(input.i_xyzang.x), cos(input.i_xyzang.x));
	float2 ay = float2(sin(input.i_xyzang.y), cos(input.i_xyzang.y));
	float2 az = float2(sin(input.i_xyzang.z), cos(input.i_xyzang.z));

	float4x4 instanceMat = float4x4(
		float4(
			t_scale.x * (ay.y * az.y - ax.x * ay.x * az.x),
			t_scale.x * (-ax.y * az.x),
			t_scale.x * (ay.x * az.y + ax.x * ay.y * az.x),
			0
		),
		float4(
			t_scale.y * (ay.y * az.x + ax.x * ay.x * az.y),
			t_scale.y * (ax.y * az.y),
			t_scale.y * (ay.x * az.x - ax.x * ay.y * az.y),
			0
		),
		float4(
			1 * (-ax.y * ay.x),
			1 * (ax.x),
			1 * (ax.y * ay.y),
			0
		),
		float4(input.i_xypos_xysc.xy, 1, 1)
	);
	
    output.position = mul(input.position, instanceMat);
    output.position = mul(output.position, g_mViewProj);
	//output.position.xyz /= output.position.w;
	output.position.z = 1.0f;
	
    output.texUV = input.texUV;
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