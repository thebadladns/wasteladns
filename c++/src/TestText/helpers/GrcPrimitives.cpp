//#include "GrcPrimitives.h"
//
//#include "Vector.h"
//
//#define GLFW_INCLUDE_GLU
//#include "GLFW/glfw3.h"


//template <> void GrcPrimitives::segment<float>(const Vector3<float>& vStart, const Vector3<float>& vEnd, Col color) {
//    glBegin(GL_LINES);
//        glColor4f(color.getRf(), color.getGf(), color.getBf(), color.getAf());
//        glVertex3f(vStart.x, vStart.y, vStart.z);
//        glVertex3f(vEnd.x, vEnd.y, vEnd.z);
//    glEnd();
//}
//
//template <> void GrcPrimitives::segment<double>(const Vector3<double>& vStart, const Vector3<double>& vEnd, Col color) {
//    glBegin(GL_LINES);
//        glColor4f(color.getRf(), color.getGf(), color.getBf(), color.getAf());
//        glVertex3d(vStart.x, vStart.y, vStart.z);
//        glVertex3d(vEnd.x, vEnd.y, vEnd.z);
//    glEnd();
//}
