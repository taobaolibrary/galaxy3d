#ifndef __Sprite_h__
#define __Sprite_h__

#include "Object.h"
#include "Texture2D.h"
#include "Rect.h"
#include "Vector2.h"
#include "Vector4.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
	class Sprite : Object
	{
    public:
        struct Type
        {
            enum Enum
            {
                Simple,
                Sliced,
                Tiled,
                Filled
            };
        };

        struct FillDirection
        {
            enum Enum
            {
                Horizontal,
                Vertical,
                Radial_90,
                Radial_180,
                Radial_360
            };
        };

		static std::shared_ptr<Sprite> LoadFromFile(const std::string &file);
		static std::shared_ptr<Sprite> Create(const std::shared_ptr<Texture2D> &texture);
		//	rect:指定原贴图上的像素矩形区域，左上为起点
		//	pivot:中心点，相对于sprite rect，(0，0)为左上角，（1，1）为右下角
		//	pixels_per_unit:The number of pixels in the sprite that correspond to one unit in world space. (Read Only)
		//	border:the border sizes of the sprite, x y z w : left top right bottom
		static std::shared_ptr<Sprite> Create(
			const std::shared_ptr<Texture2D> &texture,
			const Rect &rect,
			const Vector2 &pivot,
			float pixels_per_unit,
			const Vector4 &border,
            Type::Enum type,
            Vector2 size);
        virtual ~Sprite();
        BufferObject GetVertexBuffer();
        BufferObject GetIndexBuffer();
        int GetVertexCount() const {return m_vertices.size();}
        Vector2 *GetVertices() {if(!m_vertices.empty()){return &m_vertices[0];}else{return NULL;}}
		Vector2 *GetUV() {if(!m_uv.empty()){return &m_uv[0];}else{return NULL;}}
        int GetIndexCount() const {return m_triangles.size();}
		unsigned short *GetIndices() {if(!m_triangles.empty()){return &m_triangles[0];}else{return NULL;}}
		std::shared_ptr<Texture2D> GetTexture() const {return m_texture;}

	private:
		std::shared_ptr<Texture2D> m_texture;
		Rect m_rect;
		Vector2 m_pivot;
		float m_pixels_per_unit;
		Vector4 m_border;
        Type::Enum m_type;
        Vector2 m_size;
        float m_fill_amount;
        FillDirection::Enum m_fill_direction;
        std::vector<Vector2> m_vertices;
        std::vector<Vector2> m_uv;
        std::vector<unsigned short> m_triangles;
        BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;

		Sprite();
        void FillMeshSimple();
        void FillMeshSliced();
        void FillMeshTiled();
        void FillMeshFilled();
	};
}

#endif