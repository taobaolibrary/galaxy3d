#include "Launcher.h"

#define DEMO_TERRAIN 0
#define DEMO_SCENE 1
#define DEMO_DUST 0
#define DEMO_BATS 0
#define DEMO_SMOKE 0
#define DEMO_FIRE 0

using namespace Galaxy3D;

#if DEMO_TERRAIN
std::string clip_idle = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
std::string clip_move = "Move_Arthas_1586e0d40a0ba4545bd97991164aec42.1.clip";
#endif

static void add_fire_particles(std::shared_ptr<GameObject> &fire, std::shared_ptr<Camera> &cam3d);
static void add_lamp_particles(std::shared_ptr<GameObject> &lamp, std::shared_ptr<Camera> &cam3d);
static void add_dust_particles(std::shared_ptr<GameObject> &parent, std::shared_ptr<Camera> &cam3d, const Vector3 &local_pos);

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    
    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(Screen::GetHeight() / 200.f);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	fps = tr;
	fps->GetTransform()->SetParent(cam2d->GetTransform());
    fps->GetGameObject()->SetLayer(Layer::UI);

#if DEMO_DUST || DEMO_BATS || DEMO_SMOKE || DEMO_FIRE
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(60);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(0, 0, -5));
#endif

#if DEMO_FIRE
    cam3d->GetTransform()->SetPosition(Vector3(-2.78f, 2.72f, 4.54f));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(15.7f, 148.43f, 0));
    auto fire = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/Furnace.mesh");
    fire = fire->GetTransform()->Find("Furnace_A_01")->GetGameObject();

    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
    RenderSettings::light_directional_color = Color(1, 1, 1, 1) * 0.6f;
    RenderSettings::light_directional_intensity = 1;
    RenderSettings::light_directional_direction = Vector3(0, -1, -1);
    
    add_fire_particles(fire, cam3d);
#endif

#if DEMO_SMOKE
    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetPosition(Vector3(0, -4, 5));
    ps->GetTransform()->SetRotation(Quaternion::Euler(-90, 0, 0));
    ps->duration = 5;
    ps->start_lifetime = 9.91f;
    ps->start_speed = 1.32f;
    ps->start_size = 3.54f;
    ps->start_color = Color(135, 133, 127, 255) * (1.0f / 255);
    ps->max_particles = 1000;
    ps->emission_rate = 20;
    ps->emitter_shape = ParticleEmitterShape::Box;
    ps->emitter_shape_box_size = Vector3(2, 2, 1);
    ps->velocity_type = ValueType::Constant;
    ps->velocity = Vector3(0, 0, -0.475f);
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.282f, 10 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.706f, 9 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0.003f, Color(1, 1, 1, 1)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0.029f, 0.485f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));
    ps->angular_velocity = 45;

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlended");
    psm->SetColor("_TintColor", Color(1, 1, 1, 1) * 0.5f);
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/smoke.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
#endif

#if DEMO_BATS
    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetPosition(Vector3(0, 0, 0));
    ps->duration = 15;
    ps->start_lifetime = 5;
    ps->start_speed = 8.8f;
    ps->start_size = 1;
    ps->max_particles = 1000;
    ps->emission_rate = 20;
    ps->emitter_shape = ParticleEmitterShape::Cone;
    ps->emitter_shape_cone_angle = 11.07f;
    ps->emitter_shape_cone_radius = 1;
    ps->emitter_shape_cone_from = EmitterShapeConeFrom::Base;
    ps->force_type = ValueType::Curve;
    ps->force_curve_x = AnimationCurve();
    ps->force_curve_x.keys.push_back(Keyframe(0, 0.38f, 0, 0));
    ps->force_curve_y = AnimationCurve();
    ps->force_curve_y.keys.push_back(Keyframe(0.082f, 3.06f, 0, 0));
    ps->force_curve_z = AnimationCurve();
    ps->force_curve_z.keys.push_back(Keyframe(0.003f, -1.047f, 0, 0));
    ps->force_curve_z.keys.push_back(Keyframe(0.396f, 0.523f, 0, 0));
    ps->force_curve_z.keys.push_back(Keyframe(0.918f, -0.64f, 0, 0));
    ps->enable_texture_sheet_animation = true;
    ps->texture_sheet_animation_tile_x = 4;
    ps->texture_sheet_animation_tile_y = 4;
    ps->texture_sheet_animation_single_row = false;
    ps->texture_sheet_animation_frame_curve = AnimationCurve();
    ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(0, 0, 16, 16));
    ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(1, 16, 16, 16));
    ps->texture_sheet_animation_cycles = 32;

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/Multiply");
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/bats.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
#endif

#if DEMO_DUST
    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetPosition(Vector3(0, 0, 25));
    ps->GetTransform()->SetScale(Vector3(0.4237148f, 12.70362f, 0.3013902f));
    ps->duration = 32;
    ps->start_lifetime = 12;
    ps->start_speed = 0.2f;
    ps->start_size = 32;
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.3f, 25 / 255.0f));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
    ps->emitter_shape = ParticleEmitterShape::Box;
    ps->emitter_shape_box_size = Vector3(25, 1, 39.9f);
    ps->emitter_random_direction = true;
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.471f, 1));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0.994f, Color(10, 18, 38, 255) * (1 / 255.0f)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0, 0.5f, 0.5f, 0.5f));
    ps->size_curve.keys.push_back(Keyframe(1, 0.5f, 0.5f, 0.5f));
    ps->angular_velocity = 15;
    ps->max_particles = 1000;

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlended");
    psm->SetColor("_TintColor", Color(1, 1, 1, 1) * 0.5f);
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/dust.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
#endif

#if DEMO_SCENE
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetFieldOfView(45);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->GetTransform()->SetPosition(Vector3(-21.98f, 10.27f, 44.9f));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(24.58f, 137.26f, 0));
    cam3d->GetGameObject()->AddComponent<ImageEffectHDR>();
    cam3d->EnableHDR(true);

    auto lightmap_0 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-0_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    auto lightmap_1 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-1_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.clear();
    LightmapSettings::lightmaps.push_back(lightmap_0);
    LightmapSettings::lightmaps.push_back(lightmap_1);
    /*
    auto scene = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/Module_01.mesh");
    
    int cc = scene->GetTransform()->GetChildCount();
    for(int i=0; i<cc; i++)
    {
        auto child = scene->GetTransform()->GetChild(i);
        if("Furnace_A_01" == child->GetName())
        {
            add_fire_particles(child->GetGameObject(), cam3d);
        }
        else if("LampA_01" == child->GetName())
        {
            add_lamp_particles(child->GetGameObject(), cam3d);
        }
    }
    */
    auto dusts = GameObject::Create("FX");
    //dusts->GetTransform()->SetParent(scene->GetTransform());
    dusts->GetTransform()->SetLocalPosition(Vector3(28.68044f, -44.94969f, -16.97657f));
    add_dust_particles(dusts, cam3d, Vector3(-17.94812f, 41.30858f, 60.79422f));
    add_dust_particles(dusts, cam3d, Vector3(-7.215611f, 46.65371f, 44.56079f));
    add_dust_particles(dusts, cam3d, Vector3(-22.39325f, 39.16286f, 19.75619f));
    add_dust_particles(dusts, cam3d, Vector3(-47.34644f, 43.30156f, 15.99386f));
    
    //Renderer::BuildOctree(scene);

    //scene->SetStaticRecursively();
    Renderer::BuildStaticBatches();
#endif

#if DEMO_TERRAIN
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(30);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(101, 7, 115));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(20, -140, 0));
    
    std::vector<std::string> terrain_texs;
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/1.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/2.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/3.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/4.png");

    GameObject *terrain_obj = GameObject::Create("terrain").get();
    terrain_obj->SetLayer(Layer::Default);

    auto ter = terrain_obj->AddComponent<Terrain>();
    ter->SetCamera(cam3d);

    ter->LoadData(
        513,
        200.0f, 600.0f,
        Application::GetDataPath() + "/Assets/terrain/Terrain.raw",
        Application::GetDataPath() + "/Assets/terrain/Terrain.png",
        terrain_texs, 3);
    auto renderer = terrain_obj->AddComponent<TerrainRenderer>();
    renderer->SetTerrain(ter);
    
    auto lightmap_ter = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/LightmapFar-1.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    ter->GetSharedMaterial()->SetTexture("_Lightmap", lightmap_ter);

    auto lightmap = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/Objects/LightmapFar-0.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.push_back(lightmap);
    
    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/Objects/Objects.mesh");
    mesh->SetLayerRecursive(Layer::Default);
    Renderer::BuildOctree(mesh);
    
    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Arthas.anim");
    anim_obj->SetLayerRecursive(Layer::Default);
    anim_obj->GetTransform()->SetPosition(Vector3(91, 0.05f, 103));
    anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 0, 0));
    anim_obj->GetTransform()->SetScale(Vector3(1, 1, 1) * 0.5f);

    cam_offset = cam3d->GetTransform()->GetPosition() - anim_obj->GetTransform()->GetPosition();

    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState(clip_idle)->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState(clip_move)->wrap_mode = WrapMode::Loop;
    anim->Play(clip_idle);
    
    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
    RenderSettings::light_directional_color = Color(1, 1, 1, 1) * 0.6f;
    RenderSettings::light_directional_intensity = 1;
    RenderSettings::light_directional_direction = Vector3(0, -1, -1);

    Physics::Init();
    Physics::CreateTerrainRigidBody(ter.get());

    //set anim to ground
    Vector3 anim_pos = anim->GetTransform()->GetPosition();
    Vector3 hit;
    Vector3 nor;
    if(Physics::RarCast(Vector3(anim_pos.x, 1000, anim_pos.z), Vector3(0, -1, 0), 2000, hit, nor))
    {
        hit.y += 0.05f;
        anim->GetTransform()->SetPosition(hit);
    }
#endif
}

#if DEMO_TERRAIN
void Launcher::OnTweenSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        Vector3 *pos = (Vector3 *) value;

        Vector3 hit;
        Vector3 nor;
        if(Physics::RarCast(Vector3(pos->x, 1000, pos->z), Vector3(0, -1, 0), 2000, hit, nor))
        {
            hit.y += 0.05f;
            thiz->anim->GetTransform()->SetPosition(hit);
        }
    }
}

void Launcher::OnTweenFinished(Component *tween, std::weak_ptr<Component> &target)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        thiz->anim->CrossFade(clip_idle);
    }
}
#endif

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

#if DEMO_TERRAIN
	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
            Vector3 pos = t->position;
            Ray ray = cam3d->ScreenPointToRay(pos);

            Vector3 hit;
            Vector3 nor;
            if(Physics::RarCast(ray.origin, ray.GetDirection(), 1000, hit, nor))
            {
                Vector3 pos_old = anim->GetTransform()->GetPosition();
                Vector3 pos_new = hit + Vector3(0, 0.05f, 0);
                float move_speed = 3.0f;
                float move_time = (pos_new - pos_old).Magnitude() / move_speed;
                Vector3 dir = pos_new - pos_old;
                dir.y = 0;

                anim->CrossFade(clip_move);

                {
                    // tween position
                    auto tp = anim->GetGameObject()->GetComponent<TweenPosition>();
                    if(tp)
                    {
                        tp->Reset();
                    }
                    else
                    {
                        tp = anim->GetGameObject()->AddComponent<TweenPosition>();
                        tp->delay = 0;
                        tp->is_world = true;
                        tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                        tp->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        tp->target = GetComponentPtr();
                        tp->on_set_value = OnTweenSetValue;
                        tp->on_finished = OnTweenFinished;
                    }

                    tp->from = pos_old;
                    tp->to = pos_new;
                    tp->duration = move_time;
                }

                {
                    Vector3 origin = anim->GetTransform()->GetForward();
                    Vector3 fn = Vector3::Normalize(dir);
                    
                    if(!Mathf::FloatEqual(fn.SqrMagnitude(), 0) && fn != origin)
                    {
                        // tween rotation
                        float deg = Vector3::Angle(origin, fn);
                        Vector3 axis = origin * fn;
                        Quaternion rot;

                        float rot_speed = 270.0f;
                        float rot_time = deg / rot_speed;

                        if(axis == Vector3(0, 0, 0))
                        {
                            rot = Quaternion::AngleAxis(deg, anim->GetTransform()->GetUp());
                        }
                        else
                        {
                            rot = Quaternion::AngleAxis(deg, axis);
                        }

                        auto tr = anim->GetGameObject()->GetComponent<TweenRotation>();
                        if(tr)
                        {
                            tr->Reset();
                        }
                        else
                        {
                            tr = anim->GetGameObject()->AddComponent<TweenRotation>();
                            tr->delay = 0;
                            tr->is_world = true;
                            tr->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                            tr->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        }

                        Quaternion rot_now = anim->GetTransform()->GetRotation();

                        tr->from = rot_now;
                        tr->to = rot * rot_now;
                        tr->duration = rot_time;
                    }
                }
            }
		}
		else if(
            t->phase == TouchPhase::Ended ||
			t->phase == TouchPhase::Canceled)
		{
		}
		else if(t->phase == TouchPhase::Moved)
		{
            Vector3 pos = t->position;
		}
	}

    Physics::Step();
#endif
}

void Launcher::LateUpdate()
{
#if DEMO_TERRAIN
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
    cam3d->UpdateMatrix();
#endif
}

Launcher::~Launcher()
{
#if DEMO_TERRAIN
    Physics::Done();
#endif
}

static void add_dust_particles(std::shared_ptr<GameObject> &parent, std::shared_ptr<Camera> &cam3d, const Vector3 &local_pos)
{
    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetParent(parent->GetTransform());
    ps->GetTransform()->SetLocalPosition(local_pos);
    ps->GetTransform()->SetLocalScale(Vector3(0.4237148f, 12.70362f, 0.3013902f));
    ps->duration = 32;
    ps->start_lifetime = 12;
    ps->start_speed = 0.2f;
    ps->start_size = 32;
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.3f, 25 / 255.0f));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
    ps->emitter_shape = ParticleEmitterShape::Box;
    ps->emitter_shape_box_size = Vector3(25, 1, 39.9f);
    ps->emitter_random_direction = true;
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.471f, 1));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0.994f, Color(10, 18, 38, 255) * (1 / 255.0f)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0, 0.5f, 0.5f, 0.5f));
    ps->size_curve.keys.push_back(Keyframe(1, 0.5f, 0.5f, 0.5f));
    ps->angular_velocity = 15;
    ps->max_particles = 1000;

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlended");
    psm->SetColor("_TintColor", Color(1, 1, 1, 1) * 0.5f);
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/dust.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
}

static void add_lamp_particles(std::shared_ptr<GameObject> &lamp, std::shared_ptr<Camera> &cam3d)
{
    auto ps = GameObject::Create("lamp")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetParent(lamp->GetTransform());
    ps->GetTransform()->SetLocalPosition(Vector3(0.008f, 0.002f, -0.883f));
    ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0.0001373291f, -90, -90));
    ps->duration = 5;
    ps->start_lifetime = 5;
    ps->start_speed = 0.01f;
    ps->start_size = 7;
    ps->max_particles = 1000;
    ps->emission_rate = 1;
    ps->emitter_shape = ParticleEmitterShape::Disable;
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.215f, 39 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.765f, 69 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(152, 66, 0, 255) * (1.0f / 255)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(126, 81, 0, 255) * (1.0f / 255)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0, 1, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.235f, 0.212f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.458f, 0.995f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.744f, 0.46f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlendedPremultiply");
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/particle.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
}

static void add_fire_particles(std::shared_ptr<GameObject> &fire, std::shared_ptr<Camera> &cam3d)
{
    {
        auto ps = GameObject::Create("fire")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 0.9835533f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0;
        ps->start_size = 0.7f;
        ps->start_color = Color(255, 255, 255, 255) * (1.0f / 255);
        ps->max_particles = 1;
        ps->emission_rate = 10;
        ps->emitter_shape = ParticleEmitterShape::Sphere;
        ps->emitter_shape_sphere_radius = 0.01f;
        ps->enable_texture_sheet_animation = true;
        ps->texture_sheet_animation_tile_x = 16;
        ps->texture_sheet_animation_tile_y = 4;
        ps->texture_sheet_animation_single_row = false;
        ps->texture_sheet_animation_frame_curve = AnimationCurve();
        ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(0, 0, 64, 64));
        ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(1, 64, 64, 64));
        ps->texture_sheet_animation_cycles = 4;

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/AlphaBlended");
        psm->SetColor("_TintColor", Color(255, 175, 152, 128) * (1.0f / 255));
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/fire.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("smoke")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, -0.8561009f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0.85f;
        ps->start_size = 2.56f;
        ps->max_particles = 1000;
        ps->emission_rate = 3.6f;
        ps->emitter_shape = ParticleEmitterShape::Cone;
        ps->emitter_shape_cone_angle = 6.42f;
        ps->emitter_shape_cone_radius = 0.2f;
        ps->emitter_shape_cone_from = EmitterShapeConeFrom::Base;
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.218f, 25 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.826f, 39 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 0.03f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));
        ps->angular_velocity = 45;

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/Multiply");
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/smoke.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("spark")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 0.7186908f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 1;
        ps->start_lifetime_type = ValueType::RandomConstants;
        ps->start_lifetime_random_contants = Vector2(0.1f, 0.35f);
        ps->start_speed_type = ValueType::RandomConstants;
        ps->start_speed_random_contants = Vector2(2, 5);
        ps->start_size = 0.14f;
        ps->start_color = Color(255, 192, 56, 255) * (1.0f / 255);
        ps->max_particles = 1000;
        ps->emission_rate = 30;
        ps->emitter_shape = ParticleEmitterShape::Cone;
        ps->emitter_shape_cone_angle = 25;
        ps->emitter_shape_cone_radius = 0.43f;
        ps->emitter_shape_cone_from = EmitterShapeConeFrom::Base;
        ps->emitter_random_direction = true;
        ps->velocity_type = ValueType::Constant;
        ps->velocity = Vector3(-0.66f, -1.11f, 0.17f);
        ps->force_type = ValueType::Constant;
        ps->force = Vector3(4, 0.03f, 1.32f);
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 0.197f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.946f, 0.409f, 0, 0));

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        psr->render_mode = ParticleSystemRenderMode::Stretch;
        psr->stretch_speed_scale = 0;
        psr->stretch_length_scale = 7.55f;
        auto psm = Material::Create("Particles/Additive");
        psm->SetColor("_TintColor", Color(1, 1, 1, 0.5f));
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/spark.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("halo")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 1.146898f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(90, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0.01f;
        ps->start_size = 5;
        ps->max_particles = 1000;
        ps->emission_rate = 1;
        ps->emitter_shape = ParticleEmitterShape::Disable;
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.215f, 39 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.765f, 69 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(152, 66, 0, 255) * (1.0f / 255)));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(126, 81, 0, 255) * (1.0f / 255)));
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 1, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.235f, 0.212f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.458f, 0.995f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.744f, 0.46f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/AlphaBlendedPremultiply");
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/particle.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }
}