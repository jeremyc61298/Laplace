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

		void print(std::ostream& os) const {
			os << std::setprecision(4) << std::showpoint << "-\n";
			for (size_t r = 0; r < _num_rows; ++r) {
				os << "| ";
				for (size_t c = 0; c < _num_cols; ++c) {
					os << _elements[r * _num_cols + c] << ' ';
				}
				os << '\n';
			}
			os << '-' << endl;
		}
	};

	template<typename T>
	inline std::ostream& operator <<(std::ostream& os, const matrix<T>& m) {
		m.print(os);
		return os;
	}
}