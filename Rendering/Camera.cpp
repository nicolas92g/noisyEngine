#include "Camera.h"
#include <fstream>
#include <Utils/yamlConverter.h>
#include <Utils/DebugLayer.h>

using namespace glm;

ns::Camera::Camera(const vec3& pos, float pitch, float yaw, float fov)
	:
	DirectionalObject3d(pos, glm::vec3(1, 0, 0))
{
	pitch_ = pitch;
	yaw_ = yaw;
	fov_ = fov;
	zNear_ = 0.1f;
	zFar_ = 400.0f;
	upDirection_ =  vec3(0,1,0);

	//just to initalize vars
	updateLookWithYawAndPitch();
	calculateMatrix(10,10);
}

void ns::Camera::calculateMatrix(Window& win)
{
	if (win.width() and win.height())
		calculateMatrix((float)win.width(),(float)win.height());
}

void ns::Camera::calculateMatrix(float width, float height)
{
	//transform pitch and yaw into a look vector
#	ifndef NDEBUG
	assert(this != nullptr);// if this line create a debug breakpoint is because you try to modif a camera that does'nt exist
#	endif //NDEBUG

	//put all vars into 4x4 matrix transformation
	view_ = lookAt(position_, position_ + direction_, upDirection_);
	projection_ = perspective(fov_, width / height,zNear_, zFar_);
}

void ns::Camera::updateLookWithYawAndPitch()
{
	direction_.x = cos(yaw_) * cos(pitch_);
	direction_.y = sin(pitch_);
	direction_.z = sin(yaw_) * cos(pitch_);
}

void ns::Camera::classicMouseControls(Window& win, double mouseSensivity)
{
	//disable mouse
	static bool disableMouseKey, disableMouse;
	if (win.key(GLFW_KEY_TAB)) {
		if (!disableMouseKey) {
			disableMouse = !disableMouse;
			win.setCursorPos(.0, .0);
		}
	}
	disableMouseKey = win.key(GLFW_KEY_TAB);
	//mouse control
	
	if (!disableMouse and win.isFocused()) {
		const glm::dvec2 middle = win.size() / 2;
		glm::dvec2 offset = (middle - win.getCursorPos());
		offset.y *= -1;

		//setYaw((float)yaw_ - float(mouseSensivity * offset.x));
		//setPitch((float)pitch_ + float(mouseSensivity * offset.y));

		win.setCursorPos(middle.x, middle.y);
		win.hideCursor();
				
		float yAngle = offset.y * mouseSensivity;

		if (yAngle > 0) {
			float maxAngle = PI - acos(std::max(glm::dot(direction_, upDirection_), -1.f));
		
			if (yAngle > maxAngle) {
				yAngle = maxAngle;
			}
		}
		else if(yAngle < 0){
			float maxAngle = PI - acos(std::max(glm::dot(direction_, -upDirection_), -1.f));

			if (-yAngle > maxAngle) {
				yAngle = -maxAngle;
			}
		}

		direction_ = glm::rotate<float>(offset.x * mouseSensivity, upDirection_) * glm::vec4(direction_, 1.f);
		direction_ = glm::rotate<float>(yAngle, rightDirection()) * glm::vec4(direction_, 1.f);
		
		//updateLookWithYawAndPitch();
	}
	else {
		win.showCursor();
	}
}
void ns::Camera::classicKeyboardControls(Window& win, float speed)
{
	const glm::vec3 right = rightDirection();
	const glm::vec3 forward = glm::normalize(glm::cross(right, upDirection_));
	
	if (win.key(GLFW_KEY_LEFT_SHIFT)) {
		speed *= 2;
	}
	if (win.key(GLFW_KEY_W)) {
		position_ += (float)win.deltaTime() * forward * speed;
	}
	if (win.key(GLFW_KEY_S)) {
		position_ -= (float)win.deltaTime() * forward * speed;
	}
	if (win.key(GLFW_KEY_A)) {
		position_ += (float)win.deltaTime() * right * speed; 
	}
	if (win.key(GLFW_KEY_D)) {
		position_ -= (float)win.deltaTime() * right * speed;
	}
	if (win.key(GLFW_KEY_SPACE)) {
		position_ += (float)win.deltaTime() * upDirection_ * speed;
	}
	if (win.key(GLFW_KEY_LEFT_CONTROL)) {
		position_ -= (float)win.deltaTime() * upDirection_ * speed;
	}

}

bool ns::Camera::isVertexInTheFieldOfView(const glm::vec3& vertex, float offset)
{
	vec4 co = projection_ * view_ * vec4(vertex, 1);
	if (co.z < -0.1f) return false;
	vec2 screen = vec2(co.x / co.z, co.y / co.z);
 	return ((screen.x > (-1 - offset) and screen.x < (1 + offset)) and (screen.y > (-1 - offset) and screen.y < (1 + offset)));
}

glm::vec3 ns::Camera::rightDirection() const
{
	return glm::normalize(glm::cross(upDirection_, direction_));
}

glm::vec3 ns::Camera::upDirection() const
{
	return upDirection_;
}

float ns::Camera::pitch() const
{
	return pitch_;
}

float ns::Camera::yaw() const
{
	return yaw_;
}

glm::mat4 ns::Camera::projection() const
{
	return projection_;
}

glm::mat4 ns::Camera::view() const
{
	return view_;
}

glm::mat4 ns::Camera::projectionView() const
{
	return projection_ * view_;
}

float ns::Camera::fov() const
{
	return fov_;
}

float ns::Camera::zNear() const
{
	return zNear_;
}

float ns::Camera::zFar() const
{
	return zFar_;
}

void ns::Camera::setUpDirection(const glm::vec3& up)
{
	const glm::vec3 n = glm::normalize(up);
	if (n != upDirection_) {
		if( n == -upDirection_) upDirection_ = n;

		const vec3 axis = cross(n, upDirection_);
		const float angle = acos(std::min(std::max(dot(n, upDirection_), -1.f), 1.f));

		glm::vec3 dir = (vec4(direction_, 1.f) * rotate(angle, axis));
		
		if (!isnan(dir.x)) direction_ = dir;

		upDirection_ = n;
	}
}

void ns::Camera::setPitch(float pitch)
{
	if (pitch > PI/2.0f) {
		pitch_ = PI/ 2.0f;
	}
	else if (pitch < -PI / 2.0f) {
		pitch_ = -PI / 2.0f;
	}
	else {
		pitch_ = pitch;
	}
}

void ns::Camera::setYaw(float yaw)
{
	yaw_ = yaw;
}

void ns::Camera::setProjection(const glm::mat4& projection)
{
	projection_ = projection;
}

void ns::Camera::setView(const glm::mat4& view)
{
	view_ = view;
}

void ns::Camera::setFov(float fov)
{
	if (fov < 0.0f) {
		fov_ = 0;
	}
	else if (fov > 180.0f) {
		fov_ = 180.0f;
	}
	else {
		fov_ = fov;
	}
}

void ns::Camera::setZNear(float zNear)
{
	if (zNear > 0.0f) {
		zNear_ = zNear;
	}
}

void ns::Camera::setZFar(float zFar)
{
	if (zFar > 0.0f) {
		zFar_ = zFar;
	}
}
