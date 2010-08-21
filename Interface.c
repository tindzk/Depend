#import "Interface.h"

extern Logger logger;

void Interface_Init(Interface *this) {
	this->action = Interface_Action_Unsupported;

	Deps_Init(&this->deps);
	Prototypes_Init(&this->proto);
	Builder_Init(&this->builder, &this->deps);
}

void Interface_Destroy(Interface *this) {
	Builder_Destroy(&this->builder);
	Prototypes_Destroy(&this->proto);
	Deps_Destroy(&this->deps);
}

void Interface_SetAction(Interface *this, String action) {
	if (String_Equals(action, $("build"))) {
		this->action = Interface_Action_Build;
	} else if (String_Equals(action, $("listdeps"))) {
		this->action = Interface_Action_ListDeps;
	} else if (String_Equals(action, $("deptree"))) {
		this->action = Interface_Action_DepTree;
	} else if (String_Equals(action, $("print-queue"))) {
		this->action = Interface_Action_PrintQueue;
	} else if (String_Equals(action, $("prototypes"))) {
		this->action = Interface_Action_Prototypes;
	} else if (String_Equals(action, $("help"))) {
		this->action = Interface_Action_Help;
	}
}

bool Interface_SetOption(Interface *this, String name, String value) {
	if (String_Equals(name, $("debug"))) {
		if (String_Equals(value, $("yes"))) {
			BitMask_Set(logger.levels, Logger_Level_Debug);
		} else {
			BitMask_Clear(logger.levels, Logger_Level_Debug);
		}
	}

	if (!Deps_SetOption(&this->deps, name, value)) {
		return false;
	}

	if (!Prototypes_SetOption(&this->proto, name, value)) {
		return false;
	}

	if (!Builder_SetOption(&this->builder, name, value)) {
		return false;
	}

	return true;
}

bool Interface_Run(Interface *this) {
	switch (this->action) {
		case Interface_Action_Build:
			Deps_Scan(&this->deps);

			if (!Builder_CreateQueue(&this->builder)) {
				return false;
			}

			if (!Builder_Run(&this->builder)) {
				return false;
			}

			return true;

		case Interface_Action_ListDeps:
			Deps_Scan(&this->deps);
			Deps_ListSourceFiles(&this->deps);

			return true;

		case Interface_Action_DepTree:
			Deps_Scan(&this->deps);
			Deps_PrintTree(&this->deps);

			return true;

		case Interface_Action_PrintQueue:
			Deps_Scan(&this->deps);

			if (!Builder_CreateQueue(&this->builder)) {
				return false;
			}

			Builder_PrintQueue(&this->builder);

			return true;

		case Interface_Action_Prototypes:
			Prototypes_Generate(&this->proto);

			return true;

		case Interface_Action_Help:
			String_Print($(
				"Supported actions are: "
				"build, listdeps, deptree, print-queue, prototypes, help.\n"
			));

			return true;

		case Interface_Action_Unsupported:
			String_Print($("Action unsupported.\n"));

			return false;
	}

	return false;
}
