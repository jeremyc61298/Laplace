#include <string>

namespace my {
	struct heatmap {
		const double* sheet;
		int num_cols;
		int top_row;
		int left_col;
		int bottom_row;
		int right_col;
		int scale;
		std::string file_name;

		void print_to_file();
	};
}