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
    const float                         cm_cameraSpeedFactor = 0.003f;
    const float                         cm_mouseSensitivity = 0.008f;
    const float                         cm_wheelSensitivity = 0.001f;
    const QVector4D                     cm_clearColor = QVector4D(0.0f, 0.0f, 0.0f, 1.0f);

    unsigned int                        m_VBO, m_cubeVAO, m_lightVAO, m_EBO;
    unsigned int                        m_diffuseMap, m_specularMap, m_emissionMap;
    KeyboardState                       m_buttonsState;
    MouseState                          m_lastMouseState;

    float                               m_frameDelta = 0.0f;
    float                               m_lastFrameTime = 0.0f;

    std::vector<QOpenGLShader*>          mp_shadersList;

    QOpenGLShaderProgram*               mp_shaderProgLight;
    QOpenGLShaderProgram*               mp_shaderProgLamp;

    QMatrix4x4                          m_modelMatrix;
    QMatrix4x4                          m_viewMatrix;
    QMatrix4x4                          m_projectionMatrix;

    QCamera                             m_camera;

    Direction                           m_cameraDirection;
    QVector3D                           m_cameraPosition;
    QVector3D                           m_cameraFront;
    QVector3D                           m_cameraUp;

    QVector3D                           m_lightPos = QVector3D(1.2f, 1.0f, 2.0f);
    QVector3D                           m_lightDir = QVector3D(-0.2f, -1.0f, -0.3f);
    std::vector<QVector3D>              m_cubePositions;
    std::vector<QVector3D>              m_pointLightPositions;

    QElapsedTimer                       m_frameTimer;
public:
    RenderWindow(/*QOpenGLContext *shareContext*/);
    virtual ~RenderWindow() override;
protected:
    QOpenGLShaderProgram* loadShaders(const QString &vertexShaderFileName, const QString &fragmentShaderFileName);
    void loadTexture(unsigned int * p_texture, const QString &texture_FileName);
    void processInput();
    void defineFrameDelta();
    void processModels();

    void initializeGL()                         override;
    void resizeGL(int width, int height)        override;
    void paintGL()                              override;

    void keyPressEvent(QKeyEvent *p_key)        override;
    void keyReleaseEvent(QKeyEvent *p_key)      override;
    void mouseMoveEvent(QMouseEvent *p_mouse)   override;
    void wheelEvent(QWheelEvent *p_wheel)       override;
};

#endif // RENDERWINDOW_H
