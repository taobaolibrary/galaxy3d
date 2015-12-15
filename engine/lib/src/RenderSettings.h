#ifndef __RenderSettings_h__
#define __RenderSettings_h__

#include "Color.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace Galaxy3D
{
    class RenderSettings
    {
    public:
        static Color light_ambient;
        static Color light_directional_color;
        static Quaternion light_directional_rotation;
        static float light_directional_intensity;
    };
}

#endif