//renderer
#include <noisy.hpp>
#include <configNoisy.hpp>

namespace ns {

	struct GeneratorInterfaceSettings {
		float mouseSensivity = .004f;
		float cameraSpeed = 5.f;
	};

	class GeneratorInterface {
	public:
		GeneratorInterface();
		~GeneratorInterface();

		int run();

	protected:
		Window window_;
		Camera cam_;
		Scene scene_;
		Renderer3d renderer_;


		GeneratorInterfaceSettings settings_;

	protected:
		static size_t heightComputationCount;
		static std::vector<MapSizeType> heightComputationPos;

		void debugOptimisationHeightsComputations(); //count if the height function is called multiple times for the same point
	};
}