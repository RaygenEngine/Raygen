#include "Image.h"

glm::vec4 Image::BilinearSample(float u, float v, glm::vec2 pixelOffset) const
{
	if (format == ImageFormat::Hdr) {
		LOG_ERROR("Image::BiliniearSample is not implemented for hdr images");
		return glm::vec4(0);
	}

	auto sampleImg = [&](int w, int h) {
		int sampleW = glm::clamp(w, 0, width - 1);
		int sampleH = glm::clamp(h, 0, height - 1);

		size_t index = (sampleH * width + sampleW) * 4;

		const byte* ptr = data.data() + index;
		glm::u8vec4 content = *reinterpret_cast<const glm::u8vec4*>(ptr);
		return content;
	};


	float sw = u * width + pixelOffset.x;
	float sh = v * height + pixelOffset.y;

	glm::u8vec4 TL = sampleImg(math::roundToInt(floor(sw)), math::roundToInt(floor(sh)));
	glm::u8vec4 TR = sampleImg(math::roundToInt(ceil(sw)), math::roundToInt(floor(sh)));
	glm::u8vec4 BL = sampleImg(math::roundToInt(floor(sw)), math::roundToInt(ceil(sh)));
	glm::u8vec4 BR = sampleImg(math::roundToInt(ceil(sw)), math::roundToInt(ceil(sh)));

	float rightPercent = glm::fract(sw);
	float bottomPercent = glm::fract(sh);

	glm::u8vec4 topInterpol = glm::mix(TL, TR, rightPercent);
	glm::u8vec4 botInterpol = glm::mix(BL, BR, rightPercent);

	glm::u8vec4 color = glm::mix(topInterpol, botInterpol, bottomPercent);

	return color;
}
