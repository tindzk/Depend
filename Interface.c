#import "Interface.h"

#define self Interface

def(void, Init, Terminal *term, Logger *logger) {
	this->logger = logger;
	this->action = ref(Action_Unsupported);

	Deps_Init(&this->deps, logger);
	Prototypes_Init(&this->proto);
	Builder_Init(&this->builder, term, logger, &this->deps);
}

def(void, Destroy) {
	Builder_Destroy(&this->builder);
	Prototypes_Destroy(&this->proto);
	Deps_Destroy(&this->deps);
}

def(void, SetAction, RdString action) {
	if (String_Equals(action, $("build"))) {
		this->action = ref(Action_Build);
	} else if (String_Equals(action, $("listdeps"))) {
		this->action = ref(Action_ListDeps);
	} else if (String_Equals(action, $("deptree"))) {
		this->action = ref(Action_DepTree);
	} else if (String_Equals(action, $("print-queue"))) {
		this->action = ref(Action_PrintQueue);
	} else if (String_Equals(action, $("prototypes"))) {
		this->action = ref(Action_Prototypes);
	} else if (String_Equals(action, $("help"))) {
		this->action = ref(Action_Help);
	}
}

def(bool, SetOption, RdString name, RdString value) {
	if (String_Equals(name, $("debug"))) {
		if (String_Equals(value, $("yes"))) {
			BitMask_Set(this->logger->levels, Logger_Level_Debug);
		} else {
			BitMask_Clear(this->logger->levels, Logger_Level_Debug);
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

def(bool, Run) {
	switch (this->action) {
		case ref(Action_Build):
			Deps_Scan(&this->deps);

			if (!Builder_Run(&this->builder)) {
				return false;
			}

			return true;

		case ref(Action_ListDeps):
			Deps_Scan(&this->deps);
			Deps_ListSourceFiles(&this->deps);

			return true;

		case ref(Action_DepTree):
			Deps_Scan(&this->deps);
			Deps_PrintTree(&this->deps);

			return true;

		case ref(Action_PrintQueue):
			Deps_Scan(&this->deps);

			if (!Builder_CreateQueue(&this->builder)) {
				return false;
			}

			Builder_PrintQueue(&this->builder);

			return true;

		case ref(Action_Prototypes):
			Prototypes_Generate(&this->proto);

			return true;

		case ref(Action_Help):
			Logger_Info(this->logger, $(
				"Supported actions are: "
				"build, listdeps, deptree, print-queue, prototypes, help."
			));

			return true;

		case ref(Action_Unsupported):
			Logger_Error(this->logger, $("Action unsupported."));
			return false;
	}

	return false;
}
