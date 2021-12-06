#include <QOpenGLWindow>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QVector3D>
#include <Qt3DRender/QCamera>

#include <QElapsedTimer>

#include <keyboard_state.h>
#include <mouse_state.h>
#include <direction.h>


#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

using namespace Qt3DRender;

class RenderWindow : public QOpenGLWindow, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
private:
    const float                         cm_cameraSpeedFactor = 0.006f;
    const float                         cm_mouseSensitivity = 0.008f;
    const float                         cm_wheelSensitivity = 0.001f;

    unsigned int                        m_VBO, m_VAO, m_EBO;
    unsigned int                        m_texture1, m_texture2;
    KeyboardState                       m_buttonsState;
    MouseState                          m_lastMouseState;

    float                               m_frameDelta = 0.0f;
    float                               m_lastFrameTime = 0.0f;

    std::vector<QVector3D>              m_cubePositions;

    QOpenGLShader                       *mp_vertexShader;
    QOpenGLShader                       *mp_fragmentShader;
    QOpenGLShaderProgram                *mp_shaderProg;

    QMatrix4x4                          m_modelMatrix;
    QMatrix4x4                          m_viewMatrix;
    QMatrix4x4                          m_projectionMatrix;

    QCamera                             m_camera;

    Direction                           m_cameraDirection;
    QVector3D                           m_cameraPosition;
    QVector3D                           m_cameraFront;
    QVector3D                           m_cameraUp;

    QElapsedTimer                       m_frameTimer;
public:
    RenderWindow(/*QOpenGLContext *shareContext*/);
    ~RenderWindow();
protected:
    void loadShaders(const QString &vertexShaderFileName, const QString &fragmentShaderFileName);
    void loadTextures(const QString &texture_1FileName, const QString &texture_2FileName);
    void processInput();
    void defineFrameDelta();

    void initializeGL()                         override;
    void resizeGL(int width, int height)        override;
    void paintGL()                              override;

    void keyPressEvent(QKeyEvent *p_key)        override;
    void keyReleaseEvent(QKeyEvent *p_key)      override;
    void mouseMoveEvent(QMouseEvent *p_mouse)   override;
    void wheelEvent(QWheelEvent *p_wheel)       override;
};

#endif // RENDERWINDOW_H
