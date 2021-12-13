#ifndef MATERIALS_H
#define MATERIALS_H

#include <QVector3D>

struct Materials
{
    QVector3D   ambient;
    QVector3D   diffuse;
    QVector3D   specular;
    float       shininess;
};

namespace MatLib
{
    Materials   obsidian{QVector3D(0.05375f, 0.05f, 0.06625f),
                         QVector3D(0.18275f, 0.17f, 0.22525f),
                         QVector3D(0.332741f, 0.328634f, 0.346435f), 32.0f};

    Materials   ruby{QVector3D(0.1745f, 0.01175f, 0.01175f),
                     QVector3D(0.61424f, 0.04136f, 0.04136f),
                     QVector3D(0.727811f, 0.626959f, 0.626959f), 64.0f};

    Materials   emerald{QVector3D(0.0215f, 0.1745f, 0.0215f),
                        QVector3D(0.07568f, 0.61424f, 0.07568f),
                        QVector3D(0.633f, 0.727811f, 0.633f), 64.0f};
};
#endif // MATERIALS_H
