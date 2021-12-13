#include "renderwindow.h"

#include <QApplication>
#include <QtDebug>
#include <QFile>

#include <cassert>
#include <math.h>
#define PI 3.14159265f
#include "stb_image.h"

#include <materials.h>


RenderWindow::RenderWindow(/*QOpenGLContext *shareContext*/)
    : QOpenGLWindow(/*shareContext, QOpenGLWindow::NoPartialUpdate*/),
      mp_shaderProgLight(nullptr),
      mp_shaderProgLamp(nullptr)
{
    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setCursor(QCursor(Qt::BlankCursor));
}

RenderWindow::~RenderWindow()
{
    delete mp_shaderProgLight;
    delete mp_shaderProgLamp;

    for(auto p_shader: mp_shadersList)
    {
        delete p_shader;
    }

    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteVertexArrays(1, &m_lightVAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

QOpenGLShaderProgram *RenderWindow::loadShaders(const QString& vertexShaderFileName, const QString& fragmentShaderFileName)
{
    QOpenGLShader * p_vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    mp_shadersList.push_back(p_vertexShader);
    QOpenGLShader * p_fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    mp_shadersList.push_back(p_fragmentShader);
    QOpenGLShaderProgram * p_shaderProg = new QOpenGLShaderProgram;


    QFile vertex(vertexShaderFileName);
    assert(vertex.open(QIODevice::ReadOnly) && "Vertex shader file opening failed!");
    QByteArray vertexSource = vertex.readAll();

    QFile fragment(fragmentShaderFileName);
    assert(fragment.open(QIODevice::ReadOnly) && "Fragment shader file opening failed!");
    QByteArray fragmentSource = fragment.readAll();

    vertex.close();
    fragment.close();


    if (!p_vertexShader->compileSourceCode(vertexSource))
        {
            qDebug() << "Vertex shader compilation failed!\n" << p_vertexShader->log();
        }
    if (!p_fragmentShader->compileSourceCode(fragmentSource))
        {
            qDebug() << "Fragment shader compilation failed!\n" << p_fragmentShader->log();
        }
    p_shaderProg->addShader(p_vertexShader);
    p_shaderProg->addShader(p_fragmentShader);

    assert(p_shaderProg->link() && "ShaderProgram: Linking Failed!");

    //qDebug() << p_shaderProg->isLinked();

    return p_shaderProg;

}

void RenderWindow::loadTexture(unsigned int *p_texture, const QString &texture_FileName)
{
    glGenTextures(1, p_texture);

    glBindTexture(GL_TEXTURE_2D, *p_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, *p_texture);
    int width, height, nrChannels;
    unsigned char *p_data = stbi_load(texture_FileName.toStdString().data(), &width, &height, &nrChannels, 0);

    if (p_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, p_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        qDebug() << "Failed to load texture!";
    }
    stbi_image_free(p_data);
    p_data = nullptr;
}

void RenderWindow::initializeGL()
{
    // Set up the rendering context, load shaders and other resources, etc.:
//    qDebug() << context();

    initializeOpenGLFunctions();

    m_frameTimer.start();
#ifdef Q_OS_WINDOWS
    mp_shaderProgLight = loadShaders(QString(QApplication::applicationDirPath() + QString("\\shaders\\light_casters.vs")),
                QString(QApplication::applicationDirPath() + QString("\\shaders\\light_casters.fs")));
    mp_shaderProgLamp = loadShaders(QString(QApplication::applicationDirPath() + QString("\\shaders\\lamp.vs")),
                QString(QApplication::applicationDirPath() + QString("\\shaders\\lamp.fs")));

    loadTexture(&m_diffuseMap ,QString(QApplication::applicationDirPath() + QString("\\textures\\box_metal.jpg")));
    loadTexture(&m_specularMap ,QString(QApplication::applicationDirPath() + QString("\\textures\\box_edging.jpg")));
    loadTexture(&m_emissionMap ,QString(QApplication::applicationDirPath() + QString("\\textures\\matrix.jpg")));
#endif

#ifdef Q_OS_UNIX
    mp_shaderProgLight = loadShaders(QString(QApplication::applicationDirPath() + QString("/shaders/light_casters.vs")),
                QString(QApplication::applicationDirPath() + QString("/shaders/light_casters.fs")));
    mp_shaderProgLamp = loadShaders(QString(QApplication::applicationDirPath() + QString("/shaders/lamp.vs")),
                QString(QApplication::applicationDirPath() + QString("/shaders/lamp.fs")));

    loadTexture(&m_diffuseMap ,QString(QApplication::applicationDirPath() + QString("/textures/box_metal.jpg")));
    loadTexture(&m_specularMap ,QString(QApplication::applicationDirPath() + QString("/textures/box_edging.jpg")));
#endif

    glClearColor(cm_clearColor.x(),
                 cm_clearColor.y(),
                 cm_clearColor.z(),
                 cm_clearColor.w());

    glEnable(GL_DEPTH_TEST);

    m_camera.setProjectionType(QCameraLens::PerspectiveProjection);
    m_camera.setPosition(QVector3D(0.0f, 0.0f, 3.0f));
    m_camera.setViewCenter(QVector3D(0.0f, 0.0f, -1.0f));
    m_camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    processModels();
}

void RenderWindow::resizeGL(int width, int height)
{
    // Update projection matrix and other size related settings:
    glViewport(0, 0, width, height);

    m_lastMouseState.lastX = width/2;
    m_lastMouseState.lastY = height/2;
    QCursor::setPos(mapToGlobal(QPoint(width/2, height/2)));

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(m_lastMouseState.fov, (float)width/(float)height, 0.1f, 100.0f);
    m_camera.setProjectionMatrix(m_projectionMatrix);

}

void RenderWindow::keyPressEvent(QKeyEvent *p_key)
{
    if (p_key->key() == Qt::Key_Escape)
        QApplication::quit();
    if (p_key->key() == Qt::Key_W)
        m_buttonsState.W_keyPressed = true;
    if (p_key->key() == Qt::Key_S)
        m_buttonsState.S_keyPressed = true;
    if (p_key->key() == Qt::Key_A)
        m_buttonsState.A_keyPressed = true;
    if (p_key->key() == Qt::Key_D)
        m_buttonsState.D_keyPressed = true;
    if (p_key->key() == Qt::Key_Q)
        m_buttonsState.Q_keyPressed = true;
    if (p_key->key() == Qt::Key_E)
        m_buttonsState.E_keyPressed = true;
    if (p_key->key() == Qt::Key_L)
        m_buttonsState.Light_key_activated = !m_buttonsState.Light_key_activated;

}

void RenderWindow::keyReleaseEvent(QKeyEvent *p_key)
{
    if (p_key->key() == Qt::Key_W)
        m_buttonsState.W_keyPressed = false;
    if (p_key->key() == Qt::Key_S)
        m_buttonsState.S_keyPressed = false;
    if (p_key->key() == Qt::Key_A)
        m_buttonsState.A_keyPressed = false;
    if (p_key->key() == Qt::Key_D)
        m_buttonsState.D_keyPressed = false;
    if (p_key->key() == Qt::Key_Q)
        m_buttonsState.Q_keyPressed = false;
    if (p_key->key() == Qt::Key_E)
        m_buttonsState.E_keyPressed = false;
}

void RenderWindow::mouseMoveEvent(QMouseEvent * p_mouse)
{

    float cameraSpeed = cm_mouseSensitivity * m_frameDelta;

#ifdef QT_DEPRECATED_VERSION_5
    float xOffset = p_mouse->globalPos().x() - m_lastMouseState.lastX;
    float yOffset = m_lastMouseState.lastY - p_mouse->globalPos().y();
#elif
    float xOffset = p_mouse->globalPosition().x() - m_lastMouseState.lastX;
    float yOffset = m_lastMouseState.lastY - p_mouse->globalPosition().y();
#endif

    m_lastMouseState.lastX = mapToGlobal(QPoint(width()/2, height()/2)).x();
    m_lastMouseState.lastY = mapToGlobal(QPoint(width()/2, height()/2)).y();
    QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));

    xOffset *= cameraSpeed;
    yOffset *= cameraSpeed;

    m_cameraDirection.yaw += xOffset;
    m_cameraDirection.pitch += yOffset;

    if (m_cameraDirection.pitch > 89.0f)
    {
        m_cameraDirection.pitch =  89.0f;
        yOffset = 0.0f;
    }

    if (m_cameraDirection.pitch < -89.0f)
    {
        m_cameraDirection.pitch = -89.0f;
        yOffset = 0.0f;
    }

//Yaw rotation
    m_camera.pan(xOffset);
//Pitch rotation
    m_camera.tilt(yOffset);

}

void RenderWindow::wheelEvent(QWheelEvent *p_wheel)
{
    float cameraSpeed = cm_wheelSensitivity * m_frameDelta;

        m_lastMouseState.fov -= p_wheel->angleDelta().y() * cameraSpeed;
    if(m_lastMouseState.fov < 5.0f)
        m_lastMouseState.fov = 5.0f;
    if(m_lastMouseState.fov > 45.0f)
        m_lastMouseState.fov = 45.0f;
}



void RenderWindow::paintGL()
{
    defineFrameDelta();
    processInput();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_camera.position(),
                        m_camera.position() + m_camera.viewVector(),
                        m_camera.upVector());

//--------------------------------------------------------------------------------------------------------------
    mp_shaderProgLight->bind();

    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("view"), m_viewMatrix);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("projection"), m_projectionMatrix);

    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("material.diffuse"), 0);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("material.specular"), 1);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("material.shininess"), 64.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("viewPos"), m_camera.position());


    // Direct light
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("dirLight.direction"),
                                                                            QVector3D(-0.2f, -1.0f, -0.3f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("dirLight.ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("dirLight.diffuse"),
                                                                            QVector3D(0.4f, 0.4f, 0.4f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("dirLight.specular"),
                                                                            QVector3D(0.5f, 0.5f, 0.5f));

    // Point light №1
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].position"),
                                                                            m_pointLightPositions[0]);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].diffuse"),
                                                                            QVector3D(0.8f, 0.8f, 0.8f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].specular"),
                                                                            QVector3D(1.0f, 1.0f, 1.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].constant"), 1.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].linear"), 0.09f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[0].quadratic"), 0.032f);

    // Point light №2
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].position"),
                                                                            m_pointLightPositions[1]);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].diffuse"),
                                                                            QVector3D(0.8f, 0.8f, 0.8f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].specular"),
                                                                            QVector3D(1.0f, 1.0f, 1.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].constant"), 1.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].linear"), 0.09f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[1].quadratic"), 0.032f);

    // Point light №3
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].position"),
                                                                            m_pointLightPositions[2]);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].diffuse"),
                                                                            QVector3D(0.8f, 0.8f, 0.8f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].specular"),
                                                                            QVector3D(1.0f, 1.0f, 1.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].constant"), 1.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].linear"), 0.09f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[2].quadratic"), 0.032f);

    // Point light №4
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].position"),
                                                                            m_pointLightPositions[3]);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].diffuse"),
                                                                            QVector3D(0.8f, 0.8f, 0.8f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].specular"),
                                                                            QVector3D(1.0f, 1.0f, 1.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].constant"), 1.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].linear"), 0.09f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("pointLights[3].quadratic"), 0.032f);

    // Torch
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.position"), m_camera.position());
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.direction"), m_camera.viewVector());
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.cutOff"), cos(12.5f * PI/180.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.outerCutOff"), cos(17.5f * PI/180.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.activated"), m_buttonsState.Light_key_activated);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.ambient"),
                                                                            QVector3D(0.05f, 0.05f, 0.05f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.diffuse"),
                                                                            QVector3D(0.8f, 0.8f, 0.8f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.specular"),
                                                                            QVector3D(1.0f, 1.0f, 1.0f));
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.constant"), 1.0f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.linear"), 0.09f);
    mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("spotLight.quadratic"), 0.032f);
//--------------------------------------------------------------------------------------------------------------

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_specularMap);

    glBindVertexArray(m_cubeVAO);
    for (unsigned int i = 0; i < m_cubePositions.size(); i++)
    {
        QMatrix4x4 model;
        model.translate(m_cubePositions[i]);
        model.rotate(20.0f * (float)i, QVector3D(1.0f, 0.3f, 0.5f));
        mp_shaderProgLight->setUniformValue(mp_shaderProgLight->uniformLocation("model"), model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    mp_shaderProgLight->release();

    mp_shaderProgLamp->bind();
    mp_shaderProgLamp->setUniformValue(mp_shaderProgLamp->uniformLocation("view"), m_viewMatrix);
    mp_shaderProgLamp->setUniformValue(mp_shaderProgLamp->uniformLocation("projection"), m_projectionMatrix);

    glBindVertexArray(m_lightVAO);
    for (unsigned int i = 0; i < m_pointLightPositions.size(); i++)
    {
        QMatrix4x4 model;
        model.translate(m_pointLightPositions[i]);
        model.scale(0.1f);
        mp_shaderProgLamp->setUniformValue(mp_shaderProgLamp->uniformLocation("model"), model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }


    mp_shaderProgLamp->release();
    mp_shaderProgLight->release();

    this->update();
}

void RenderWindow::processInput()
{

    float cameraSpeed = cm_cameraSpeedFactor * m_frameDelta;

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(m_lastMouseState.fov, (float)width()/(float)height(), 0.1f, 100.0f);

    m_camera.setProjectionMatrix(m_projectionMatrix);

    if (m_buttonsState.W_keyPressed == true)
        m_camera.translateWorld(cameraSpeed * m_camera.viewVector());

    if (m_buttonsState.S_keyPressed == true)
        m_camera.translateWorld(-cameraSpeed * m_camera.viewVector());

    if (m_buttonsState.A_keyPressed == true)
        m_camera.translateWorld(-QVector3D::normal(m_camera.viewVector(), m_camera.upVector()) * cameraSpeed);

    if (m_buttonsState.D_keyPressed == true)
        m_camera.translateWorld(QVector3D::normal(m_camera.viewVector(), m_camera.upVector()) * cameraSpeed);

    if (m_buttonsState.Q_keyPressed == true)
    {
        m_cameraDirection.roll -= 5.0f * cameraSpeed;
        if (m_cameraDirection.roll >= -89.0f)
            m_camera.roll(-5.0f * cameraSpeed);
        else
            m_cameraDirection.roll = -89.0f;
    }

    if (m_buttonsState.E_keyPressed == true)
    {
        m_cameraDirection.roll += 5.0f * cameraSpeed;
        if (m_cameraDirection.roll <= 89.0f)
            m_camera.roll(5.0f * cameraSpeed);
        else
            m_cameraDirection.roll =  89.0f;
    }
}

void RenderWindow::defineFrameDelta()
{
    float currentFrameTime = m_frameTimer.elapsed();
    m_frameDelta = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;

}
