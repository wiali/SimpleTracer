#ifndef RAY_H
#define RAY_H

#include <QVector3D>

class Ray
{
public:
    Ray(QVector3D newOrig, QVector3D newDir){Direction = newDir.normalized(); Origin = newOrig;}

    QVector3D getDirection(){return Direction;}
    QVector3D getOrigin(){return Origin;}

    void setDirection(QVector3D newDir){Direction = newDir;}
    void setOrigin(QVector3D newOrig){Origin = newOrig;}
    QVector3D Direction;

private:
    QVector3D Origin;
};

#endif // RAY_H
