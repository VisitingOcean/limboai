/**
 * bt_run_limit.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_RUN_LIMIT_H
#define BT_RUN_LIMIT_H

#include "../bt_decorator.h"

class BTRunLimit : public BTDecorator {
	GDCLASS(BTRunLimit, BTDecorator);
	TASK_CATEGORY(Decorators);

private:
	int run_limit = 1;
	int num_runs = 0;

protected:
	static void _bind_methods();

	virtual String _generate_name() override;
	virtual Status _tick(double p_delta) override;

public:
	void set_run_limit(int p_value);
	int get_run_limit() const { return run_limit; }
};

#endif // BT_RUN_LIMIT_H
