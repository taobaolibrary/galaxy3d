SkinnedMesh/Diffuse
{
	Tags
	{
		Queue Geometry
	}

	Pass
	{
		VS vs
		PS ps
		RenderStates rs
	}

	RenderStates rs
	{
        Cull Back
		ZWrite On
		ZTest LEqual
		Offset 0, 0
		Blend Off
		Stencil
		{
			Ref referenceValue
			ReadMask readMask
			WriteMask writeMask
			Comp comparisonFunction
			Pass stencilOperation
			Fail stencilOperation
			ZFail stencilOperation
		}
	}

	HLVS vs
	{
        cbuffer cbuffer0 : register( b0 )
        {
            matrix ViewProjection;
        };

        cbuffer cbuffer1 : register( b1 )
        {
            float4 _Color;
        };

        cbuffer cbuffer2 : register( b2 )
        {
            float4 Bones[108];
        };

		struct VS_INPUT
		{
			float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float4 BlendWeight : BLENDWEIGHT;
            float4 BlendIndices : BLENDINDICES;
			float2 Texcoord0 : TEXCOORD0;
		};

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
		};

		PS_INPUT main( VS_INPUT input )
		{
			PS_INPUT output = (PS_INPUT) 0;

            float4 skinned_pos_world = 0;
            for(int i=0; i<4; i++)
            {
                float weight = input.BlendWeight[i];
                int index = (int) input.BlendIndices[i];

                if(weight > 0 && index >= 0)
                {
                    matrix bone = {
                        Bones[index*3].x, Bones[index*3+1].x, Bones[index*3+2].x, 0,
                        Bones[index*3].y, Bones[index*3+1].y, Bones[index*3+2].y, 0,
                        Bones[index*3].z, Bones[index*3+1].z, Bones[index*3+2].z, 0,
                        Bones[index*3].w, Bones[index*3+1].w, Bones[index*3+2].w, 1
                    };
                    float4 world = mul(input.Position, bone);
                    skinned_pos_world += world * weight;
                }
            }

            output.v_pos = mul(skinned_pos_world, ViewProjection);
			output.v_uv = input.Texcoord0;
            output.v_color = _Color;
    
			return output;
		}
	}

	HLPS ps
	{
		Texture2D _MainTex : register( t0 );
		SamplerState _MainTex_Sampler : register( s0 );

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
		};

		float4 main( PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;
			return c;
		}
	}

	GLVS vs
	{
		uniform mat4 ViewProjection;
        uniform vec4 Bones[108];
        uniform vec4 _MainColor;

		attribute vec3 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
        attribute vec4 BlendWeight;
        attribute vec4 BlendIndices;
		attribute vec2 Texcoord0;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
            vec4 skinned_pos_world = vec4(0.0);
            int indices[4];
            indices[0] = int(BlendIndices.x);
            indices[1] = int(BlendIndices.y);
            indices[2] = int(BlendIndices.z);
            indices[3] = int(BlendIndices.w);
            float weights[4];
            weights[0] = BlendWeight.x;
            weights[1] = BlendWeight.y;
            weights[2] = BlendWeight.z;
            weights[3] = BlendWeight.w;
            for(int i=0; i<4; i++) {
                float weight = weights[i];
                int index = indices[i];
                if(weight > 0.0) {
                    mat4 bone = mat4(Bones[index*3], Bones[index*3+1], Bones[index*3+2], vec4(0, 0, 0, 1));
                    vec4 world = Position * bone;
                    skinned_pos_world += world * weight;
                }
            }

            gl_Position = skinned_pos_world * ViewProjection;
            v_uv = Texcoord0;
			v_color = _MainColor;
		}
	}

	GLPS ps
	{
		uniform sampler2D _MainTex;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
			vec4 color = texture2D(_MainTex, v_uv) * v_color;

			gl_FragColor = color;
		}
	}
}