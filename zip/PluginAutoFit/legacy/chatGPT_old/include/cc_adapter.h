#pragma once

// Pas onderstaande includes aan naar jouw lokale CloudCompare pad.
// Deze adapter biedt minimale typedefs/bridge die in h_do_optimize_frame3.cpp worden gebruikt.

#include <vector>
#include <string>

struct CCVector3 {
float x, y, z;
};

struct CCPointCloud {
std::vector<CCVector3> points;
size_t size() const { return points.size(); }
const CCVector3* getPoint(size_t i) const { return &points[i]; }
void getBoundingBox(CCVector3& mn, CCVector3& m_x) const {
if (points.empty()){ mn={0,0,0}; m_x={0,0,0}; return; }
mn = m_x = points[0];
for (auto &p: points){
if (p.x<mn.x) mn.x=p.x; if (p.y<mn.y) mn.y=p.y; if (p.z<mn.z) mn.z=p.z;
if (p.x>m_x.x) m_x.x=p.x; if (p.y>m_x.y) m_x.y=p.y; if (p.z>m_x.z) m_x.z=p.z;
}
}
};

// In jouw plugin: vervang bovenstaande dummy door echte CC includes,
// of maak een converteerfunctie die jouw CC cloud omzet naar deze dummy-structs.