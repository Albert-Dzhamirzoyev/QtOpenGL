

//  ravesli.com
//
//  Lesson #15
//

#include "renderwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RenderWindow *p_rWindow = new RenderWindow;

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    p_rWindow->setFormat(format);

    p_rWindow->showMaximized();

    return a.exec();
}
