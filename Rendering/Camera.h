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


namespace ns {
    template<typename P = DEFAULT_PTYPE, typename D = DEFAULT_DTYPE>
    /**
     * @class Camera
     * @brief create some 4x4 matrices to simulate a camera in a 3D world\n
     * this allow to display your 3D scene on a 2d screen
     */
    class Camera : public DirectionalObject3d<P, D> 
    {
    public:
        using vec3p = glm::vec<3, P>;
        using mat4p = glm::mat<4, 4, P>;
        using vec3d = glm::vec<3, D>;
        /**
         * @brief create the camera with some default values
         * \param pos
         * \param pitch
         * \param yaw
         * \param fov
         */
        Camera(const vec3p& pos = glm::vec3(0), const vec3d& direction = vec3d(1, 0, 0), float fov = glm::pi<float>() * .4f);
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
        void calculateMatrix(P width, P height);                             
        /**
         * @brief allow to modif pitch and yaw (the view direction) with the mouse
         * \param win
         * \param mouseSensivity
         */
        void classicMouseControls(Window& win, double mouseSensivity, float limitOffset = .1f);
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
        bool isVertexInTheFieldOfView(const vec3p& vertexLocation, P offset = 0.0f);
       
        //MODIFIERS
        /**
         * @brief set the up direction of the camera vec3(0, 1, 0) by default vec3(0, -1, 0) will look upside down
         * \param up_
         */
        void setUpDirection(const vec3d& up);
        /**
         * @brief set your own projection matrix
         * \param projection_
         */
        void setProjection(const mat4p& projection);
        /**
         * @brief set your own view matrix
         * \param view_
         */
        void setView(const mat4p& view);
        /**
         * @brief angle of view in degrees
         * \param fov_
         */
        void setFov(P fov);
        /**
         * @brief all the fragments which have a distance less than ZNear are not displayed
         * so this method change  that ZNear value (by default equals to 0.1f)
         * \param zNear_
         */
        void setZNear(P zNear);
        /**
         * @brief this is the maximun distance where fragment can be seen by this camera
         * don't put a too big value because this will provoke some performances issues
         * \param zFar_
         */
        void setZFar(P zFar);

        //accessors :
        /**
         * @brief return the right direction relatively to the camera
         * \return 
         */
        vec3d rightDirection() const;
        /**
         * @brief return the up direction
         * \return 
         */
        const vec3d& upDirection() const;
        /**
         * @brief return the projection matrix
         * \return 
         */
        const mat4p& projection() const;
        /**
         * @brief return the view matrix
         * \return 
         */
        const mat4p& view() const;
        /**
         * @brief return projection * view
         * \return 
         */
        mat4p projectionView() const;
        /**
         * @brief return the field of view of the camera
         * \return 
         */
        P fov() const;
        /**
         * @brief return the minimun Z of the camera
         * \return 
         */
        P zNear() const;
        /**
         * @brief return the maximun Z of the camera
         * \return 
         */
        P zFar() const;

    protected:
        vec3d upDirection_;

        mat4p projection_;
        mat4p view_;

        P fov_;
        P zNear_;
        P zFar_;

        friend class Debug;
    };
}
