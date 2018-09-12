#pragma once
// matrix.hpp
// This code was given by Dr. Foust, Harding University
#include <vector>
#include <iterator>
#include <ostream>
#include <iomanip>

namespace my {

	template <typename T>
	class matrix {
		std::vector<T> _elements;
		size_t _num_rows;
		size_t _num_cols;

	public:
		class row;

		matrix() : _num_rows(0), _num_cols(0)
		{ }

		matrix(size_t num_rows, size_t num_cols)
			: _num_rows(num_rows), _num_cols(num_cols), _elements(num_rows * num_cols)
		{ }

		size_t num_rows() const {
			return _num_rows;
		}

		size_t num_cols() const {
			return _num_cols;
		}

		void resize(size_t num_rows, size_t num_cols) {
			_num_rows = num_rows;
			_num_cols = num_cols;
			_elements.resize(num_rows * num_cols);
		}

		T* operator [](size_t i) {
			return _elements.data() + _num_cols * i;
		}

		const T* operator [](size_t i) const {
			return _elements.data() + _num_cols * i;
		}

		T* data() {
			return _elements.data();
		}

		const T* data() const {
			return _elements.data();
		}

		// Added by Jeremy Campbell 9/11/18
		void fill(T item) {
			std::fill(_elements.begin(), _elements.end(), item);
		}

		// Added by Jeremy Campbell 9/11/18
		void fill_region(T item, matrix &m, int x1, int y1, int x2, int y2) {
			for (int i = x1; i <= x2; i++) {
				for (int j = y1; j <= y2; j++) {
					m[i][j] = item;
				}
			}
		}

		void print(std::ostream& os) const {
			os << std::setprecision(4) << std::showpoint << "-\n";
			for (size_t r = 0; r < _num_rows; ++r) {
				os << "| ";
				for (size_t c = 0; c < _num_cols; ++c) {
					os << _elements[r * _num_cols + c] << ' ';
				}
				os << '\n';
			}
			os << '-' << std::endl;
		}

		// Added by Jeremy Campbell 9/11/18
		void operator=(const matrix& m) {
			_elements = m._elements;
			_num_cols = m.num_cols();
			_num_rows = m.num_rows();
		}
	};

	template<typename T>
	inline std::ostream& operator <<(std::ostream& os, const matrix<T>& m) {
		m.print(os);
		return os;
	}
}