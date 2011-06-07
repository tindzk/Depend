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
		String_Destroy(&item->source);
		String_Destroy(&item->output);
	}

	scall(Items_Free, this->queue);
}

static def(void, addToQueue, String source, String output) {
	fwd(i, this->queue->len) {
		if (String_Equals(this->queue->buf[i].source.rd, source.rd)) {
			String_Destroy(&source);
			String_Destroy(&output);
			return;
		}
	}

	ref(Item) item = {
		.source = source,
		.output = output
	};

	scall(Items_Push, &this->queue, item);
}

static def(String, getSource, RdString path) {
	ssize_t pos = String_ReverseFind(path, '.');

	if (pos == String_NotFound) {
		return String_New(0);
	}

	RdString ext = String_Slice(path, pos);

	if (String_Equals(ext, $("c")) ||
		String_Equals(ext, $("cpp")))
	{
		/* Already a source file. */
		return String_Clone(path);
	}

	String res = String_Clone(String_Slice(path, 0, pos + 1));

	String_Append(&res, 'c');

	if (!Path_Exists(res.rd)) {
		String_Append(&res, $("pp"));

		if (!Path_Exists(res.rd)) {
			res.len = 0;
		}
	}

	return res;
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

static def(void, traverse, Deps_Component *comp) {
	RdString headerPath = comp->path.rd;
	String   sourcePath = call(getSource, headerPath);

	/* Skip all non-source files. */
	if (sourcePath.len == 0) {
		Logger_Debug(this->logger,
			$("'%' has no corresponding source file."), headerPath);
		String_Destroy(&sourcePath);
	} else {
		Logger_Info(this->logger, $("Processing %..."), sourcePath.rd);

		String output = call(getOutput, sourcePath.rd);

		if (output.len == 0) {
			Logger_Error(this->logger,
				$("No output path is mapped for '%'."), sourcePath.rd);
			String_Destroy(&output);
			String_Destroy(&sourcePath);
			throw(RuntimeError);
		}

		if (!Path_Exists(output.rd)) {
			Logger_Info(this->logger, $("Not built yet."));
			call(addToQueue, sourcePath, output);
		} else if (File_IsModified(sourcePath.rd, output.rd)) {
			Logger_Info(this->logger, $("Source modified."));
			call(addToQueue, sourcePath, output);
		} else if (File_IsModified(headerPath, output.rd)) {
			Logger_Info(this->logger, $("Header modified."));
			call(addToQueue, sourcePath, output);
		} else {
			Logger_Debug(this->logger, $("Not building."));
			String_Destroy(&output);
			String_Destroy(&sourcePath);
		}
	}
}

def(void, create) {
	Deps_Components *comps = Deps_getComponents(this->deps);

	fwd(i, comps->len) {
		call(traverse, &comps->buf[i]);
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
		String src = call(getSource, comps->buf[i].path.rd);

		if (src.len != 0) {
			String path = call(getOutput, src.rd);

			if (path.len == 0) {
				StringArray_Destroy(files);
				StringArray_Free(files);

				String_Destroy(&src);

				return NULL;
			}

			if (StringArray_Contains(files, path.rd)) {
				String_Destroy(&path);
			} else {
				StringArray_Push(&files, path);
			}
		}

		String_Destroy(&src);
	}

	return files;
}
