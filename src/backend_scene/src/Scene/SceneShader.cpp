#include "SceneShader.h"

using namespace wallpaper;

void ShaderValue::fromSpan(std::span<const value_type> s) noexcept {
        m_size    = (size_t)s.size();
        m_dynamic = s.size() > m_value.size();
        if (m_dynamic) {
            m_dvalue.resize(m_size);
            std::copy(s.begin(), s.end(), m_dvalue.begin());
        } else
            std::copy(s.begin(), s.end(), m_value.begin());
    }
