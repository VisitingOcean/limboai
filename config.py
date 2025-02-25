# config.py


def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_path():
    return "doc_classes"


# def get_icons_path():
#     return "icons"


def get_doc_classes():
    return [
        "BBAabb",
        "BBArray",
        "BBBasis",
        "BBBool",
        "BBByteArray",
        "BBColor",
        "BBColorArray",
        "BBDictionary",
        "BBFloat",
        "BBFloat32Array",
        "BBFloat64Array",
        "BBInt",
        "BBInt32Array",
        "BBInt64Array",
        "BBNode",
        "BBParam",
        "BBPlane",
        "BBProjection",
        "BBQuat",
        "BBQuaternion",
        "BBRealArray",
        "BBRect2",
        "BBRect2i",
        "BBString",
        "BBStringArray",
        "BBStringName",
        "BBTransform",
        "BBTransform2D",
        "BBTransform3D",
        "BBVariant",
        "BBVector2",
        "BBVector2Array",
        "BBVector2i",
        "BBVector3",
        "BBVector3Array",
        "BBVector3i",
        "BBVector4",
        "BBVector4i",
        "BehaviorTree",
        "BehaviorTreeView",
        "Blackboard",
        "BlackboardPlan",
        "BT",
        "BTAction",
        "BTAlwaysFail",
        "BTAlwaysSucceed",
        "BTAwaitAnimation",
        "BTCallMethod",
        "BTCheckAgentProperty",
        "BTCheckTrigger",
        "BTCheckVar",
        "BTComment",
        "BTComposite",
        "BTCondition",
        "BTConsolePrint",
        "BTCooldown",
        "BTDecorator",
        "BTDelay",
        "BTDynamicSelector",
        "BTDynamicSequence",
        "BTFail",
        "BTForEach",
        "BTInvert",
        "BTNewScope",
        "BTParallel",
        "BTPauseAnimation",
        "BTPlayAnimation",
        "BTPlayer",
        "BTProbability",
        "BTProbabilitySelector",
        "BTRandomSelector",
        "BTRandomSequence",
        "BTRandomWait",
        "BTRepeat",
        "BTRepeatUntilFailure",
        "BTRepeatUntilSuccess",
        "BTRunLimit",
        "BTSelector",
        "BTSequence",
        "BTSetAgentProperty",
        "BTSetVar",
        "BTState",
        "BTStopAnimation",
        "BTSubtree",
        "BTTask",
        "BTTimeLimit",
        "BTWait",
        "BTWaitTicks",
        "LimboHSM",
        "LimboState",
        "LimboUtility",
    ]
