#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <Eigen/Dense>

#include "Utils/span.hpp"
#include "Utils/MapSet.hpp"

namespace wallpaper
{

using ShaderValueInter = std::array<float, 16>;

class ShaderValue {
public:
	using value_type = float;
public:
	ShaderValue() = default;
	~ShaderValue() = default;

	ShaderValue(const ShaderValue&) = default;
	ShaderValue& operator=(const ShaderValue&) = default;

	constexpr ShaderValue(const value_type& value) noexcept : m_value{value}, m_size{1} {}
    template <typename Range>
    ShaderValue(const Range& range) noexcept { fromSpan(range); }
	ShaderValue(const value_type* ptr, std::size_t num) noexcept { fromSpan({ptr, num}); }

	void fromSpan(Span<float> s) noexcept {
		m_size = std::min(m_value.size(), (size_t)s.size());
		std::copy(s.begin(), s.begin()+m_size, m_value.begin());
	}
	static ShaderValue fromMatrix(const Eigen::Ref<const Eigen::MatrixXf>& mat) {
		return ShaderValue(Span{mat.data(), (size_t)mat.size()});
	}
	static ShaderValue fromMatrix(const Eigen::Ref<const Eigen::MatrixXd>& mat) {
		const Eigen::Ref<const Eigen::MatrixXf>& matf = mat.cast<float>();
		return fromMatrix(matf);
	};
	constexpr const auto& operator[](std::size_t index) const { return m_value[index]; }
	constexpr auto& operator[](std::size_t index) { return m_value[index]; }

	constexpr auto data() const noexcept { return m_value.data(); };
	constexpr auto data() noexcept { return m_value.data(); };
	constexpr size_t size() const noexcept { return m_size; };

	void setSize(size_t v) noexcept { m_size = std::min(v, m_value.size()); }
private:
	ShaderValueInter m_value;
	size_t m_size {0};
};

using ShaderValues = Map<std::string, ShaderValue>;

struct ShaderAttribute {
public:
	std::string name;
	uint32_t location;
};


struct SceneShader {
public:
	uint32_t id;
	std::string name;

	std::string vertexCode;
	std::string geometryCode;
	std::string fragmentCode;

	std::vector<ShaderAttribute> attrs;
	ShaderValues default_uniforms;
};
}
