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
        "BBFloatArray",
        "BBInt",
        "BBIntArray",
        "BBNode",
        "BBParam",
        "BBPlane",
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
        "BTAction",
        "BTAlwaysFail",
        "BTAlwaysSucceed",
        "BTCheckAgentProperty",
        "BTCheckTrigger",
        "BTCheckVar",
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
