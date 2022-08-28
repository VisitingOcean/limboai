/* bt_selector.h */

#ifndef BT_SELECTOR_H
#define BT_SELECTOR_H

#include "bt_composite.h"

class BTSelector : public BTComposite {
	GDCLASS(BTSelector, BTComposite);

private:
	int last_running_idx = 0;

protected:
	virtual void _enter();
	virtual int _tick(float p_delta);
};
#endif // BT_SELECTOR_H