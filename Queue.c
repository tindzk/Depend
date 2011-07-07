#import "Queue.h"

#define self Queue

rsdef(self, new, Logger *logger, Deps *deps, MappingArray *mappings) {
	return (self) {
		.logger   = logger,
		.queue    = scall(Items_New, 0),
		.deps     = deps,
		.mappings = mappings,
		.ofs      = 0,
		.running  = 0
	};
}

def(void, destroy) {
	each(item, this->queue) {
		String_Destroy(&item->output);
	}

	scall(Items_Free, this->queue);
}

static def(void, addToQueue, RdString source, String output) {
	ref(Item) item = {
		.source = source,
		.output = output,
		.pid    = 0
	};

	scall(Items_Push, &this->queue, item);
}

static def(String, getOutput, RdString path) {
	size_t len = Path_getFileExtension(path).len + 1;

	String out = String_New(0);
	String realpath = Path_expand(path);

	fwd(i, this->mappings->len) {
		String mapping = Path_expand(this->mappings->buf[i].src.rd);

		if (String_BeginsWith(realpath.rd, mapping.rd)) {
			RdString path = String_Slice(realpath.rd, mapping.len, -len);

			out = String_Format($("%%.o"),
				this->mappings->buf[i].dest.rd, path);
		}

		String_Destroy(&mapping);

		if (out.len != 0) {
			break;
		}
	}

	String_Destroy(&realpath);

	return out;
}

static def(bool, traverse, Deps_Component *comp) {
	RdString headerPath = comp->header.rd;
	RdString sourcePath = comp->source.rd;

	if (sourcePath.len == 0) {
		Logger_Debug(this->logger, $("Skipping %..."), headerPath);
		return false;
	}

	Logger_Debug(this->logger, $("Processing %..."), sourcePath);

	String output = call(getOutput, sourcePath);

	if (output.len == 0) {
		Logger_Error(this->logger,
			$("No output path is mapped for '%'."), sourcePath);
		String_Destroy(&output);
		throw(RuntimeError);
	}

	if (!Path_exists(output.rd)) {
		Logger_Debug(this->logger, $("Not built yet."));
		goto build;
	}

	if (File_IsModified(sourcePath, output.rd)) {
		Logger_Debug(this->logger, $("Source modified."));
		goto build;
	}

	if (headerPath.len != 0 && File_IsModified(headerPath, output.rd)) {
		Logger_Debug(this->logger, $("Header modified."));
		goto build;
	}

	when(build) {
		Logger_Debug(this->logger, $("Building %."), sourcePath);
		call(addToQueue, sourcePath, output);
		comp->build = true;
		return true;
	}

	Logger_Debug(this->logger, $("Not building."));
	String_Destroy(&output);
	return false;
}

static def(void, addDependants, Deps_Components *comps, size_t ofs) {
	fwd(i, comps->len) {
		/* Already building? */
		if (comps->buf[i].build) {
			continue;
		}

		RdString sourcePath = comps->buf[i].source.rd;
		Deps_ComponentOffsets *deps = comps->buf[i].deps;

		if (sourcePath.len == 0) {
			continue;
		}

		fwd (j, deps->len) {
			if (deps->buf[j] == ofs) {
				Logger_Debug(this->logger, $("Pulling in dependant %..."),
					sourcePath);
				String output = call(getOutput, sourcePath);
				call(addToQueue, sourcePath, output);
				comps->buf[i].build = true;
				break;
			}
		}
	}
}

def(void, create) {
	Deps_Components *comps = Deps_getComponents(this->deps);

	fwd(i, comps->len) {
		if (comps->buf[i].build) {
			continue;
		}

		if (call(traverse, &comps->buf[i])) {
			call(addDependants, comps, i);
		}
	}

	/* Reset `build' flag. */
	fwd(i, comps->len) {
		comps->buf[i].build = false;
	}
}

def(bool, hasNext) {
	return this->ofs < this->queue->len;
}

def(ref(Item) *, getNext) {
	assert(this->ofs < this->queue->len);
	this->ofs++;
	return &this->queue->buf[this->ofs - 1];
}

def(void, setBuilding, ref(Item) *item, pid_t pid) {
	assert(item != NULL);
	item->pid = pid;
	this->running++;
}

def(void, setBuilt, pid_t pid) {
	assert(this->running != 0);

	fwd(i, this->queue->len) {
		if (this->queue->buf[i].pid == pid) {
			this->queue->buf[i].built = true;
			this->queue->buf[i].pid = 0;
			this->running--;
			return;
		}
	}

	assert(0);
}

def(StringArray *, getLinkingFiles) {
	StringArray *files = StringArray_New(0);

	Deps_Components *comps = Deps_getComponents(this->deps);

	fwd(i, comps->len) {
		RdString src = comps->buf[i].source.rd;

		if (src.len != 0) {
			String path = call(getOutput, src);
			assert(path.len != 0);
			StringArray_Push(&files, path);
		}
	}

	return files;
}
