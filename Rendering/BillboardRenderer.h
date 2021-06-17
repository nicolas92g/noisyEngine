#pragma once
#include "Camera.h"
#include "Billboard.h"

namespace ns {
	class BillboardRenderer
	{
	public:
		BillboardRenderer(Camera& cam);
		~BillboardRenderer();

		void update();
		void draw();

		void addBillboard(Billboard& billboard);
		void removeBillboard(Billboard& billboard);

		void clearBillboards();

	protected:
		Shader shader_;

		Camera& cam_;
		std::vector<Billboard*> billboards_;
	};
}
