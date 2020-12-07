sampler g_sSampler : register(s0);

float4x4 g_mWorld : WORLD;
float4x4 g_mView : VIEW;
float4x4 g_mProj : PROJECTION;

float4 g_vColor : OBJCOLOR;
float2 g_f2Scroll : UVSCROLL;

bool g_bFog : FOGENABLE;
float4 g_vFogColor : FOGCOLOR;
float2 g_f2FogDist : FOGDIST;

struct VS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
    float4 diffuse : COLOR0;
};
struct PS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
	float4 diffuse : COLOR0;
	float fogBlend : FOG;
};

PS_INPUT MainVS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
	
    output.position = mul(input.position, g_mWorld);
    output.position = mul(output.position, g_mView);
	
	//(No fog -> Full fog) == (Nearest -> Furthest) == (0 -> 1)
	output.fogBlend = g_bFog * (1.0f - saturate((g_f2FogDist.y - output.position.z) / (g_f2FogDist.y - g_f2FogDist.x)));
	
    output.position = mul(output.position, g_mProj);
	
    output.texUV = input.texUV + g_f2Scroll;
    output.diffuse = input.diffuse * g_vColor;
	
    return output;
}

float4 MainPS(PS_INPUT input) : COLOR0 {
	return lerp(tex2D(g_sSampler, input.texUV) * input.diffuse, g_vFogColor.rgb, input.fogBlend);
}
float4 MainPS_Untextured(PS_INPUT input) : COLOR0 {
    return lerp(input.diffuse, g_vFogColor.rgb, input.fogBlend);
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