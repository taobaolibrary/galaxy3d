#ifndef __Component_h__
#define __Component_h__

#include "Object.h"
#include "ComponentClassMap.h"

namespace Galaxy3D
{
	class Transform;
	class GameObject;

	class Component : public Object
	{
        DECLARE_COM_BASE(Component);

    private:
		friend class GameObject;

	public:
		static void Destroy(std::shared_ptr<Component> &com);
        virtual ~Component() {}
		virtual void SetName(const std::string &value);
		std::shared_ptr<GameObject> GetGameObject() const;
		std::shared_ptr<Transform> GetTransform() const;
        std::shared_ptr<Component> GetComponentPtr() const;
		void Enable(bool enable);
		bool IsEnable() const {return m_enable;}
		bool IsStarted() const {return m_started;}

	protected:
		std::weak_ptr<GameObject> m_gameobject;
		std::weak_ptr<Transform> m_transform;

		Component();
		//没有OnDestroy，用析构函数
        virtual void Awake() {}
		virtual void Start() {}
		virtual void Update() {}
		virtual void LateUpdate() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
        virtual void OnTranformChanged() {}
        virtual void OnWillRenderObject(int material_index) {}
        virtual void OnDidRenderObject(int material_index) {}

	private:
		bool m_deleted;
		bool m_started;
		bool m_enable;

		void Delete();
	};
}

#endif