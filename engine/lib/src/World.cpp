#include "World.h"
#include "GTTime.h"

namespace Galaxy3D
{
	std::unordered_map<GameObject *, std::shared_ptr<GameObject>> World::m_gameobjects;
	std::unordered_map<GameObject *, std::shared_ptr<GameObject>> World::m_gameobjects_new;

	void World::AddGameObject(const std::shared_ptr<GameObject> &obj)
	{
		m_gameobjects_new[obj.get()] = obj;
	}

	std::weak_ptr<GameObject> World::FindGameObject(GameObject *obj)
	{
		auto find = m_gameobjects.find(obj);
		if(find != m_gameobjects.end())
		{
			return find->second;
		}

		find = m_gameobjects_new.find(obj);
		if(find != m_gameobjects_new.end())
		{
			return find->second;
		}

		return std::weak_ptr<GameObject>();
	}

	void World::Update()
	{
		float time = GTTime::GetRealTimeSinceStartup();

		//	start
		std::unordered_map<GameObject *, std::shared_ptr<GameObject>> starts(m_gameobjects);
		do
		{
			for(auto &i : starts)
			{
				auto &obj = i.second;

				if(obj->IsActiveInHierarchy())
				{
					i.second->Start();
				}
			}
			starts.clear();

			starts = m_gameobjects_new;
			m_gameobjects.insert(m_gameobjects_new.begin(), m_gameobjects_new.end());
			m_gameobjects_new.clear();
		}while(!starts.empty());

		//	update
		for(auto &i : m_gameobjects)
		{
			auto &obj = i.second;

			if(obj->IsActiveInHierarchy())
			{
				i.second->Update();
			}
		}

		//	late update
		for(auto &i : m_gameobjects)
		{
			auto &obj = i.second;

			if(obj->IsActiveInHierarchy())
			{
				i.second->LateUpdate();
			}
		}

		//	delete
		auto it = m_gameobjects.begin();
		while(it != m_gameobjects.end())
		{
			if(it->second->m_deleted)
			{
				it = m_gameobjects.erase(it);
			}
			else
			{
				it++;
			}
		}

		GTTime::m_update_time = GTTime::GetRealTimeSinceStartup() - time;
	}

	void World::Destroy()
	{
		m_gameobjects_new.clear();
		m_gameobjects.clear();
	}
}