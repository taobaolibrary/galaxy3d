#include "Light.h"
#include "Mesh.h"
#include "GraphicsDevice.h"
#include "RenderSettings.h"
#include "RenderTexture.h"

namespace Galaxy3D
{
    const float Light::SPOT_NEAR = 0.3f;
    std::list<Light *> Light::m_lights;
    std::shared_ptr<Mesh> Light::m_volume_sphere;
    std::shared_ptr<Mesh> Light::m_volume_cone;

    Light::Light():
        m_type(LightType::Point),
        m_spot_angle(30),
        m_range(10),
        m_color(1, 1, 1, 1),
        m_intensity(1),
        m_shadow_enable(false),
        m_shadow_bias(0.005f),
        m_shadow_strength(1.0f),
        m_cascade(false),
        m_cascade_rendering_index(0)
    {
        m_lights.push_back(this);
    }

    Light::~Light()
    {
        m_lights.remove(this);
    }

    void Light::SetRange(float range)
    {
        m_range = range;
    }

    std::shared_ptr<RenderTexture> Light::GetShadowMap()
    {
        if(m_shadow_enable)
        {
            if(!m_shadow_map)
            {
                m_shadow_map = RenderTexture::Create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, RenderTextureFormat::Depth, DepthBuffer::Depth_0, FilterMode::Point);
            }
        }

        return m_shadow_map;
    }

    void Light::PrepareForRenderShadowMap()
    {
        auto shadow_map = GetShadowMap();
        auto camera = Camera::GetCurrent();

        camera->SetRenderTarget(shadow_map);

        BuildViewProjectionMatrix();
    }

    Matrix4x4 Light::BuildDirectionalMatrix(float clip_near, float clip_far)
    {
        auto camera = Camera::GetCurrent();

        float far_z = clip_far;
        float near_z = clip_near;
        float far_h = far_z * tanf(camera->GetFieldOfView() * 0.5f * Mathf::Deg2Rad);
        float far_w = far_h * camera->GetAspect();
        float near_h = far_h * near_z / far_z;
        float near_w = far_w * near_z / far_z;
        Vector3 corners[8];
        corners[0] = Vector3(-near_w, near_h, near_z);
        corners[1] = Vector3(-near_w, -near_h, near_z);
        corners[2] = Vector3(near_w, -near_h, near_z);
        corners[3] = Vector3(near_w, near_h, near_z);
        corners[4] = Vector3(-far_w, far_h, far_z);
        corners[5] = Vector3(-far_w, -far_h, far_z);
        corners[6] = Vector3(far_w, -far_h, far_z);
        corners[7] = Vector3(far_w, far_h, far_z);

        // transform corners from view space to light space
        Vector3 max_v(Mathf::MinFloatValue, Mathf::MinFloatValue, Mathf::MinFloatValue);
        Vector3 min_v(Mathf::MaxFloatValue, Mathf::MaxFloatValue, Mathf::MaxFloatValue);
        for(auto &i : corners)
        {
            Vector3 in_wolrd = camera->GetTransform()->TransformPoint(i);
            Matrix4x4 inverse_mat = Matrix4x4::TRS(GetTransform()->GetPosition(), GetTransform()->GetRotation(), Vector3(1, 1, 1));
            inverse_mat = inverse_mat.Inverse();
            Vector3 in_light = inverse_mat.MultiplyPoint3x4(in_wolrd);

            max_v = Vector3::Max(max_v, in_light);
            min_v = Vector3::Min(min_v, in_light);
        }

        Vector3 center = (max_v + min_v) * 0.5f;
        Vector3 size = max_v - min_v;

        GetTransform()->SetPosition(GetTransform()->TransformPoint(center));

        //build light orthographic matrix
        float top = size.y / 2;
        float bottom = - size.y / 2;
        float plane_h = size.y;
        float plane_w = size.x;
        float plane_near = - size.z / 2;
        float plane_far = size.z / 2;
        return Matrix4x4::Ortho(-plane_w/2, plane_w/2, bottom, top, plane_near, plane_far);
    }

    void Light::SetCascadeViewport(int index)
    {
        m_cascade_rendering_index = index;

        auto shadow_map = GetShadowMap();

        float top[3] = {0, 0.6f, 0.9f};
        float height[3] = {0.6f, 0.3f, 0.1f};

        Rect rect;
        rect.left = 0;
        rect.top = top[index] * shadow_map->GetHeight();
        rect.width = 1.0f * shadow_map->GetWidth();
        rect.height = height[index] * shadow_map->GetHeight();
        Camera::SetViewport(rect);
    }

    void Light::BuildViewProjectionMatrix()
    {
        auto shadow_map = GetShadowMap();
        auto camera = Camera::GetCurrent();

        if(m_type == LightType::Directional)
        {
            if(m_cascade)
            {
                const int nears[] = {0, 1, 5};
                const int fars[] = {1, 5, 21};
                const float full = 21.0f;

                for(int i=0; i<CASCADE_SHADOW_COUNT; i++)
                {
                    float clip_near = Mathf::Lerp(camera->GetClipNear(), camera->GetClipFar(), nears[i] / full, false);
                    float clip_far = Mathf::Lerp(camera->GetClipNear(), camera->GetClipFar(), fars[i] / full, false);
                    auto projection_matrix = BuildDirectionalMatrix(clip_near, clip_far);

                    auto view_matrix = Matrix4x4::LookTo(
                        GetTransform()->GetPosition(),
                        GetTransform()->GetRotation() * Vector3(0, 0, 1),
                        GetTransform()->GetRotation() * Vector3(0, 1, 0));

                    m_view_projection_matrices[i] = projection_matrix * view_matrix;
                }
            }
            else
            {
                auto projection_matrix = BuildDirectionalMatrix(camera->GetClipNear(), camera->GetClipFar());

                auto view_matrix = Matrix4x4::LookTo(
                    GetTransform()->GetPosition(),
                    GetTransform()->GetRotation() * Vector3(0, 0, 1),
                    GetTransform()->GetRotation() * Vector3(0, 1, 0));

                m_view_projection_matrices[0] = projection_matrix * view_matrix;
            }
        }
        else if(m_type == LightType::Spot)
        {
            auto projection_matrix = Matrix4x4::Perspective(m_spot_angle, 1.0f, SPOT_NEAR, m_range);

            auto view_matrix = Matrix4x4::LookTo(
                GetTransform()->GetPosition(),
                GetTransform()->GetRotation() * Vector3(0, 0, 1),
                GetTransform()->GetRotation() * Vector3(0, 1, 0));

            m_view_projection_matrices[0] = projection_matrix * view_matrix;
        }
    }

    void Light::CreateVolumeMeshIfNeeded()
    {
        if(!m_volume_sphere)
        {
            m_volume_sphere = Mesh::CreateMeshSphere();
        }

        if(!m_volume_cone)
        {
            m_volume_cone = Mesh::CreateMeshCone();
        }
    }

    std::list<Light *> Light::GetLightsHasShadow()
    {
        std::list<Light *> lights;

        for(auto i : m_lights)
        {
            if(i->m_shadow_enable)
            {
                lights.push_back(i);
            }
        }

        return lights;
    }

    void Light::ShadingDirectionalLight(const Light *light, std::shared_ptr<Material> &material, bool add)
    {
        auto camera = Camera::GetCurrent();

        material->SetVector("LightDirection", Vector4(light->GetTransform()->GetRotation() * Vector3(0, 0, 1)));
        material->SetColor("LightColor", light->GetColor() * light->GetIntensity());
        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, add ? 1 : 0);
    }

    void Light::DeferredShadingLights(std::shared_ptr<Material> &material)
    {
        CreateVolumeMeshIfNeeded();

        auto camera = Camera::GetCurrent();
        auto vp = camera->GetViewProjectionMatrix();
        FrustumBounds frustum(camera->GetViewProjectionMatrix());

        material->SetZBufferParams(camera);

        // shading global directional light first with blend off
        auto global_dir = RenderSettings::GetGlobalDirectionalLight();
        material->SetVector("ShadowParam", Vector4(global_dir->m_shadow_bias, global_dir->m_shadow_strength, 0, global_dir->IsShadowEnable() ? 1.0f : 0));
        if(global_dir->IsShadowEnable())
        {
            material->SetTexture("_ShadowMapTexture", global_dir->GetShadowMap());
            std::vector<Matrix4x4> mats(3);
            memcpy(&mats[0], global_dir->m_view_projection_matrices, sizeof(Matrix4x4) * 3);
            material->SetMatrixArray("ViewProjectionLight", mats);
        }
        ShadingDirectionalLight(global_dir.get(), material, false);
 
        // shading other lights with blend add
        for(auto i : m_lights)
        {
            if(camera->IsCulling(i->GetGameObject()))
            {
                continue;
            }

            material->SetVector("ShadowParam", Vector4(i->m_shadow_bias, i->m_shadow_strength, 0, i->IsShadowEnable() ? 1.0f : 0));
            if(i->IsShadowEnable())
            {
                material->SetTexture("_ShadowMapTexture", i->GetShadowMap());
                material->SetMatrix("ViewProjectionLight", i->GetViewProjectionMatrix());
            }

            material->SetVector("LightRange", Vector4(i->m_range));

            if(i->m_type == LightType::Point)
            {
                Vector3 pos = i->GetTransform()->GetPosition();
                float radius = i->m_range;

                if(frustum.ContainsSphere(pos, radius) != ContainsResult::Out)
                {
                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);

                    float distance = frustum.TestPlane(pos, 5);
                    if(fabs(distance) < radius)
                    {
                        // bound sphere cross with far plane, stencil test will get error result
                        // so draw full screen quad like directional light
                        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, 5);
                    }
                    else
                    {
                        auto wvp = vp * Matrix4x4::TRS(pos, Quaternion::Identity(), Vector3(1, 1, 1) * radius);
                        material->SetMatrix("WorldViewProjection", wvp);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 2);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_sphere, 0, material, 3);
                    }
                }
            }
            else if(i->m_type == LightType::Spot)
            {
                // 使用圆锥的外接球进行视锥剔除
                float tg = tanf(i->m_spot_angle * 0.5f * Mathf::Deg2Rad);
                float radius = i->m_range * 0.5f * (1 + tg * tg);
                Vector3 pos = i->GetTransform()->GetRotation() * (i->GetTransform()->GetPosition() + Vector3(0, 0, radius));

                if(frustum.ContainsSphere(pos, radius) != ContainsResult::Out)
                {
                    Vector3 spot_dir = (i->GetTransform()->GetRotation() * Vector3(0, 0, 1));
                    spot_dir.Normalize();
                    Vector4 spot_param = spot_dir;
                    spot_param.w = i->m_spot_angle * Mathf::Deg2Rad;
                    material->SetVector("SpotParam", spot_param);
                    material->SetVector("LightPositon", Vector4(i->GetTransform()->GetPosition()));
                    material->SetColor("LightColor", i->m_color * i->m_intensity);

                    float distance = frustum.TestPlane(pos, 5);
                    if(fabs(distance) < radius)
                    {
                        // bound sphere cross with far plane, stencil test will get error result
                        // so draw full screen quad like directional light
                        GraphicsDevice::GetInstance()->Blit(material->GetMainTexture(), camera->GetRenderTarget(), material, 6);
                    }
                    else
                    {
                        float scale_xy = i->m_range * tg;
                        auto wvp = vp * Matrix4x4::TRS(i->GetTransform()->GetPosition(), i->GetTransform()->GetRotation(), Vector3(scale_xy, scale_xy, i->m_range));
                        material->SetMatrix("WorldViewProjection", wvp);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 2);
                        GraphicsDevice::GetInstance()->DrawMeshNow(m_volume_cone, 0, material, 4);
                    }
                }
            }
            else if(i->m_type == LightType::Directional)
            {
                if(i != RenderSettings::GetGlobalDirectionalLight().get())
                {
                    ShadingDirectionalLight(i, material, true);
                }
            }
        }
    }
}