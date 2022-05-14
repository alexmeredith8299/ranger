/*-------------------------------------------------------------------------------
 This file is part of ranger.

 Copyright (c) [2014-2018] [Marvin N. Wright]

 This software may be modified and distributed under the terms of the MIT license.

 Please note that the C++ core of ranger is distributed under MIT license and the
 R package "ranger" under GPL3 license.
 #-------------------------------------------------------------------------------*/

// Ignore in coverage report (not used in R package)
// #nocov start
#ifndef DATAINT_H_
#define DATAINT_H_

#include <vector>
#include <utility>

#include "globals.h"
#include "utility.h"
#include "Data.h"

namespace ranger {

class DataInt: public Data {
public:
  DataInt() = default;

  DataInt(const DataInt&) = delete;
  DataInt& operator=(const DataInt&) = delete;

  virtual ~DataInt() override = default;

  double get_x(size_t row, size_t col) const override {
    // Use permuted data for corrected impurity importance
    size_t col_permuted = col;
    if (col >= num_cols) {
      col = getUnpermutedVarID(col);
      row = getPermutedSampleID(row);
    }

    if (col < num_cols_no_snp) {
      return x[col * num_rows + row];
    } else {
      return getSnp(row, col, col_permuted);
    }
  }

  double get_y(size_t row, size_t col) const override {
    return y[col * num_rows + row];
  }

  void reserveMemory(size_t y_cols) override {
    x.resize(num_cols * num_rows);
    y.resize(y_cols * num_rows);
  }

  void set_x(size_t col, size_t row, double value, bool& error) override {
    x[col * num_rows + row] = value;
  }

  void set_y(size_t col, size_t row, double value, bool& error) override {
    y[col * num_rows + row] = value;
  }

private:
  std::vector<uint32_t> x;
  std::vector<uint32_t> y;
};

} // namespace ranger

#endif /* DATAINT_H_ */
// #nocov end
