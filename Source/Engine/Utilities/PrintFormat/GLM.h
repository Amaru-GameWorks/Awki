#pragma once

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

#include <format>

template <glm::length_t L, typename T, glm::qualifier Q>
struct std::formatter<glm::vec<L, T, Q>> : std::formatter<std::string>
{
	auto format(const glm::vec<L, T, Q>& value, std::format_context& ctx) const
	{
		std::formatter<T> underlying;

		auto out = ctx.out();
		out = std::format_to(out, "[");

		for (glm::length_t i = 0; i < L; ++i) 
		{
			out = underlying.format(value[i], ctx);

			if (i < L - 1)
				out = std::format_to(out, " ");
		}

		return std::format_to(out, "]");
	}
};

template <typename T, glm::qualifier Q>
struct std::formatter<glm::qua<T, Q>> : std::formatter<std::string>
{
	auto format(const glm::qua<T, Q>& value, std::format_context& ctx) const
	{
		std::formatter<T> underlying;

		auto out = ctx.out();
		out = std::format_to(out, "[");

		for (glm::length_t i = 0; i < 4; ++i) 
		{
			out = underlying.format(value[i], ctx);

			if (i < 3)
				out = std::format_to(out, " ");
		}

		return std::format_to(out, "]");
	}
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct std::formatter<glm::mat<C, R, T, Q>> : std::formatter<std::string>
{
	std::formatter<T> underlying;

	constexpr auto parse(format_parse_context& ctx)
	{
		return underlying.parse(ctx);
	}

	auto format(const glm::mat<C, R, T, Q>& value, std::format_context& ctx) const
	{
		auto out = ctx.out();

		for (glm::length_t i = 0; i < C; ++i)
		{
			out = std::format_to(out, "[");
			for (glm::length_t j = 0; j < R; ++j)
			{
				out = underlying.format(value[i][j], ctx);

				if (j < R - 1)
					out = std::format_to(out, " ");
			}
			out = std::format_to(out, "]");
		}

		return out;
	}
};