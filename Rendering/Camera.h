#pragma once

//glm
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

//ns
#include "Window.h"
#include "Shader.h"
#include "Object3d.h"

#define PI 3.14f


namespace ns {
    /**
     * @class Camera
     * @brief create some 4x4 matrices to simulate a camera in a 3D world\n
     * this allow to display your 3D scene on a 2d screen
     */
    class Camera : public DirectionalObject3d
    {
    public:
        /**
         * @brief create the camera with some default values
         * \param pos
         * \param pitch
         * \param yaw
         * \param fov
         */
        Camera(const glm::vec3& pos = glm::vec3(0), float pitch = PI * .5f, float yaw = 0, float fov = PI * .4f);
        /**
         * @brief this take the values of the camera and update the matrices with them
         * the change of the camera will not be applied until you call this method
         * \param windowObject (needed to know the size of the window)
         */
        void calculateMatrix(Window& windowObject);
        /**
         * @brief this take the values of the camera and update the matrices with them
         * the change of the camera will not be applied until you call this method
         * \param width of the screen in pixel
         * \param height of the screen in pixel
         */
        void calculateMatrix(float width, float height);
        /**
         * @brief just modify the look vector with the yaw and the pitch of the camera
         */
        void updateLookWithYawAndPitch();                                
        /**
         * @brief allow to modif pitch and yaw (the view direction) with the mouse
         * \param win
         * \param mouseSensivity
         */
        void classicMouseControls(Window& win, double mouseSensivity);
        /**
         * @brief allow to modif the camera with z,q,s,d, space and left keys
         * \param win
         * \param speed
         */
        void classicKeyboardControls(Window& win, float speed);
        /**
         * @brief return if a point is in the field of view of the camera
         * \param vertexPosition
         * \param offset
         * \return 
         */
        bool isVertexInTheFieldOfView(const glm::vec3& vertexLocation, float offset = 0.0f);
       
        //MODIFIERS
        /**
         * @brief set the up direction of the camera vec3(0, 1, 0) by default vec3(0, -1, 0) will look upside down
         * \param up_
         */
        void setUpDirection(const glm::vec3& up);
        /**
         * @brief this is the rotation around the x axis of the camera in radians
         * \param pitch_
         */
        void setPitch(float pitch);
        /**
         * @brief this is the rotation around the y axis of the camera in radians
         * \param yaw_
         */
        void setYaw(float yaw);
        /**
         * @brief set your own projection matrix
         * \param projection_
         */
        void setProjection(const glm::mat4& projection);
        /**
         * @brief set your own view matrix
         * \param view_
         */
        void setView(const glm::mat4& view);
        /**
         * @brief angle of view in degrees
         * \param fov_
         */
        void setFov(float fov);
        /**
         * @brief all the fragments which have a distance less than ZNear are not displayed
         * so this method change  that ZNear value (by default equals to 0.1f)
         * \param zNear_
         */
        void setZNear(float zNear);
        /**
         * @brief this is the maximun distance where fragment can be seen by this camera
         * don't put a too big value because this will provoke some performances issues
         * \param zFar_
         */
        void setZFar(float zFar);

        //accessors :
        /**
         * @brief return the right direction relatively to the camera
         * \return 
         */
        glm::vec3 rightDirection() const;
        /**
         * @brief return the up direction
         * \return 
         */
        glm::vec3 upDirection() const;
        /**
         * @brief return Pitch
         * \return 
         */
        float pitch() const;
        /**
         * @brief return yaw
         * \return 
         */
        float yaw() const;
        /**
         * @brief return the projection matrix
         * \return 
         */
        glm::mat4 projection() const;
        /**
         * @brief return the view matrix
         * \return 
         */
        glm::mat4 view() const;
        /**
         * @brief return projection * view
         * \return 
         */
        glm::mat4 projectionView() const;
        /**
         * @brief return the field of view of the camera
         * \return 
         */
        float fov() const;
        /**
         * @brief return the minimun Z of the camera
         * \return 
         */
        float zNear() const;
        /**
         * @brief return the maximun Z of the camera
         * \return 
         */
        float zFar() const;

    protected:
        glm::vec3 upDirection_;

        float pitch_;     //vertical look angle
        float yaw_;       //horizontal look angle

        glm::mat4 projection_;
        glm::mat4 view_;

        float fov_;
        float zNear_;
        float zFar_;
    };
}
