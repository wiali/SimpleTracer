#include "pathtracer.h"


Pathtracer::Pathtracer(int x, int y, uint newDepth, World *newWorld)
{
    renderImage = new QImage(x, y, QImage::Format_ARGB32);
    world = newWorld;
    depth = newDepth;

    for(int yPix = 0; yPix < y; yPix++)
    {
        QVector<renderPixel> Row;
        for(int xPix = 0; xPix < x; xPix++)
        {
            renderPixel currPix;
            currPix.r = 1.0;
            currPix.g = 1.0;
            currPix.b = 1.0;
            currPix.i = 0;
            Row.push_back(currPix);
        }
        RenderedSamples.push_back(Row);
    }
}

QVector<QVector<renderPixel> > Pathtracer::render(){

        for(int y = 0; y < world->getCamera()->getImgHeigth(); y++){
            for(int x = 0; x < world->getCamera()->getImgWidth(); x++){

                Ray ray = world->getCamera()->shootRay(x+(qrand()%1000)/500.0-1.0, y+(qrand()%1000)/500.0-1.0);
                QVector3D ColorAtPixel = tracer(ray, 0);
                RenderedSamples[x][y].r = ColorAtPixel.x();
                RenderedSamples[x][y].g = ColorAtPixel.y();
                RenderedSamples[x][y].b = ColorAtPixel.z();
            }
        }

    return RenderedSamples;
}


QColor Pathtracer::clampColor(QVector3D hdr)
{
    //clamping
    int r = hdr.x() * 255;
    int g = hdr.y() * 255;
    int b = hdr.z() * 255;

    return QColor(r, g, b);
}


QVector3D Pathtracer::tracer(Ray ray, uint current_depth){
    /*****************************************************************************************************************/
    double nearestDist = world->getCamera()->getFarClip(); //far clipping border of the scene

    Sphere Test(QVector3D(0,0,0),1);
    Material TestMat;

    Object nearestObj(&Test,&TestMat);
    Object obj(&Test, &TestMat);
    Object obstacle(&Test, &TestMat);

    QVector3D Accumulated_Color = QVector3D(0, 0, 0);   //the color of the pixel, stored in a QVector3D
                                                        //gets accumulated in the following process
    /*****************************************************************************************************************/
    //find closest intersection (can be accelerated extremely lateron!)
    for(int i = 0; i < world->getObjects().size(); i++){
        obj = *world->getObjects().at(i);
        Geometry::IntersectionInfo IntersectInfo = obj.getMesh()->getIntersectionInfo(ray);

        if(IntersectInfo.hit == true){

            if(IntersectInfo.distance < nearestDist){
                nearestDist = IntersectInfo.distance;
                nearestObj = Object(obj.getMesh(), obj.getMat());
            }
        }
    }

    //not physically correct, has to be extended or replaced later (maybe with oren-nayar implementation?)

    if(nearestDist == world->getCamera()->getFarClip())
        return Accumulated_Color += world->getBgColor(); //background color

    //hitpoint on the object's surface
    QVector3D hitpoint = ray.getOrigin() + ray.getDirection() * nearestDist;
    QVector3D normal = (nearestObj.getMesh()->getNormal(hitpoint)).normalized();

    /*****************************************************************************************************************/
    //trace lights

        /*****************************************************************************************************************/
        //calculate emit shading
        if((current_depth < depth) && (nearestObj.getMat()->getEmit() > 0.0f) ){

            Accumulated_Color = nearestObj.getMat()->getDiffuseColor() * nearestObj.getMat()->getEmit();
        }


        /*****************************************************************************************************************/
        //calculate diffuse shading
        //dotproduct of lightray and surface normal
        /*
        if(nearestObj->getMat()->getDiffuseColor().length() > 0.0f){
            double dotLN = fabs(QVector3D::dotProduct(lightray, normal));

            if(dotLN > 0){
                //get the color the surface has at the hitpoint and multiply it with the shadowing factor
                QVector3D diffuseCol = dotLN * nearestObj->getMat()->getDiffuseColor() * shade;
                //add the diffuse color to the ray's color
                //square light falloff
                //Accumulated_Color += (diffuseCol * ((light->getColor() * light->getIntensity())/pow((lightrayLength + EPSILON), 2)))/100; //first implementation, only direct lighting
                //linear light falloff
                Accumulated_Color += (diffuseCol * ((light->getColor() * light->getIntensity())/(lightrayLength + EPSILON)))/100;
            }
        }
        */

        /*****************************************************************************************************************/
        //calculate specular shading
        /*
        if(nearestObj->getMat()->getSpecularColor().length() > 0.0f){
            QVector3D Incoming_Vector = ray.getDirection();
            QVector3D Reflected_Vector = lightray - 2 * QVector3D::dotProduct(lightray, normal) * normal; //normalized by design
            double dotIR = QVector3D::dotProduct(Incoming_Vector, Reflected_Vector);

            if(dotIR > 0){
                double specularAmount = powf(dotIR, nearestObj->getMat()->getExponent()) * nearestObj->getMat()->getSpecularColor().length()/100;
                //add the specular color to the ray's color
                Accumulated_Color += (specularAmount * light->getColor() * light->getIntensity());
            }
        }*/

        /*****************************************************************************************************************/
        //calculate diffuse Reflection
        if((current_depth < depth) && (nearestObj.getMat()->getReflectionAmount() > 0.0f) && (nearestObj.getMat()->getTransparency())){
            QVector3D Reflected_Vector;
            Reflected_Vector = normal; //Normals have to be calculated  in right direction to work with this
            Reflected_Vector.setX(Reflected_Vector.x() + (qrand()%2000)/1000.0 - 1.0);
            Reflected_Vector.setY(Reflected_Vector.y() + (qrand()%2000)/1000.0 - 1.0);
            Reflected_Vector.setZ(Reflected_Vector.z() + (qrand()%2000)/1000.0 - 1.0);
            QVector3D Reflected_Color = tracer(Ray(hitpoint + Reflected_Vector * EPSILON, Reflected_Vector), current_depth + 1);
            Accumulated_Color = Reflected_Color * nearestObj.getMat()->getDiffuseColor() * nearestObj.getMat()->getReflectionAmount();
        }

        /*****************************************************************************************************************/
        //calculate reflection
      if((current_depth < depth) && (nearestObj.getMat()->getReflectionAmount() > 0.0f) && (!nearestObj.getMat()->getTransparency()) ){
            QVector3D Reflected_Vector = ray.getDirection() - 2 * QVector3D::dotProduct(ray.getDirection(), normal) * normal; //normalized by design
            QVector3D Reflected_Color = tracer(Ray(hitpoint + Reflected_Vector * EPSILON, Reflected_Vector), current_depth + 1);
            Accumulated_Color = Reflected_Color * nearestObj.getMat()->getSpecularColor();
        }


        /*****************************************************************************************************************/
        //calculate refraction
/*        if((current_depth < depth) && (nearestObj->getMat()->getTransparency() == true)){
            QVector3D Refracted_Vector;
            QVector3D normal = nearestObj->getMesh()->getNormal(hitpoint);
            double n = world->getIoR()/nearestObj->getMat()->getIoR();
            double cosI = QVector3D::dotProduct(normal, ray.getDirection());
            double sinT2 = n*n * (1.0 - cosI*cosI);
            if(sinT2 <= 1.0)
            {
                Refracted_Vector = n * ray.getDirection() - (n + sqrtf(1.0 - sinT2)) * normal;
                QVector3D Refracted_Color = raytrace(Ray(hitpoint + Refracted_Vector * EPSILON, Refracted_Vector), current_depth + 1);
                Accumulated_Color = Refracted_Color; //maybe add a blending factor with the diffuse color later
            }
        }*/

    return Accumulated_Color;
}




Pathtracer::~Pathtracer()
{
    delete world;
    delete renderImage;
}
