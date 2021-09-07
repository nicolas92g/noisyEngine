#include "Camera.h"
#include <fstream>
#include <Utils/yamlConverter.h>

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
		glm::vec<2, double> cursorPos = win.getCursorPos();

		this->setYaw((float)yaw_ - float(mouseSensivity * float(win.width() / 2 - cursorPos.x)));
		this->setPitch((float)pitch_ + float(mouseSensivity * float(win.height() / 2 - cursorPos.y)));

		win.setCursorPos((double)win.width() * .5, (double)win.height() * .5);
		win.hideCursor();
		
		updateLookWithYawAndPitch();
	}
	else {
		win.showCursor();
	}
}
void ns::Camera::classicKeyboardControls(Window& win, float speed)
{
	glm::vec3 lookWithoutY = direction_;
	lookWithoutY.y = 0;

	if (win.key(GLFW_KEY_W)) {
		position_ += (float)win.deltaTime() * glm::normalize(lookWithoutY) * speed;
	}
	if (win.key(GLFW_KEY_S)) {
		position_ -= (float)win.deltaTime() * glm::normalize(lookWithoutY) * speed;
	}
	const glm::vec3 up = glm::vec3(0, 1, 0);
	//lateral vector
	glm::vec3 right = glm::normalize(glm::cross(up, direction_));

	if (win.key(GLFW_KEY_A)) {
		position_ += (float)win.deltaTime() * glm::normalize(right) * speed;
	}
	if (win.key(GLFW_KEY_D)) {
		position_ -= (float)win.deltaTime() * glm::normalize(right) * speed;
	}
	if (win.key(GLFW_KEY_SPACE)) {
		position_.y += (float)win.deltaTime() * speed;
	}
	if (win.key(GLFW_KEY_LEFT_CONTROL)) {
		position_.y -= (float)win.deltaTime() * speed;
	}
}

bool ns::Camera::isVertexInTheFieldOfView(const glm::vec3& vertex, float offset)
{
	vec4 co = projection_ * view_ * vec4(vertex, 1);
	vec2 screen = vec2(co.x / co.z, co.y / co.z);
	if (co.z < -0.1f)
		return false;
 	return ((screen.x > (-1 - offset) and screen.x < (1 + offset)) and (screen.y > (-1 - offset) and screen.y < (1 + offset)));
}

glm::vec3 ns::Camera::rightDirection() const
{
	return glm::vec3(glm::normalize(-glm::cross(vec3(0.0f,1.0f,0.0f), direction_)));
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

void ns::Camera::setUpDirection(const glm::vec3& up_)
{
	upDirection_ = glm::normalize(up_);
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
