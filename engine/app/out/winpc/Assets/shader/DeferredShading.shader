DeferredShading
{
    Tags
    {
        Queue Geometry
    }

    Pass 0
    {
        VS vs_light_dir
        PS ps_light_dir
        RenderStates rs_light_dir
    }

    Pass 1
    {
        VS vs_volume_cull
        PS ps_volume_cull
        RenderStates rs_volume_cull
    }

    Pass 2
    {
        VS vs_lighting
        PS ps_point
        RenderStates rs_lighting
    }

    Pass 3
    {
        VS vs_lighting
        PS ps_spot
        RenderStates rs_lighting
    }

    RenderStates rs_light_dir
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    RenderStates rs_volume_cull
    {
        Cull Back
        ZWrite Off
        ZTest LEqual
        Blend Off
        ColorMask 0
        Stencil
        {
            Ref 0
            ReadMask 255
            WriteMask 255
            Comp Always
            Pass Keep
            Fail Keep
            ZFail IncrSat
        }
    }

    RenderStates rs_lighting
    {
        Cull Front
        ZWrite Off
        ZTest GEqual
        Blend One, One
        Stencil
        {
            Ref 0
            ReadMask 255
            WriteMask 255
            Comp Equal
            Pass Replace
            Fail Replace
            ZFail Replace
        }
    }

    HLVS vs_light_dir
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 LightDirection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 EyePosition;
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;
            output.v_uv = input.Texcoord0;
            output.v_light_dir_world = -LightDirection.xyz;
            output.v_eye_pos_world = EyePosition.xyz;

            return output;
        }
    }

    HLPS ps_light_dir
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 GlobalAmbient;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 LightColor;
        };

        cbuffer cbuffer2 : register(b2)
        {
            matrix InvViewProjection;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _GBuffer1 : register(t1);
        SamplerState _GBuffer1_Sampler : register(s1);
        Texture2D _GBuffer2 : register(t2);
        SamplerState _GBuffer2_Sampler : register(s2);
        Texture2D _GBuffer3 : register(t3);
        SamplerState _GBuffer3_Sampler : register(s3);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            float2 normal_2 = _GBuffer1.Sample(_GBuffer1_Sampler, input.v_uv).rg;
            float2 specular = _GBuffer2.Sample(_GBuffer2_Sampler, input.v_uv).zw;
            float depth = _GBuffer3.Sample(_GBuffer3_Sampler, input.v_uv).r;

            float3 normal = 0;

            // decode normal2 to normal3
            if(length(normal_2) > 1)
            {
                normal = float3(0, 0, sign(normal_2.x));
            }
            else
            {
                float len = length(normal_2);
                normal.z = len*len*2-1;
                normal.xy = normalize(normal_2.xy)*sqrt(1-normal.z*normal.z);
            }
            //

            // get world position from depth and uv
            float x = input.v_uv.x * 2 - 1;
            float y = (1 - input.v_uv.y) * 2 - 1;
            float z = depth;

            float4 pos_world = mul(float4(x, y, z, 1), InvViewProjection);
            pos_world /= pos_world.w;
            //

            float3 eye_dir = normalize(input.v_eye_pos_world - pos_world.xyz);
            float3 light_dir = normalize(input.v_light_dir_world);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular.x) * specular.y;

            c.rgb = GlobalAmbient.rgb * c.rgb +
                diff * c.rgb * LightColor.rgb +
                spec * LightColor.rgb;

            return c;
        }
    }

    HLVS vs_volume_cull
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix WorldViewProjection;
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);

            return output;
        }
    }

    HLPS ps_volume_cull
    {
        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            return 1;
        }
    }

    HLVS vs_lighting
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix WorldViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 EyePosition;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 LightPositon;
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float4 v_pos_proj : TEXCOORD0;
            float3 v_light_pos_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);

            output.v_pos_proj = output.v_pos;
            output.v_light_pos_world = LightPositon.xyz;
            output.v_eye_pos_world = EyePosition.xyz;

            return output;
        }
    }

    HLPS ps_point
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 LightColor;
        };

        cbuffer cbuffer1 : register(b1)
        {
            matrix InvViewProjection;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 LightRange;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _GBuffer1 : register(t1);
        SamplerState _GBuffer1_Sampler : register(s1);
        Texture2D _GBuffer2 : register(t2);
        SamplerState _GBuffer2_Sampler : register(s2);
        Texture2D _GBuffer3 : register(t3);
        SamplerState _GBuffer3_Sampler : register(s3);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float4 v_pos_proj : TEXCOORD0;
            float3 v_light_pos_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float2 uv = 0;
            uv.x = input.v_pos_proj.x / input.v_pos_proj.w * 0.5 + 0.5;
            uv.y = 1 - (input.v_pos_proj.y / input.v_pos_proj.w * 0.5 + 0.5);

            float4 color = _MainTex.Sample(_MainTex_Sampler, uv);
            float2 normal_2 = _GBuffer1.Sample(_GBuffer1_Sampler, uv).rg;
            float2 specular = _GBuffer2.Sample(_GBuffer2_Sampler, uv).zw;
            float depth = _GBuffer3.Sample(_GBuffer3_Sampler, uv).r;

            float3 normal = 0;

            // decode normal2 to normal3
            if(length(normal_2) > 1)
            {
                normal = float3(0, 0, sign(normal_2.x));
            }
            else
            {
                float len = length(normal_2);
                normal.z = len*len*2-1;
                normal.xy = normalize(normal_2.xy)*sqrt(1-normal.z*normal.z);
            }
            //

            // get world position from depth and uv
            float4 pos_world = mul(float4(input.v_pos_proj.xy / input.v_pos_proj.w, depth, 1), InvViewProjection);
            pos_world /= pos_world.w;
            //

            float3 eye_dir = normalize(input.v_eye_pos_world - pos_world);
            float3 dis = input.v_light_pos_world - pos_world;
            float3 light_dir = normalize(dis);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular.x) * specular.y;

            float3 c = diff * color * LightColor.rgb +
                spec * LightColor.rgb;

            float range = (LightRange.x - length(dis)) / LightRange.x;
            float intensity = range * range;

            return float4(c * intensity, 1);
        }
    }

    HLPS ps_spot
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 LightColor;
        };

        cbuffer cbuffer1 : register(b1)
        {
            matrix InvViewProjection;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 SpotParam;
        };

        cbuffer cbuffer3 : register(b3)
        {
            float4 LightRange;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _GBuffer1 : register(t1);
        SamplerState _GBuffer1_Sampler : register(s1);
        Texture2D _GBuffer2 : register(t2);
        SamplerState _GBuffer2_Sampler : register(s2);
        Texture2D _GBuffer3 : register(t3);
        SamplerState _GBuffer3_Sampler : register(s3);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float4 v_pos_proj : TEXCOORD0;
            float3 v_light_pos_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float2 uv = 0;
            uv.x = input.v_pos_proj.x / input.v_pos_proj.w * 0.5 + 0.5;
            uv.y = 1 - (input.v_pos_proj.y / input.v_pos_proj.w * 0.5 + 0.5);

            float4 color = _MainTex.Sample(_MainTex_Sampler, uv);
            float2 normal_2 = _GBuffer1.Sample(_GBuffer1_Sampler, uv).rg;
            float2 specular = _GBuffer2.Sample(_GBuffer2_Sampler, uv).zw;
            float depth = _GBuffer3.Sample(_GBuffer3_Sampler, uv).r;

            float3 normal = 0;

            // decode normal2 to normal3
            if(length(normal_2) > 1)
            {
                normal = float3(0, 0, sign(normal_2.x));
            }
            else
            {
                float len = length(normal_2);
                normal.z = len*len*2-1;
                normal.xy = normalize(normal_2.xy)*sqrt(1-normal.z*normal.z);
            }
            //

            // get world position from depth and uv
            float4 pos_world = mul(float4(input.v_pos_proj.xy / input.v_pos_proj.w, depth, 1), InvViewProjection);
            pos_world /= pos_world.w;
            //

            float3 eye_dir = normalize(input.v_eye_pos_world - pos_world);
            float3 dis = input.v_light_pos_world - pos_world;
            float3 light_dir = normalize(dis);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular.x) * specular.y;

            float3 c = diff * color * LightColor.rgb +
                spec * LightColor.rgb;

            float range = (LightRange.x - length(dis)) / LightRange.x;
            float intensity = range * range;

            float rho = dot(SpotParam.xyz, -light_dir);
            float cos_phi = cos(SpotParam.w * 0.5);
            float cos_theta = cos(SpotParam.w * 0.5 * 0.7);
            float factor = 1;
            if(rho < cos_phi)
            {
                factor = 0;
            }
            else if(rho < cos_theta)
            {
                float p = abs((rho - cos_phi) / (cos_theta - cos_phi));
                factor = pow(p, 1);
            }

            return float4(c * intensity * factor, 1);
        }
    }
}