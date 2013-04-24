#include "repo.h"

// This will get extended as I get more capabilities
static char *cap = "side-band agent=gitorium/"GITORIUM_VERSION;

// upload-pack stuff
typedef struct
{
	int multi_ack;
	int side_band;
} flag_t;

static flag_t transfer_flags =
{
	.multi_ack = 0,
	.side_band = 0
};

typedef struct commit_node commit_node_t;
struct commit_node
{
	git_oid *id;
	commit_node_t *next;
};

static commit_node_t *wanted_ref = NULL;
static commit_node_t *common_ref = NULL;
static commit_node_t *shallow_ref = NULL;
static int depth = 0;

int repo_create(char *name)
{
	char *nFullpath, *rPath;
	struct stat rStat;
	int ret = GITORIUM_ERROR;

	config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

	nFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(name) + strlen(".git") + 1));
	if (NULL == nFullpath)
		return GITORIUM_MEM_ALLOC;

	strcat(strcat(strcpy(nFullpath, rPath), name), ".git");

	if (stat(nFullpath, &rStat))
	{
		git_repository *nRepo;

		printf("Creating repo: %s\n", name);
		fflush(stdout);

		int repc = git_repository_init(&nRepo, nFullpath, 1);
		git_repository_free(nRepo);

		if (!repc)
		{
			char *puPath = malloc(sizeof(char) * (strlen(nFullpath) + strlen("/hooks/post-update") + 1));
			if (NULL == puPath)
				goto cleanup_mem;

			strcat(strcpy(puPath, nFullpath), "/hooks/post-update");
			symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook", puPath);
			free(puPath);

			char *uPath = malloc(sizeof(char) * (strlen(nFullpath) + strlen("/hooks/update") + 1));
			if (NULL == uPath)
				goto cleanup_mem;

			strcat(strcpy(uPath, nFullpath), "/hooks/update");
			symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook", uPath);
			free(uPath);
		}

		ret = 0;
	}
	else
		ret = GITORIUM_REPO_EXISTS;

	free(nFullpath);
	return ret;

	cleanup_mem:
	rrmdir((const char *) nFullpath);
	free(nFullpath);
	return GITORIUM_MEM_ALLOC;
}

char *repo_massage(char *orig)
{
	if ('\'' == orig[0])
		orig++;

	if ('\'' == orig[strlen(orig)-1])
		orig[strlen(orig)-1] = 0;

	if ('/' == orig[0])
		orig++;

	if ('/' == orig[strlen(orig)-1])
		orig[strlen(orig)-1] = 0;

	if (!strcmp(".git", &orig[strlen(orig)-4]))
		orig[strlen(orig)-4] = 0;

	return orig;
}

void repo_list_refs(git_repository **repo)
{
	git_strarray ref_list;

	git_reference_list(&ref_list, *repo, GIT_REF_LISTALL);

	if (0 == ref_list.count)
		gitio_write("0000000000000000000000000000000000000000 capabilities^{}%c%s\n", 0, cap);
	else
	{
		git_reference *ref;
		git_tag *tag;
		char out[41];
		out[40] = 0;

		if (!git_reference_lookup(&ref, *repo, "HEAD"))
		{
			git_reference *ref_head;
			git_reference_resolve(&ref_head, ref);
			git_oid_fmt(out, git_reference_target(ref_head));
			gitio_write("%s HEAD%c%s\n", out, 0, cap);
			cap = NULL;
			git_reference_free(ref_head);
		}

		git_reference_free(ref);

		for (unsigned int i = 0; i < ref_list.count; i++)
		{
			/* code */
			const char *refname = ref_list.strings[i];
			
			git_reference_lookup(&ref, *repo, refname);

			if (GIT_REF_SYMBOLIC == git_reference_type(ref))
			{
				git_reference *goal;
				git_reference_resolve(&goal, ref);
				git_reference_free(ref);
				git_reference_resolve(&ref, goal);
				git_reference_free(goal);
			}

			git_oid_fmt(out, git_reference_target(ref));

			if (cap)
			{
				gitio_write("%s %s%c%s\n", out, refname, 0, cap);
				cap = NULL;
			}
			else
				gitio_write("%s %s\n", out, refname);

			if (!git_tag_lookup(&tag, *repo, git_reference_target(ref)))
			{
				git_object *target;

				if (!git_tag_peel(&target, tag))
				{
					git_oid_fmt(out, git_object_id(target));
					gitio_write("%s %s^{}\n", out, refname);
				}

				git_object_free(target);
			}

			git_tag_free(tag);
			git_reference_free(ref);
		}
	}
	
	gitio_fflush(stdout);

	git_strarray_free(&ref_list);
}

static int oid_inlist(const commit_node_t * const start, const git_oid *id)
{
	const commit_node_t *cur = start;
	while (cur)
	{
		if (!git_oid_cmp(cur->id, id))
			return 1;

		cur = cur->next;
	}
	return 0;
}

static int repo__get_common(git_repository *repo)
{
	int found_common = 0;

	for (;;)
	{
		git_oid oid;
		git_commit *commit;
		char *line = gitio_fread_line(stdin);

		if (!line && !found_common)
		{
			gitio_write("NAK\n");
			return 0;
		}

		if (!strprecmp(line, "have "))
		{
			if ((0 == transfer_flags.multi_ack) && found_common)
				continue;

			if (git_oid_fromstr(&oid, line+5))
			{
				fatalf("protocol error, expected to get sha, not '%s'", line);
				return GITORIUM_ERROR;
			}

			if (git_commit_lookup(&commit, repo, &oid))
			{
				fatalf("not our ref %.40s", line+5);
				return GITORIUM_ERROR;
			}

			git_commit_free(commit);

			if (!oid_inlist(common_ref, &oid))
			{

				commit_node_t new_ref = {&oid, common_ref};
				common_ref = &new_ref;
				found_common = 1;

				gitio_write("ACK %.40s\n", line+5);
				fflush(stdout);
			}

			continue;
		}

		if (!strprecmp(line, "done") && !found_common)
		{
			gitio_write("NAK\n");
			return 0;
		}
	}
	return GITORIUM_ERROR;
}

typedef void (*traversal_cb)(const git_oid *, void *);

static void load_ancestors(git_commit *commit, int dep, traversal_cb cb, void *payload)
{
	if (0 == dep)
		(*cb)(git_commit_id(commit), payload);
	else
	{
		for (unsigned int i = 0; i < git_commit_parentcount(commit); i++)
		{
			git_commit *ancestor;
			git_commit_parent(&ancestor, commit, i);
			load_ancestors(ancestor, dep-1, cb, payload);
			git_commit_free(ancestor);
		}
	}
}

static void traverse_ancestors(git_commit *commit, int dep, traversal_cb cb, void *payload)
{
	(*cb)(git_commit_id(commit), payload);
	if (0 == depth || 0 < dep)
	{
		for (unsigned int i = 0; i < git_commit_parentcount(commit); i++)
		{
			git_commit *ancestor;
			git_commit_parent(&ancestor, commit, i);
			// In the case that depth is zero, we don't care about the value of 
			// dep. In the case that depth is non-zero, we are sure that it will 
			// be a positive or zero value if we reach this point. Therefore, we
			// can cast it to an unsigned int before removing one from it.
			traverse_ancestors(ancestor, ((unsigned int) dep)-1, cb, payload);
			git_commit_free(ancestor);
		}	
	}
}

static void __print_shallow(const git_oid *oid, void *payload)
{
	UNUSED(payload);
	char id[40];
	git_oid_fmt(id, oid);
	gitio_write("shallow %.40s\n", id);
}

static void __check_shallow(const git_oid *oid, void *payload)
{
	UNUSED(payload);
	commit_node_t *cur = shallow_ref;

	if (NULL == cur)
		return;

	if (!git_oid_cmp(oid, cur->id))
	{
		char id[40];

		git_oid_fmt(id, oid);
		gitio_write("unshallow %.40s\n", id);

		shallow_ref = shallow_ref->next;
		return;
	}

	while (cur->next)
	{
		if (!git_oid_cmp(oid, cur->next->id))
		{
			char id[40];

			git_oid_fmt(id, oid);
			gitio_write("unshallow %.40s\n", id);

			cur->next = cur->next->next;
			return;
		}
	}
}

static int repo__shallow_update(git_repository *repo)
{
	if (0 < depth)
	{
		commit_node_t *cur = wanted_ref;
		while (cur)
		{
			git_commit *commit;

			git_commit_lookup(&commit, repo, cur->id);
			load_ancestors(commit, depth, &__print_shallow, NULL);
			git_commit_free(commit);

			cur = cur->next;
		}

		cur = wanted_ref;
		while (cur)
		{
			git_commit *commit;

			git_commit_lookup(&commit, repo, cur->id);
			traverse_ancestors(commit, depth, &__check_shallow, NULL);
			git_commit_free(commit);

			cur = cur->next;
		}
	}
	else
	{
		commit_node_t *cur = shallow_ref;
		while (cur)
		{
			char id[40];

			git_oid_fmt(id, cur->id);
			gitio_write("unshallow %.40s\n", id);

			cur = cur->next;
		}
	}

	gitio_fflush(stdout);

	return 0;
}

struct mydata
{
	git_repository *repo;
	git_packbuilder *pb;
};

static void __insert_commit(const git_oid *id, void *payload)
{
	struct mydata *info = (struct mydata *) payload;
	git_commit *commit;

	git_packbuilder_insert(info->pb, id, NULL);
	git_commit_lookup(&commit, info->repo, id);
	git_packbuilder_insert_tree(info->pb, git_commit_tree_id(commit));
	git_commit_free(commit);

	return;
}

static int __send_pack(void *buf, size_t size, void *payload)
{
	UNUSED(payload);
	if (2 == transfer_flags.side_band)
	{
		//
	}
	else if (1 == transfer_flags.side_band)
	{
		size_t n = size;
		const char *p = buf;
		while (size)
		{
			if (DEFAULT_PACKET_SIZE < n+1)
			{
				n -= (DEFAULT_PACKET_SIZE-1);
				p += (DEFAULT_PACKET_SIZE-1);
			}
			else
				n = 0;

			gitio_write("%c%.999s", '\1', p);
		}
	}
	else
	{
		fwrite(buf, sizeof(char), size, stdout);
	}
	return 0;
}

static int repo__build_send_pack(git_repository *repo)
{
	git_packbuilder *pb;

	if (git_packbuilder_new(&pb, repo))
	{
		fatal("unexpected internal error");
		return GITORIUM_ERROR;
	}

	git_packbuilder_set_threads(pb, 0);

	commit_node_t *cur = wanted_ref;

	struct mydata info = {repo, pb};

	while (cur)
	{
		git_commit *commit;

		git_commit_lookup(&commit, repo, cur->id);
		traverse_ancestors(commit, depth, &__insert_commit, (void *) &info);
		git_commit_free(commit);

		cur = cur->next;
	}

	git_packbuilder_foreach(pb, &__send_pack, (void *) NULL);

	return 0;
}

void repo_upload_pack(git_repository **repo, int stateless) 
{
	if (stateless)
	{
		//
	}
	else
	{
		repo_list_refs(repo);
		fflush(stdout);

		int have_flags = 0;

		for (;;)
		{
			git_oid oid;
			git_commit *commit;
			char *line = gitio_fread_line(stdin);

			if (!line)
				break;

			if (!strprecmp(line, "shallow "))
			{
				if (git_oid_fromstr(&oid, line+8))
				{
					fatalf("invalid shallow line: %s", line);
					goto cleanup;
				}

				if (git_commit_lookup(&commit, *repo, &oid))
				{
					fatalf("invalid shallow object %.40s", line+8);
					goto cleanup;
				}

				git_commit_free(commit);

				if (!oid_inlist(shallow_ref, &oid))
				{
					commit_node_t new_ref = {&oid, shallow_ref};
					shallow_ref = &new_ref;
				}

				continue;
			}

			if (!strprecmp(line, "want "))
			{
				if (!have_flags)
				{
					have_flags = 1;
					const char *cap = line+46;

					if (strstr(cap, "side-band"))
						transfer_flags.side_band = 1;
				}

				if (git_oid_fromstr(&oid, line+5))
				{
					fatalf("protocol error, expected to get sha, not '%s'", line);
					goto cleanup;
				}

				if (git_commit_lookup(&commit, *repo, &oid))
				{
					fatalf("not our ref %.40s", line+5);
					goto cleanup;
				}

				git_commit_free(commit);

				if (!oid_inlist(wanted_ref, &oid))
				{
					commit_node_t new_ref = {&oid, wanted_ref};
					wanted_ref = &new_ref;
				}

				continue;
			}

			if (!strprecmp(line, "deepen "))
			{
				char *end;

				depth = strtol(line + 7, &end, 0);
				if (end == line + 7 || depth <= 0)
				{
					fatalf("invalid deepen: %s", line);
					goto cleanup;
				}

				continue;
			}
		}

		if (depth)
		{
			if (repo__shallow_update(*repo))
				goto cleanup;

			fflush(stdout);
		}

		if (repo__get_common(*repo))
			goto cleanup;

		fflush(stdout);

		if (repo__build_send_pack(*repo))
			goto cleanup;

		goto cleanup;
	}

	return;

	cleanup:
	return;
}