#include <png++/png.hpp>
#include "heatmap.hpp"

namespace my {

	png::rgb_pixel map_color(double t) {
		const double m1 = 1.0 / 6.0;
		const double m2 = 2.0 / 6.0;
		const double m3 = 3.0 / 6.0;
		const double m4 = 4.0 / 6.0;
		const double m5 = 5.0 / 6.0;
		png::byte red = 0;
		png::byte green = 0;
		png::byte blue = 0;

		if (t < m2) {

		}
		else if (t < m4) {
			red = 255.0 * (t - m2) / m2;
		}
		else if (t < m5) {
			red = 255;
		}
		else if (t < 1) {
			red = 255.0 * ((1 - t) / m1 / 2 + 0.5);
		}
		else if (t >= 1) {
			red = 255.0 / 2;
		}

		if (t < m1) {

		}
		else if (t < m2) {
			green = 255.0 * (t - m1) / m1;
		}
		else if (t < m4) {
			green = 255;
		}
		else if (t < m5) {
			green = 255.0 * (m5 - t) / m1;
		}

		if (t < 0) {
			blue = 255.0 / 2;
		}
		else if (t < m1) {
			blue = 255.0 * (t / m1 / 2 + 0.5);
		}
		else if (t < m2) {
			blue = 255;
		}
		else if (t < m4) {
			blue = 255.0 * (m4 - t) / m2;
		}

		return png::rgb_pixel(red, green, blue);
	}

	void heatmap::print_to_file() {
		const int width = right_col - left_col;
		const int height = bottom_row - top_row;
		png::image<png::rgb_pixel> image(width * scale, height * scale);
		for (png::uint_32 r = 0; r < width; ++r) {
			for (png::uint_32 c = 0; c < height; ++c) {
				double t = sheet[(r + top_row) * num_cols + c + left_col];
				png::rgb_pixel color = map_color(t);
				for (int sr = 0; sr < scale; ++sr) {
					for (int sc = 0; sc < scale; ++sc) {
						image[scale * r + sr][scale * c + sc] = color;
					}
				}
			}
		}
		image.write(file_name);
	}

}