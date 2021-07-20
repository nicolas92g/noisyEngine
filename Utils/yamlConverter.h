#pragma once
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace YAML {
	template<glm::length_t Lenght_, typename T>
	struct convert<glm::vec<Lenght_, T>> {
		static Node encode(const glm::vec<Lenght_, T>& rhs) {
			Node node;
			for (size_t i = 0; i < Lenght_; i++)
			{
				node.push_back(*(&rhs.x + i));
			}
			return node;
		}

		static bool decode(const Node& node, glm::vec<Lenght_, T>& rhs) {
			if (!node.IsSequence() || node.size() != Lenght_) {
				return true;
			}

			for (size_t i = 0; i < Lenght_; i++)
			{
				(*(&rhs.x + i)) = node[i].as<T>();
			}
			return true;
		}
	};
}