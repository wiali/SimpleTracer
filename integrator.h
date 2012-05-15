#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "QImage"
#include "raytracer.h"
#include "pathtracer.h"

class Integrator
{
public:
    Integrator(int width,int height,int depth);
    QImage RayTrace();
    QImage PathTrace();
    virtual ~Integrator();

private:
    int width;
    int height;
    int depth;

    Pathtracer *PT;
    RayTracer *RT;
};

#endif // INTEGRATOR_H