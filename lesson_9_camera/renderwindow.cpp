#include "renderwindow.h"

#include <QApplication>
#include <QtDebug>
#include <QFile>

#include <cassert>
#include <math.h>
#include "stb_image.h"

//#define USE_EULER_ANGLES
#define USE_QUATERNIONS


RenderWindow::RenderWindow(/*QOpenGLContext *shareContext*/)
    : QOpenGLWindow(/*shareContext, QOpenGLWindow::NoPartialUpdate*/),
      mp_vertexShader(nullptr),
      mp_fragmentShader(nullptr),
      mp_shaderProg(nullptr)
{
    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setCursor(QCursor(Qt::BlankCursor));
}

RenderWindow::~RenderWindow()
{
    makeCurrent();

    delete mp_shaderProg;
    delete mp_vertexShader;
    delete mp_fragmentShader;

    doneCurrent();

    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void RenderWindow::loadShaders(const QString& vertexShaderFileName,const QString& fragmentShaderFileName)
{
    mp_vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    mp_fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    mp_shaderProg = new QOpenGLShaderProgram;

    QFile vertex(vertexShaderFileName);
    assert(vertex.open(QIODevice::ReadOnly) && "Vertex shader file opening failed!");
    QByteArray vertexSource = vertex.readAll();

    QFile fragment(fragmentShaderFileName);
    assert(fragment.open(QIODevice::ReadOnly) && "Fragment shader file opening failed!");
    QByteArray fragmentSource = fragment.readAll();

    vertex.close();
    fragment.close();

    if (!mp_vertexShader->compileSourceCode(vertexSource))
        {
            qDebug() << "Vertex shader compilation failed!\n" << mp_vertexShader->log();
        }
    if (!mp_fragmentShader->compileSourceCode(fragmentSource))
        {
            qDebug() << "Fragment shader compilation failed!\n" << mp_fragmentShader->log();
        }
    mp_shaderProg->addShader(mp_vertexShader);
    mp_shaderProg->addShader(mp_fragmentShader);

    assert(mp_shaderProg->link() && "ShaderProgram: Linking Failed!");



}

void RenderWindow::loadTextures(const QString &texture_1FileName, const QString &texture_2FileName)
{
    glGenTextures(1, &m_texture1);
    glGenTextures(1, &m_texture2);

    glBindTexture(GL_TEXTURE_2D, m_texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, m_texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, m_texture1);
    int width, height, nrChannels;
    unsigned char *p_data = stbi_load(texture_1FileName.toStdString().data(), &width, &height, &nrChannels, 0);

    if (p_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, p_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        qDebug() << "Failed to load texture1!";
    }
    stbi_image_free(p_data);
    p_data = nullptr;

    glBindTexture(GL_TEXTURE_2D, m_texture2);
    stbi_set_flip_vertically_on_load(true);
    p_data = stbi_load(texture_2FileName.toStdString().data(), &width, &height, &nrChannels, 0);

    if (p_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, p_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        qDebug() << "Failed to load texture2!";
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
    loadShaders(QString(QApplication::applicationDirPath() + QString("\\shaders\\vertex_shader.sh")),
                QString(QApplication::applicationDirPath() + QString("\\shaders\\fragment_shader.sh")));
    loadTextures(QString(QApplication::applicationDirPath() + QString("\\textures\\container.jpg")),
                 QString(QApplication::applicationDirPath() + QString("\\textures\\awesomeface.png")));
#endif

#ifdef Q_OS_UNIX
    loadShaders(QString(QApplication::applicationDirPath() + QString("/shaders/vertex_shader.sh")),
                QString(QApplication::applicationDirPath() + QString("/shaders/fragment_shader.sh")));
    loadTextures(QString(QApplication::applicationDirPath() + QString("/textures/container.jpg")),
                 QString(QApplication::applicationDirPath() + QString("/textures/awesomeface.png")));
#endif

    mp_shaderProg->bind();

    mp_shaderProg->setUniformValue(mp_shaderProg->uniformLocation("texture1"), 0);
    mp_shaderProg->setUniformValue(mp_shaderProg->uniformLocation("texture2"), 1);

    mp_shaderProg->release();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);

#ifdef USE_EULER_ANGLES
    m_cameraPosition = {0.0f, 0.0f, 3.0f};
    m_cameraFront = {0.0f, 0.0f, -1.0f};
    m_cameraUp = {0.0f, 1.0f, 0.0f};
#endif

#ifdef USE_QUATERNIONS
    m_camera.setProjectionType(QCameraLens::PerspectiveProjection);
    m_camera.setPosition(QVector3D(0.0f, 0.0f, 3.0f));
    m_camera.setViewCenter(QVector3D(0.0f, 0.0f, -1.0f));
    m_camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
#endif

    m_cubePositions = {
      QVector3D( 0.0f,  0.0f,  0.0f),
      QVector3D( 2.0f,  5.0f, -15.0f),
      QVector3D(-1.5f, -2.2f, -2.5f),
      QVector3D(-3.8f, -2.0f, -12.3f),
      QVector3D( 2.4f, -0.4f, -3.5f),
      QVector3D(-1.7f,  3.0f, -7.5f),
      QVector3D( 1.3f, -2.0f, -2.5f),
      QVector3D( 1.5f,  2.0f, -2.5f),
      QVector3D( 1.5f,  0.2f, -1.5f),
      QVector3D(-1.3f,  1.0f, -1.5f)
    };


    float vertices[] = {
        //vertex coords         //texture coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };


    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);


    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        //Cordinate attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

        //Texture attributes
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


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
#ifdef USE_QUATERNIONS
    m_camera.setProjectionMatrix(m_projectionMatrix);
#endif

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

#ifdef USE_QUATERNIONS
//Yaw rotation
    m_camera.pan(xOffset);
//Pitch rotation
    m_camera.tilt(yOffset);
#endif

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

    static float rotation = 0;
    rotation += 3.6f * static_cast<float>(m_frameDelta)/1000.0f;
    if (rotation >= 360.0f)
        rotation = 0.0f;

#ifdef USE_EULER_ANGLES
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition,
                        m_cameraPosition + m_cameraFront,
                        m_cameraUp);
#endif

#ifdef USE_QUATERNIONS
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_camera.position(),
                        m_camera.position() + m_camera.viewVector(),
                        m_camera.upVector());
#endif

    mp_shaderProg->bind();

    mp_shaderProg->setUniformValue(mp_shaderProg->uniformLocation("view"), m_viewMatrix);
    mp_shaderProg->setUniformValue(mp_shaderProg->uniformLocation("projection"), m_projectionMatrix);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2);
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0; i < m_cubePositions.size(); i++)
    {
        m_modelMatrix.setToIdentity();
        m_modelMatrix.translate(m_cubePositions[i]);

        m_modelMatrix.rotate(rotation * (i+1) * 10, 1.0f, 0.3f, 0.5f);
        mp_shaderProg->setUniformValue(mp_shaderProg->uniformLocation("model"), m_modelMatrix);

        glDrawArrays(GL_TRIANGLES, 0, 36);

    }

    mp_shaderProg->release();

    this->update();
}

void RenderWindow::processInput()
{

    float cameraSpeed = cm_cameraSpeedFactor * m_frameDelta;

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(m_lastMouseState.fov, (float)width()/(float)height(), 0.1f, 100.0f);

#ifdef USE_EULER_ANGLES
    if (m_buttonsState.W_keyPressed == true)
        m_cameraPosition += cameraSpeed * m_cameraFront;

    if (m_buttonsState.S_keyPressed == true)
        m_cameraPosition -= cameraSpeed * m_cameraFront;

    if (m_buttonsState.A_keyPressed == true)
        m_cameraPosition -= QVector3D::normal(m_cameraFront, m_cameraUp) * cameraSpeed;

    if (m_buttonsState.D_keyPressed == true)
        m_cameraPosition += QVector3D::normal(m_cameraFront, m_cameraUp) * cameraSpeed;

    if (m_buttonsState.Q_keyPressed == true)
        m_cameraDirection.roll -= 5.0f * cameraSpeed;

    if (m_buttonsState.E_keyPressed == true)
        m_cameraDirection.roll += 5.0f * cameraSpeed;

    if (m_cameraDirection.roll > 89.0f)
        m_cameraDirection.roll =  89.0f;

    if (m_cameraDirection.roll < -89.0f)
        m_cameraDirection.roll = -89.0f;

    float toRadiansFactor = M_PI/180.0f;


    QVector3D direction;
    direction.setX(cos(m_cameraDirection.yaw * toRadiansFactor) *
                   cos(m_cameraDirection.pitch * toRadiansFactor));
    direction.setY(sin(m_cameraDirection.pitch * toRadiansFactor));
    direction.setZ(sin(m_cameraDirection.yaw * toRadiansFactor) *
                   cos(m_cameraDirection.pitch * toRadiansFactor));
    m_cameraFront = direction.normalized();

    m_cameraUp = QVector3D(sin(m_cameraDirection.roll * toRadiansFactor),
                           cos(m_cameraDirection.roll * toRadiansFactor),
                           0.0f).normalized();
#endif

#ifdef USE_QUATERNIONS

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
#endif

}

void RenderWindow::defineFrameDelta()
{
    float currentFrameTime = m_frameTimer.elapsed();
    m_frameDelta = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;

}
