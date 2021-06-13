#include <iostream>

#include "Window.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "Renderer3d.h"
#include "Scene.h"

#include "Timer.h"

using namespace ns;
using namespace glm;

int main() {
	Material::describeMaterialsWhenCreate = false;
	Window win(800, 600);

	performanceBench bench;
	Timer loading("loading");
	
	Model model("resources/rockyfloor.fbx");
	Object3d floor(model);

	Model model2("C:/Users/nicol/OneDrive/Documents/Graphismes/models/starship/starship.fbx");
	
	Material* fuselage = model2.getMaterial("fuselage");
	Material* ailette = model2.getMaterial("ailette");
	Material* raptor = model2.getMaterial("raptorVacuum");
	if (fuselage) {
		fuselage->setRoughnessConstant(.15f);
		fuselage->setMetallicConstant(.9f);
	}
	if (ailette) {
		ailette->setRoughnessConstant(.15f);
		ailette->setMetallicConstant(.9f);
	}
	if (raptor) {
		raptor->setRoughnessConstant(.15f);
		raptor->setMetallicConstant(.9f);
	}
	Object3d starship(model2, vec3(0, 2, 8));



	Model model3("C:/Users/nicol/OneDrive/Documents/Graphismes/models/tree/mylowPolyTree.fbx");
	Object3d tree(model3, vec3(4, 0, -8));
	
	
	Camera cam(vec3(0, 3, 0));
	Scene scene({&floor, &starship, &tree});
	Renderer3d render(win, cam, scene);

	loading.stop();
	while (win.shouldNotClose())
	{
		Shader::update(win);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, win.width(), win.height());

		cam.classicKeyboardControls(win, 5);
		cam.classicMouseControls(win, .004);

		render.draw();

		glfwPollEvents();
		win.swapBuffers();
		win.recordFrameTiming();

		win.setTitle((std::to_string(win.framerate()) + " fps").c_str());
		bench.recordFrame();
	}

	Material::clearTextures();

	return EXIT_SUCCESS;
}
