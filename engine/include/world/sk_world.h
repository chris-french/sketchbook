#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <boost/geometry.hpp>
#include "sk_geometry.h"
#include <unordered_map>
#include <limits>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace SketchBook
{
namespace View
{

    /* Specs:
    - [ ] WorldObject struct with box and id values
    - [ ] create r-tree pairs of <box,id> tuples
    - [ ] store 
    
    */

   struct WorldObject
   {
        glm::vec3 position = {};
        glm::vec3 min_corner = {};
        glm::vec3 max_corner = {};
        int id;
        void* obj = nullptr;
        
        void update_position(const glm::vec3& delta)
        {
            position += delta;
            min_corner += delta;
            max_corner += delta;
        };
   };

   class World2dView
   {

   };

   class World2dController
   {

   };

    class World2dModel
    {
        glm::vec3 view_boundary = {1.f, 1.f, 1.f};
        glm::vec3 world_origin = {0.f, 0.f, 0.f};
        glm::vec3 corner_extent = {1.f, 1.f, 1.f};

        std::unordered_map<int, WorldObject> objects;

        /*
        Use boost to implement an r-tree for geo queries and item display (for infinite world)?
        - https://stackoverflow.com/questions/42182537/how-can-i-use-the-rtree-of-the-boost-library-in-c
        */
    };
}
}

/*

// https://stackoverflow.com/a/42253289

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <iostream>
#include <limits>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

class LatLon
{
public:
    LatLon(){}
    explicit LatLon(float lat_, float lon_) : m_lat(lat_),m_lon(lon_){}

    float lat() const { return m_lat; }
    float lon() const { return m_lon; }

private:
    float m_lat = std::numeric_limits<float>::quiet_NaN();
    float m_lon = std::numeric_limits<float>::quiet_NaN();
};

struct MyLatLon
{
    MyLatLon() {}
    MyLatLon(float lat_, float lon_) : ll(lat_, lon_){}

    float get_lat() const { return ll.lat(); }
    float get_lon() const { return ll.lon(); }
    void set_lat(float v) { ll = LatLon(v, ll.lon()); }
    void set_lon(float v) { ll = LatLon(ll.lat(), v); }

    LatLon ll;
};

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(MyLatLon, float,
                                         bg::cs::spherical_equatorial<bg::degree>,
                                         get_lon, get_lat, set_lon, set_lat)

int main()
{
    typedef std::pair<MyLatLon, unsigned> point_pair;

    bgi::rtree<point_pair, bgi::quadratic<16>> intersectionRTree;

    intersectionRTree.insert(std::make_pair(MyLatLon(0, 0), 0));
    intersectionRTree.insert(std::make_pair(MyLatLon(2, 2), 1));

    bg::model::box<MyLatLon> b(MyLatLon(1, 1), MyLatLon(3, 3));
    std::vector<point_pair> result1;
    intersectionRTree.query(bgi::intersects(b), std::back_inserter(result1));
    if (! result1.empty())
        std::cout << bg::wkt(result1[0].first) << std::endl;

    std::vector<point_pair> result2;
    intersectionRTree.query(bgi::nearest(MyLatLon(0, 1), 1), std::back_inserter(result2));
    if (! result2.empty())
        std::cout << bg::wkt(result2[0].first) << std::endl;
}
*/