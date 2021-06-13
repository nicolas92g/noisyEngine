#include "Camera.h"
using namespace glm;

ns::Camera::Camera(const vec3& pos, float pitch_, float yaw_, float fov_)
{
	position = pos;
	pitch = pitch_;
	yaw = yaw_;
	fov = fov_;
	zNear = 0.1f;
	zFar = 100.0f;
	upDirection =  vec3(0,1,0);

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
	view = lookAt(position, position + look, upDirection);
	projection = perspective(fov, width / height,zNear, zFar);
}

void ns::Camera::updateLookWithYawAndPitch()
{
	look.x = cos(yaw) * cos(pitch);
	look.y = sin(pitch);
	look.z = sin(yaw) * cos(pitch);
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
	
	if (!disableMouse) {
		glm::vec<2, double> cursorPos = win.getCursorPos();

		this->setYaw((float)yaw - float(mouseSensivity * float(win.width() / 2 - cursorPos.x)));
		this->setPitch((float)pitch + float(mouseSensivity * float(win.height() / 2 - cursorPos.y)));

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
	glm::vec3 lookWithoutY = look;
	lookWithoutY.y = 0;

	if (win.key(GLFW_KEY_W)) {
		position += (float)win.deltaTime() * glm::normalize(lookWithoutY) * speed;
	}
	if (win.key(GLFW_KEY_S)) {
		position -= (float)win.deltaTime() * glm::normalize(lookWithoutY) * speed;
	}
	const glm::vec3 up = glm::vec3(0, 1, 0);
	//lateral vector
	glm::vec3 right = glm::normalize(glm::cross(up, look));

	if (win.key(GLFW_KEY_A)) {
		position += (float)win.deltaTime() * glm::normalize(right) * speed;
	}
	if (win.key(GLFW_KEY_D)) {
		position -= (float)win.deltaTime() * glm::normalize(right) * speed;
	}
	if (win.key(GLFW_KEY_SPACE)) {
		position.y += (float)win.deltaTime() * speed;
	}
	if (win.key(GLFW_KEY_LEFT_CONTROL)) {
		position.y -= (float)win.deltaTime() * speed;
	}
}

bool ns::Camera::isVertexInTheFieldOfView(const glm::vec3& vertex, float offset)
{
	vec4 co = projection * view * vec4(vertex, 1);
	vec2 screen = vec2(co.x / co.z, co.y / co.z);
	if (co.z < -0.1f)
		return false;
 	return ((screen.x > (-1 - offset) and screen.x < (1 + offset)) and (screen.y > (-1 - offset) and screen.y < (1 + offset)));	
}

glm::vec3 ns::Camera::getRightDirection() const
{
	return glm::vec3(glm::normalize(-glm::cross(vec3(0.0f,1.0f,0.0f), look)));
}

glm::vec3 ns::Camera::getPosition() const
{
	return position;
}

glm::vec3 ns::Camera::getLook() const
{
	return glm::normalize(look);
}

glm::vec3 ns::Camera::getUpDirection() const
{
	return upDirection;
}

float ns::Camera::getPitch() const
{
	return pitch;
}

float ns::Camera::getYaw() const
{
	return yaw;
}

glm::mat4 ns::Camera::getProjection() const
{
	return projection;
}

glm::mat4 ns::Camera::getView() const
{
	return view;
}

glm::mat4 ns::Camera::getProjectionView() const
{
	return projection * view;
}

float ns::Camera::getFov() const
{
	return fov;
}

float ns::Camera::getZNear() const
{
	return zNear;
}

float ns::Camera::getZFar() const
{
	return zFar;
}

void ns::Camera::setPosition(const glm::vec3& position_)
{
	position = position_;
}

void ns::Camera::setLook(const glm::vec3& look_)
{
	look = look_;
}

void ns::Camera::setUpDirection(const glm::vec3& up_)
{
	upDirection = glm::normalize(up_);
}

void ns::Camera::setPitch(float pitch_)
{
	if (pitch_ > PI/2.0f) {
		pitch = PI/ 2.0f;
	}
	else if (pitch_ < -PI / 2.0f) {
		pitch = -PI / 2.0f;
	}
	else {
		pitch = pitch_;
	}
}

void ns::Camera::setYaw(float yaw_)
{
	yaw = yaw_;
}

void ns::Camera::setProjection(const glm::mat4& projection_)
{
	projection = projection_;
}

void ns::Camera::setView(const glm::mat4& view_)
{
	view = view_;
}

void ns::Camera::setFov(float fov_)
{
	if (fov_ < 0.0f) {
		fov = 0;
	}
	else if (fov_ > 180.0f) {
		fov = 180.0f;
	}
	else {
		fov = fov_;
	}
}

void ns::Camera::setZNear(float zNear_)
{
	if (zNear_ >= 0.0f) {
		zNear = zNear_;
	}
}

void ns::Camera::setZFar(float zFar_)
{
	if (zFar_ >= 0.0f) {
		zFar = zFar_;
	}
}
