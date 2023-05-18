#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>

namespace SketchBook
{
namespace View
{
   struct WorldPoint
   {
        float x, y, h;
   };

   struct WorldBox
   {
        WorldPoint minCorner, maxCorner;
   };
}
}

BOOST_GEOMETRY_REGISTER_POINT_3D(SketchBook::View::WorldPoint, float, cs::cartesian, x, y, h);
BOOST_GEOMETRY_REGISTER_BOX(SketchBook::View::WorldBox, SketchBook::View::WorldPoint, minCorner, maxCorner);