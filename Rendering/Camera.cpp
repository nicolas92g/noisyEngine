#include "Camera.h"
#include <fstream>
#include <Utils/yamlConverter.h>
#include <Utils/DebugLayer.h>

template<typename P, typename D>
ns::Camera<P, D>::Camera(const vec3p& pos, const vec3d& direction, float fov)
	:
	DirectionalObject3d<P, D>(pos, direction)
{
	fov_ = fov;
	zNear_ = 0.1;
	zFar_ = 400.0;
	upDirection_ =  vec3d(0,1,0);

	//just to initalize vars
	this->calculateMatrix(10,10);
}

template<typename P, typename D>
void ns::Camera<P, D>::calculateMatrix(Window& win)
{
	if (win.width() and win.height())
		this->calculateMatrix((P)win.width(),(P)win.height());
}

template<typename P, typename D>
void ns::Camera<P, D>::calculateMatrix(P width, P height)
{
	//transform pitch and yaw into a look vector
#	ifndef NDEBUG
	_STL_ASSERT(this != nullptr, "you tried to edit a camera that does'nt exist (maybe you forget to add a camera to the renderer ?)");
#	endif //NDEBUG

	//put all vars into 4x4 matrix transformation
	view_ = glm::lookAt<P>(this->position_, this->position_ + static_cast<glm::vec<3, P>>(this->direction_), static_cast<glm::vec<3, P>>(this->upDirection_));
	projection_ = glm::perspective<P>((P)this->fov_, (P)width / (P)height, (P)this->zNear_, (P)this->zFar_);
}

template<typename P, typename D>
void ns::Camera<P, D>::classicMouseControls(Window& win, double mouseSensivity, float limitOffset)
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

		win.setCursorPos(middle.x, middle.y);
		win.hideCursor();
				
		D yAngle = offset.y * mouseSensivity;

		if (yAngle > 0.0) {
			D maxAngle = glm::pi<D>() * (1.f - limitOffset) - acos(std::max<D>(glm::dot<3, D>(this->direction_, this->upDirection_), -1.0));
		
			if (yAngle > maxAngle) {
				yAngle = maxAngle;
			}
		}
		else if(yAngle < 0.0){
			D maxAngle = glm::pi<D>() * (1.f - limitOffset) - acos(std::max<D>(glm::dot<3, D>(this->direction_, -this->upDirection_), -1.0));
		
			if (-yAngle > maxAngle) {
				yAngle = -maxAngle;
			}
		}

		this->direction_ = glm::rotate<D>(offset.x * mouseSensivity, this->upDirection_) * glm::vec<4, D>(this->direction_, 1.0);
		this->direction_ = glm::rotate<D>(yAngle, rightDirection()) * glm::vec<4, D>(this->direction_, 1.0);
	}
	else {
		win.showCursor();
	}
}

template<typename P, typename D>
void ns::Camera<P, D>::classicKeyboardControls(Window& win, float speed)
{
	const vec3d right = rightDirection();
	const vec3d forward = glm::normalize(glm::cross(right, this->upDirection_));
	
	if (win.key(GLFW_KEY_LEFT_SHIFT)) {
		speed *= 2;
	}
	if (win.key(GLFW_KEY_W)) {
		this->position_ += (D)win.deltaTime() * forward * (D)speed;
	}
	if (win.key(GLFW_KEY_S)) {
		this->position_ -= (D)win.deltaTime() * forward * (D)speed;
	}
	if (win.key(GLFW_KEY_A)) {
		this->position_ += (D)win.deltaTime() * right * (D)speed;
	}
	if (win.key(GLFW_KEY_D)) {
		this->position_ -= (D)win.deltaTime() * right * (D)speed;
	}
	if (win.key(GLFW_KEY_SPACE)) {
		this->position_ += (D)win.deltaTime() * this->upDirection_ * (D)speed;
	}
	if (win.key(GLFW_KEY_LEFT_CONTROL)) {
		this->position_ -= (D)win.deltaTime() * this->upDirection_ * (D)speed;
	}

}

template<typename P, typename D>
bool ns::Camera<P, D>::isVertexInTheFieldOfView(const vec3p& vertex, P offset)
{
	glm::vec<4, P> co = this->projection_ * this->view_ * glm::vec<4, P>(vertex, 1);
	if (co.z < -0.1) return false;
	glm::vec<2, P> screen = glm::vec<2, P>(co.x / co.z, co.y / co.z);
 	return ((screen.x > (-1 - offset) and screen.x < (1 + offset)) and (screen.y > (-1 - offset) and screen.y < (1 + offset)));
}

template<typename P, typename D>
glm::vec<3, D> ns::Camera<P, D>::rightDirection() const
{
	return glm::normalize(glm::cross(this->upDirection_, this->direction_));
}

template<typename P, typename D>
glm::vec<3, D> const& ns::Camera<P, D>::upDirection() const
{
	return this->upDirection_;
}

template<typename P, typename D>
const glm::mat<4, 4, P>& ns::Camera<P, D>::projection() const
{
	return this->projection_;
}

template<typename P, typename D>
const glm::mat<4, 4, P>& ns::Camera<P, D>::view() const
{
	return this->view_;
}

template<typename P, typename D>
glm::mat<4, 4, P> ns::Camera<P, D>::projectionView() const
{
	return this->projection_ * this->view_;
}

template<typename P, typename D>
P ns::Camera<P, D>::fov() const
{
	return this->fov_;
}

template<typename P, typename D>
P ns::Camera<P, D>::zNear() const
{
	return this->zNear_;
}

template<typename P, typename D>
P ns::Camera<P, D>::zFar() const
{
	return this->zFar_;
}

template<typename P, typename D>
void ns::Camera<P, D>::setUpDirection(const vec3d& up)
{
	const vec3d n = glm::normalize(up);
	if (n != this->upDirection_) {
		if (n == -upDirection_) { 
			this->upDirection_ = n;
			//direction_ *= -1;//negate the camera direction when the up direction is negated
			return;
		};

		const vec3d axis = glm::cross<D>(n, this->upDirection_);
		const D angle = acos(glm::dot<3, D>(n, this->upDirection_));

		vec3d dir = (glm::vec<4, D>(this->direction_, 1.f) * glm::rotate<D>(angle, axis));
		
		if (!glm::isnan<D>(dir.x)) this->direction_ = dir;

		this->upDirection_ = n;
	}
}

template<typename P, typename D>
void ns::Camera<P, D>::setProjection(const mat4p& projection)
{
	this->projection_ = projection;
}

template<typename P, typename D>
void ns::Camera<P, D>::setView(const mat4p& view)
{
	this->view_ = view;
}

template<typename P, typename D>
void ns::Camera<P, D>::setFov(P fov)
{
	if (fov < 0.0) {
		this->fov_ = 0.0;
	}
	else if (fov > glm::pi<P>()) {
		this->fov_ = glm::pi<P>();
	}
	else {
		this->fov_ = fov;
	}
}

template<typename P, typename D>
void ns::Camera<P, D>::setZNear(P zNear)
{
	if (zNear > 0.0) {
		this->zNear_ = zNear;
	}
}

template<typename P, typename D>
void ns::Camera<P, D>::setZFar(P zFar)
{
	if (zFar > 0.0) {
		this->zFar_ = zFar;
	}
}

template<typename P, typename D>
void templateFixLinkFunction_Camera_(){
	_STL_REPORT_ERROR("called a template fix link function !");
	using namespace ns;
	Window* win = nullptr;
	Camera<P, D> cam;
	cam.classicKeyboardControls(*win, .0);
	cam.classicMouseControls(*win, .0);
	cam.setUpDirection(glm::vec3(0));
	cam.setZFar(0);
	cam.upDirection();
	cam.projectionView();
	cam.calculateMatrix(*win);
	cam.projection();
	cam.view();
}

void FixLinkFunction_Camera_(){
	templateFixLinkFunction_Camera_<float, float>();
	templateFixLinkFunction_Camera_<double, float>();
	templateFixLinkFunction_Camera_<float, double>();
	templateFixLinkFunction_Camera_<double, double>();
}
