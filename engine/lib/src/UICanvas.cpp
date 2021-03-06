#include "UICanvas.h"
#include "Screen.h"
#include "Transform.h"
#include "Input.h"
#include "GameObject.h"
#include "GTTime.h"
#include "Renderer.h"
#include "UIEventListener.h"
#include "SpriteNode.h"
#include "SpriteBatchRenderer.h"
#include "TextRenderer.h"
#include "LabelNode.h"
#include "LabelBatchRenderer.h"

namespace Galaxy3D
{
    class MouseOrTouch
    {
    public:
        KeyCode::Enum key;
        Vector2 pos;				// Current position of the mouse or touch event
        Vector2 last_pos;			// Previous position of the mouse or touch event
        Vector2 delta;		 	    // Delta since last update
        Vector2 total_delta;	    // Delta since the event started being tracked

        std::weak_ptr<GameObject> last;			// Last object under the touch or mouse
        std::weak_ptr<GameObject> current;		// Current game object under the touch or mouse
        std::weak_ptr<GameObject> pressed;		// Last game object to receive OnPress
        std::weak_ptr<GameObject> dragged;		// Game object that's being dragged

        float press_time;	// When the touch event started
        float click_time;	// The last time a click event was sent out

        bool touch_began;
        bool press_started;
        bool drag_started;
        int ignore_delta;

        MouseOrTouch():
            key(KeyCode::None),
            press_time(0),
            click_time(0),
            touch_began(true),
            press_started(false),
            drag_started(false),
            ignore_delta(0)
        {}

        /// <summary>
        /// Delta time since the touch operation started.
        /// </summary>
        float GetDeltaTime() {
            return GTTime::GetRealTimeSinceStartup() - press_time;
        }
    };

    struct DepthEntry
    {
        long long depth;
        int second_order;
        RaycastHit hit;
        Vector3 point;
        std::weak_ptr<GameObject> go;

        int operator <(DepthEntry &a) const
        {
            if(depth == a.depth)
            {
                return second_order > a.second_order;
            }

            return depth > a.depth;
        }
    };

    static int g_current_mouse = -1;
    static MouseOrTouch g_mouse[3];
    static MouseOrTouch *g_current_touch = NULL;
    static Vector2 g_last_pos;
    static int g_current_touch_id = -100;
    static std::weak_ptr<GameObject> g_ray_hit_object;
    static std::weak_ptr<GameObject> g_hover_object;
    static std::weak_ptr<GameObject> g_selected_object;
    static std::weak_ptr<Camera> g_camera;
    static RaycastHit g_last_hit;
    static Vector3 g_last_world_position;
    static bool g_input_focus = false;

    std::weak_ptr<GameObject> UICanvas::GetRayHitObject()
    {
        return g_ray_hit_object;
    }

    Vector2 UICanvas::GetLastPosition()
    {
        return g_last_pos;
    }

    RaycastHit UICanvas::GetLastRayHit()
    {
        return g_last_hit;
    }

	bool UICanvas::IsPressStarted()
	{
		if(g_current_touch != NULL)
		{
			return g_current_touch->press_started;
		}
		
		return false;
	}

    static bool raycast(const Vector3 &in_pos)
    {
		if(g_camera.expired())
		{
			return false;
		}

        auto cam = g_camera.lock();

        Vector3 pos = cam->ScreenToViewportPoint(in_pos);
        if(pos.x < 0 || pos.x > 1 || pos.y < 0 || pos.y > 1)
            return false;

        Ray ray = cam->ScreenPointToRay(in_pos);
        int mask = cam->GetCullingMask();
        float lenth = cam->GetClipFar() - cam->GetClipNear();
        std::list<DepthEntry> renderer_hits;

        auto hits = Physics::RaycastAll(ray.origin, ray.GetDirection(), lenth, mask);
        if(!hits.empty())
        {
            for(size_t i=0; i<hits.size(); i++)
            {
                DepthEntry hit;
                hit.second_order = 0;

                auto go = hits[i].collider.lock()->GetGameObject();
                auto renderer = go->GetComponent<Renderer>();
                if(!renderer)
                {
                    auto sprite_node = go->GetComponent<SpriteNode>();
                    if(sprite_node)
                    {
                        auto batch = sprite_node->GetBatch().lock();
                        renderer = batch;
                        hit.second_order = sprite_node->GetSortingOrder();

                        if(batch->IsClip() && !batch->IsPointInClipRect(hits[i].point))
                        {
                            renderer.reset();
                        }
                    }
					else
					{
						auto label_node = go->GetComponent<LabelNode>();
						if(label_node)
						{
							auto batch = label_node->GetBatch().lock();
							renderer = batch;
							hit.second_order = label_node->GetSortingOrder();

							auto clip_panel = batch->GetClipPanel().lock();
							if(batch->IsClip() && clip_panel && !clip_panel->IsPointInClipRect(hits[i].point))
							{
								renderer.reset();
							}
						}
					}
                }
                else
                {
                    auto text = std::dynamic_pointer_cast<TextRenderer>(renderer);
                    if(text && text->IsClip())
                    {
                        auto batch = text->GetClipPanel();
                        if(!batch.expired() && batch.lock()->IsPointInClipRect(hits[i].point))
                        {
                            renderer.reset();
                        }
                    }
                }

                if(renderer)
                {
                    long long layer = renderer->GetSortingLayer();
                    layer <<= 32;
                    long long order = renderer->GetSortingOrder();

                    hit.depth = layer | order;
                    hit.hit = hits[i];
                    hit.point = hits[i].point;
                    hit.go = hits[i].collider.lock()->GetGameObject();

                    renderer_hits.push_back(hit);
                }
            }
            
            if(!renderer_hits.empty())
            {
                renderer_hits.sort();

                g_last_hit = renderer_hits.front().hit;
                g_ray_hit_object = renderer_hits.front().go;
                g_last_world_position = renderer_hits.front().point;
                return true;
            }
            else
            {
                return false;
            }
        }

        return false;
    }

    static void raycast(MouseOrTouch &touch)
    {
        if(!raycast(Vector3(touch.pos)))
            g_ray_hit_object = std::weak_ptr<GameObject>();
        touch.last = touch.current;
        touch.current = g_ray_hit_object;
        g_last_pos = touch.pos;
    }

    int UICanvas::GetWidth()
    {
        if(m_width == 0)
        {
            m_width = m_camera.lock()->GetPixelWidth();
        }

        return m_width;
    }

    int UICanvas::GetHeight()
    {
        if(m_height == 0)
        {
            m_height = m_camera.lock()->GetPixelHeight();
        }

        return m_height;
    }

    void UICanvas::SetSize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }

    void UICanvas::AnchorTransform(std::shared_ptr<Transform> &t, const Vector4 &anchor)
    {
        int canvas_w = GetWidth();
        int canvas_h = GetHeight();

        float x = (anchor.x - 0.5f) * canvas_w + anchor.z;
        float y = ((1.0f - anchor.y) - 0.5f) * canvas_h + anchor.w;
        Vector3 pos_local = Vector3(Mathf::Round(x), Mathf::Round(y), 0);

        t->SetPosition(GetTransform()->TransformPoint(pos_local));
    }

    void UICanvas::Start()
    {
        
    }

    void UICanvas::Update()
    {
        g_camera = m_camera;

		ProcessMouse();
    }

    void UICanvas::SetHoveredObject(const std::weak_ptr<GameObject> &obj)
    {
        if(g_hover_object.lock() == obj.lock())
        {
            return;
        }

        auto hover = g_hover_object.lock();
        if(hover)
        {
            auto listener = hover->GetComponent<UIEventListener>();
            if(listener)
            {
                listener->OnHover(false);
            }
        }

        g_hover_object = obj;
        hover = g_hover_object.lock();
        if(hover)
        {
            auto listener = hover->GetComponent<UIEventListener>();
            if(listener)
            {
                listener->OnHover(true);
            }
        }
    }

    void UICanvas::ProcessRelease(float drag)
    {
        g_current_touch->press_started = false;

        if(!g_current_touch->pressed.expired())
        {
            // If there was a drag event in progress, make sure OnDragOut gets sent
            if(g_current_touch->drag_started)
            {
                if(!g_current_touch->last.expired())
                {
                    auto listener = g_current_touch->last.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDragOut(g_current_touch->dragged);
                    }
                }
                
                if(!g_current_touch->dragged.expired())
                {
                    auto listener = g_current_touch->dragged.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDragEnd();
                    }
                }
            }

            // Send the notification of a touch ending
            auto pressed_listener = g_current_touch->pressed.lock()->GetComponent<UIEventListener>();
            if(pressed_listener)
            {
                pressed_listener->OnPress(false);
            }

            // Send a hover message to the object
            // OnHover is sent to restore the visual state
            if(g_hover_object.lock() == g_current_touch->current.lock())
            {
                if(!g_current_touch->current.expired())
                {
                    auto listener = g_current_touch->current.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnHover(true);
                    }
                }
            }
            else
                SetHoveredObject(g_current_touch->current);

            // If the button/touch was released on the same object, consider it a click and select it
            if( g_current_touch->dragged.lock() == g_current_touch->current.lock() ||
                Vector3(g_current_touch->total_delta).SqrMagnitude() < drag)
            {
                // If the touch should consider clicks, send out an OnClick notification
                if(g_current_touch->pressed.lock() == g_current_touch->current.lock())
                {
                    float time = GTTime::GetRealTimeSinceStartup();

                    if(pressed_listener)
                    {
                        pressed_listener->OnClick();
                    }

                    if(g_current_touch->click_time + 0.35f > time)
                    {
                        if(pressed_listener)
                        {
                            pressed_listener->OnDoubleClick();
                        }
                    }
                    g_current_touch->click_time = time;
                }
            }
            else if(g_current_touch->drag_started) // The button/touch was released on a different object
            {
                // Send a drop notification (for drag & drop)
                if(!g_current_touch->current.expired())
                {
                    auto listener = g_current_touch->current.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDrop(g_current_touch->dragged);
                    }
                }
            }
        }

        g_current_touch->drag_started = false;
        g_current_touch->pressed.reset();
        g_current_touch->dragged.reset();
    }

    void UICanvas::ProcessPress(bool pressed, float click, float drag)
    {
        if(pressed)
        {
            g_current_touch->press_started = true;

            if(!g_current_touch->pressed.expired())
            {
                auto listener = g_current_touch->pressed.lock()->GetComponent<UIEventListener>();
                if(listener)
                {
                    listener->OnPress(false);
                }
            }

            g_current_touch->pressed = g_current_touch->current;
            g_current_touch->dragged = g_current_touch->current;
            g_current_touch->total_delta = Vector2(0, 0);
            g_current_touch->drag_started = false;

            if(!g_current_touch->pressed.expired())
            {
                auto listener = g_current_touch->pressed.lock()->GetComponent<UIEventListener>();
                if(listener)
                {
                    listener->OnPress(true);
                }
            }

            if(g_selected_object.lock() != g_current_touch->pressed.lock())
            {
                g_input_focus = false;

                if(!g_selected_object.expired())
                {
                    auto listener = g_selected_object.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnSelect(false);
                    }
                }

                // Change the selection
                g_selected_object = g_current_touch->pressed;

                // Set the selection
                if(!g_selected_object.expired())
                {
                    auto selected = g_selected_object.lock();

                    //g_input_focus = (selected->IsActiveInHierarchy() && selected->GetComponent<UIInput>());
                    g_input_focus = false;
                    
                    auto listener = selected->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnSelect(true);
                    }
                }
            }
        }
        else if(!g_current_touch->pressed.expired() &&
            (Vector3(g_current_touch->delta).SqrMagnitude() > 0 || g_current_touch->current.lock() != g_current_touch->last.lock()))
        {
            // Keep track of the total movement
            g_current_touch->total_delta = g_current_touch->total_delta + g_current_touch->delta;
            float mag = Vector3(g_current_touch->total_delta).SqrMagnitude();
            bool just_started = false;

            // If the drag process hasn't started yet but we've already moved off the object, start it immediately
            if(!g_current_touch->drag_started && g_current_touch->current.lock() != g_current_touch->last.lock())
            {
                g_current_touch->drag_started = true;
                g_current_touch->delta = g_current_touch->total_delta;

                // OnDragOver is sent for consistency, so that OnDragOut is always preceded by OnDragOver
                if(!g_current_touch->dragged.expired())
                {
                    auto listener = g_current_touch->dragged.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDragStart();
                    }
                }
                
                if(!g_current_touch->last.expired())
                {
                    auto listener = g_current_touch->last.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDragOver(g_current_touch->dragged);
                    }
                }
            }
            else if(!g_current_touch->drag_started && drag < mag)
            {
                // If the drag event has not yet started, see if we've dragged the touch far enough to start it
                just_started = true;
                g_current_touch->drag_started = true;
                g_current_touch->delta = g_current_touch->total_delta;
            }

            // If we're dragging the touch, send out drag events
            if(g_current_touch->drag_started)
            {
                if(just_started)
                {
                    if(!g_current_touch->dragged.expired())
                    {
                        auto listener = g_current_touch->dragged.lock()->GetComponent<UIEventListener>();
                        if(listener)
                        {
                            listener->OnDragStart();
                        }
                    }

                    if(!g_current_touch->current.expired())
                    {
                        auto listener = g_current_touch->current.lock()->GetComponent<UIEventListener>();
                        if(listener)
                        {
                            listener->OnDragOver(g_current_touch->dragged);
                        }
                    }
                }
                else if(g_current_touch->last.lock() != g_current_touch->current.lock())
                {
                    if(!g_current_touch->last.expired())
                    {
                        auto listener = g_current_touch->last.lock()->GetComponent<UIEventListener>();
                        if(listener)
                        {
                            listener->OnDragOut(g_current_touch->dragged);
                        }
                    }

                    if(!g_current_touch->current.expired())
                    {
                        auto listener = g_current_touch->current.lock()->GetComponent<UIEventListener>();
                        if(listener)
                        {
                            listener->OnDragOver(g_current_touch->dragged);
                        }
                    }
                }

                if(!g_current_touch->dragged.expired())
                {
                    auto listener = g_current_touch->dragged.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnDrag(g_current_touch->delta);
                    }
                }

                g_current_touch->last = g_current_touch->current;
            }
        }
    }

    void UICanvas::ProcessTouch(bool pressed, bool released)
    {
        float drag_threshold = 4;
        float click_threshold = 10;

        drag_threshold *= drag_threshold;
        click_threshold *= click_threshold;

        if(!g_current_touch->pressed.expired())
        {
            if(released)
                ProcessRelease(drag_threshold);
            ProcessPress(pressed, click_threshold, drag_threshold);

            if (g_current_touch->pressed.lock() == g_current_touch->current.lock() &&
                !g_current_touch->drag_started &&
                g_current_touch->GetDeltaTime() > 1)
            {
                if(!g_current_touch->current.expired())
                {
                    auto listener = g_current_touch->current.lock()->GetComponent<UIEventListener>();
                    if(listener)
                    {
                        listener->OnLongPress();
                    }
                }
            }
        }
        else
        {
            ProcessPress(pressed, click_threshold, drag_threshold);
            if(released)
                ProcessRelease(drag_threshold);
        }
    }

    void UICanvas::ProcessMouse()
    {
        bool is_pressed = false;
        bool just_pressed = false;

        for(int i=0; i<3; i++)
        {
            if(Input::GetMouseButtonDown(i))
            {
                g_current_mouse = i;
                just_pressed = true;
                is_pressed = true;
            }
            else if (Input::GetMouseButton(i))
            {
                g_current_mouse = i;
                is_pressed = true;
            }
        }

        g_current_touch = &g_mouse[0];

        Vector2 pos = Input::GetMousePosition();

        if(g_current_touch->ignore_delta == 0)
        {
            g_current_touch->delta = pos - g_current_touch->pos;
        }
        else
        {
            --g_current_touch->ignore_delta;
            g_current_touch->delta.x = 0.f;
            g_current_touch->delta.y = 0.f;
        }

        float sqr_mag = g_current_touch->delta.SqrMagnitude();
        g_current_touch->pos = pos;
        g_last_pos = pos;

        bool pos_changed = false;

        if(sqr_mag > 0.001f)
            pos_changed = true;

        for(int i=1; i<3; i++)
        {
            g_mouse[i].pos = g_current_touch->pos;
            g_mouse[i].delta = g_current_touch->delta;
        }

        if(is_pressed || pos_changed)
        {
            raycast(*g_current_touch);
            for(int i=0; i<3; i++)
                g_mouse[i].current = g_current_touch->current;
        }

        bool highlight_changed = (g_current_touch->last.lock() != g_current_touch->current.lock());
        bool was_pressed = (!g_current_touch->pressed.expired());

        if(!was_pressed)
            SetHoveredObject(g_current_touch->current);

        g_current_touch_id = -1;
        if(highlight_changed)
            g_current_mouse = 0;

        if(highlight_changed && (just_pressed || (was_pressed && !is_pressed)))
            SetHoveredObject(std::weak_ptr<GameObject>());

        for(int i=0; i<3; ++i)
        {
            bool pressed = Input::GetMouseButtonDown(i);
            bool unpressed = Input::GetMouseButtonUp(i);
            if (pressed || unpressed) g_current_mouse = i;
            g_current_touch = &g_mouse[i];

            g_current_touch_id = -1 - i;
            g_current_mouse = i;

            if(pressed)
            {
                g_current_touch->press_time = GTTime::GetRealTimeSinceStartup();
            }

            // Process the mouse events
            ProcessTouch(pressed, unpressed);
        }

        if(!is_pressed && highlight_changed)
        {
            g_current_touch = &g_mouse[0];
            g_current_touch_id = -1;
            g_current_mouse = 0;
            SetHoveredObject(g_current_touch->current);
        }
        g_current_touch = NULL;

        g_mouse[0].last = g_mouse[0].current;
        for(int i = 1; i < 3; ++i)
            g_mouse[i].last = g_mouse[0].last;
    }
}