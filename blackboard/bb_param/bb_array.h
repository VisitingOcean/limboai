/**
 * bb_array.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_ARRAY_H
#define BB_ARRAY_H

#include "bb_param.h"

class BBArray : public BBParam {
	GDCLASS(BBArray, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::ARRAY; }
};

#endif // BB_ARRAY_H