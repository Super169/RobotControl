#include "robot.h"

void ActionPlaySystemAction(byte systemActionId) {
	_dbg->msg("ActionPlaySystemAction: %d", systemActionId);
	switch (systemActionId) {
		case 1:
			EyeLedHandle();
			break;	
	}
}