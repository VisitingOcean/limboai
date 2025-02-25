/**
 * bb_transform3d.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BB_TRANSFORM3D_H
#define BB_TRANSFORM3D_H

#include "bb_param.h"

class BBTransform3D : public BBParam {
	GDCLASS(BBTransform3D, BBParam);

protected:
	virtual Variant::Type get_type() const override { return Variant::TRANSFORM3D; }
};

#endif // BB_TRANSFORM3D_H