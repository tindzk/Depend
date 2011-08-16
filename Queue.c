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

static def(void, addToQueue, RdString source, String output, RdString namespace) {
	ref(Item) item = {
		.source = source,
		.output = output,
		.namespace = namespace,
		.pid = 0
	};

	scall(Items_Push, &this->queue, item);
}

static def(String, getOutput, RdString path, RdString *namespace) {
	size_t len = Path_getFileExtension(path).len + 1;

	String out = String_New(0);
	String realpath = Path_expand(path);

	fwd(i, this->mappings->len) {
		String mapping = Path_expand(this->mappings->buf[i].src.rd);

		if (String_BeginsWith(realpath.rd, mapping.rd)) {
			RdString path = String_Slice(realpath.rd, mapping.len, -len);

			out = String_Format($("%%.o"),
				this->mappings->buf[i].dest.rd, path);

			if (namespace != null) {
				*namespace = this->mappings->buf[i].namespace.rd;
			}
		}

		String_Destroy(&mapping);

		if (out.len != 0) {
			break;
		}
	}

	String_Destroy(&realpath);

	return out;
}

static sdef(bool, wasModified, RdString sourceFile, RdString outputFile) {
	Stat64 src = Path_getMeta(sourceFile);
	Stat64 out = Path_getMeta(outputFile);

	return src.mtime.sec > out.mtime.sec;
}

static def(bool, buildSource, Deps_Component *comp) {
	RdString headerPath = comp->header.rd;
	RdString sourcePath = comp->source.rd;

	if (sourcePath.len == 0) {
		Logger_Debug(this->logger,
			t("No source file found for %."), headerPath);
		return false;
	}

	Logger_Debug(this->logger, t("Processing source %..."), sourcePath);

	RdString namespace;

	String output = call(getOutput, sourcePath, &namespace);

	if (output.len == 0) {
		Logger_Debug(this->logger, t("No output path mapped."));
		goto notBuilding;
	}

	if (!Path_exists(output.rd)) {
		Logger_Debug(this->logger, t("Not built yet."));
		goto build;
	}

	if (scall(wasModified, sourcePath, output.rd)) {
		Logger_Debug(this->logger, t("Source modified."));
		goto build;
	}

	if (headerPath.len != 0 && scall(wasModified, headerPath, output.rd)) {
		Logger_Debug(this->logger, t("Header modified."));
		goto build;
	}

	when(build) {
		Logger_Debug(this->logger, t("Building %."), sourcePath);
		call(addToQueue, sourcePath, output, namespace);
		comp->build = true;
		return true;
	}

notBuilding:
	Logger_Debug(this->logger, t("Not building."));
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
				Logger_Debug(this->logger, t("Pulling in dependant %..."),
					sourcePath);

				RdString namespace;
				String output = call(getOutput, sourcePath, &namespace);

				if (output.len == 0) {
					Logger_Debug(this->logger, t("No output path mapped."));
				} else {
					call(addToQueue, sourcePath, output, namespace);
					comps->buf[i].build = true;
				}

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

		if (call(buildSource, &comps->buf[i])) {
			call(addDependants, comps, i);
		}
	}

	/* Reset `build' flag. */
	fwd(i, comps->len) {
		comps->buf[i].build = false;
	}
}

/* Deletes already existing files which will be (re-)built anyway.
 * If the user decided to kill Depend, this ensures that after
 * restarting the process Depend will continue where the compilation
 * stopped. Otherwise, some dependencies that were previously pulled
 * in won't be built because their dependants might have already been
 * compiled when the user interrupted the process.
 */
def(void, purge) {
	Deps_Components *comps = Deps_getComponents(this->deps);

	each(item, this->queue) {
		if (Path_exists(item->output.rd)) {
			Logger_Info(this->logger, t("Purging %..."), item->output.rd);
			Path_deleteFile(item->output.rd);
		}
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
	assert(item != null);
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
			String path = call(getOutput, src, null);

			if (path.len == 0) {
				String_Destroy(&path);
			} else {
				StringArray_Push(&files, path);
			}
		}
	}

	return files;
}
