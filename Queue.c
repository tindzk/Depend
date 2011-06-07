#import "Queue.h"

#define self Queue

rsdef(self, new, Logger *logger, Deps *deps, MappingArray *mappings) {
	return (self) {
		.logger   = logger,
		.queue    = scall(Items_New, 0),
		.deps     = deps,
		.mappings = mappings,
		.ofs      = 0
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
		.output = output
	};

	scall(Items_Push, &this->queue, item);
}

static def(String, getOutput, RdString path) {
	size_t len = Path_GetExtension(path).len + 1;

	String out = String_New(0);
	String realpath = Path_Resolve(path);

	fwd(i, this->mappings->len) {
		String mapping = Path_Resolve(this->mappings->buf[i].src.rd);

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
		Logger_Info(this->logger, $("Skipping %..."), headerPath);
		return false;
	}

	Logger_Info(this->logger, $("Processing %..."), sourcePath);

	String output = call(getOutput, sourcePath);

	if (output.len == 0) {
		Logger_Error(this->logger,
			$("No output path is mapped for '%'."), sourcePath);
		String_Destroy(&output);
		throw(RuntimeError);
	}

	if (!Path_Exists(output.rd)) {
		Logger_Info(this->logger, $("Not built yet."));
		call(addToQueue, sourcePath, output);
		return true;
	}

	if (File_IsModified(sourcePath, output.rd)) {
		Logger_Info(this->logger, $("Source modified."));
		call(addToQueue, sourcePath, output);
		return true;
	}

	if (headerPath.len != 0 && File_IsModified(headerPath, output.rd)) {
		Logger_Info(this->logger, $("Header modified."));
		call(addToQueue, sourcePath, output);
		return true;
	}

	Logger_Debug(this->logger, $("Not building."));
	String_Destroy(&output);
	return false;
}

static def(void, addDependants, Deps_Components *comps, size_t ofs) {
	fwd(i, comps->len) {
		RdString sourcePath = comps->buf[i].source.rd;
		Deps_ComponentOffsets *deps = comps->buf[i].deps;

		if (sourcePath.len == 0) {
			continue;
		}

		fwd (j, deps->len) {
			if (deps->buf[j] == ofs) {
				Logger_Info(this->logger, $("Pulling in dependant %..."),
					sourcePath);
				String output = call(getOutput, sourcePath);
				call(addToQueue, sourcePath, output);
				break;
			}
		}
	}
}

def(void, create) {
	Deps_Components *comps = Deps_getComponents(this->deps);

	fwd(i, comps->len) {
		if (call(traverse, &comps->buf[i])) {
			call(addDependants, comps, i);
		}
	}
}

def(bool, hasNext) {
	return this->ofs < this->queue->len;
}

def(ref(Item), getNext) {
	assert(this->ofs < this->queue->len);
	this->ofs++;
	return this->queue->buf[this->ofs - 1];
}

def(size_t, getTotal) {
	return this->queue->len;
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
