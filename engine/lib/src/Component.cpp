#include "Component.h"
#include "GameObject.h"

namespace Galaxy3D
{
	Component::Component():
		m_deleted(false),
		m_started(false),
		m_enable(true)
	{
	}

	void Component::Destroy(std::shared_ptr<Component> &com)
	{
		if(com)
		{
            com->Delete();
            com.reset();
		}
	}

    void Component::DeepCopy(std::shared_ptr<Object> &source)
    {
        Object::DeepCopy(source);

        auto com_src = std::dynamic_pointer_cast<Component>(source);
        m_deleted = com_src->m_deleted;
        m_started = com_src->m_started;
        m_enable = com_src->m_enable;
    }

	void Component::Delete()
	{
		if(!m_deleted)
		{
			m_deleted = true;
			Enable(false);
		}
	}

	void Component::Enable(bool enable)
	{
		if(m_enable != enable)
		{
			m_enable = enable;

			bool obj_active = GetGameObject()->IsActiveInHierarchy();
			if(obj_active)
			{
				if(m_enable)
				{
					OnEnable();
				}
				else
				{
					OnDisable();
				}
			}
		}
	}

    std::shared_ptr<Component> Component::GetComponentPtr() const
    {
        return GetGameObject()->GetComponentPtr(this);
    }
}