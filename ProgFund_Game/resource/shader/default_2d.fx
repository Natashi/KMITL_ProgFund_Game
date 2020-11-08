sampler g_sSampler : register(s0);

float4x4 g_mWorld : WORLD;
float4x4 g_mProj : VIEWPROJECTION;
float4 g_vColor : OBJCOLOR;
float2 g_f2Scroll : UVSCROLL;

struct VS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
    float4 diffuse : COLOR0;
};
struct PS_INPUT {
    float4 position : POSITION;
    float2 texUV : TEXCOORD0;
	float4 diffuse : COLOR0;
};

PS_INPUT MainVS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
	
    output.position = mul(input.position, g_mWorld);
    output.position = mul(output.position, g_mProj);
	//output.position.xyz /= output.position.w;
	output.position.z = 1.0f;
    output.texUV = input.texUV + g_f2Scroll;
    output.diffuse = input.diffuse * g_vColor;
	
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
technique10 RenderUntextured {
	pass P0 {
		VertexShader = compile vs_2_0 MainVS();
		PixelShader = compile ps_2_0 MainPS_Untextured();
	}
}