#include "upload-pack.h"

// This will get extended as I get more cap_upabilities
static char *cap_up = "side-band side-band-64k shallow no-progress agent=gitorium/"GITORIUM_VERSION;

// upload-pack stuff
static struct
{
	int multi_ack;
	int side_band;
	int no_progress;
} transfer_flags =
{
	.multi_ack = 0,
	.side_band = 0,
	.no_progress = 0
};

typedef struct commit_node commit_node_t;
struct commit_node
{
	git_oid *id;
	commit_node_t *next;
};

typedef void (*traversal_cb)(const git_oid *, void *);

typedef struct
{
	git_repository *repo;
	git_packbuilder *pb;
} mydata;

static commit_node_t *wanted_ref = NULL;
static commit_node_t *common_ref = NULL;
static commit_node_t *shallow_ref = NULL;
static int depth = 0;


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

static void __insert_commit(const git_oid *id, void *payload)
{
	mydata *info = (mydata *) payload;
	git_commit *commit;

	if (!oid_inlist(common_ref, id))
	{
		git_packbuilder_insert(info->pb, id, NULL);
		git_commit_lookup(&commit, info->repo, id);
		git_packbuilder_insert_tree(info->pb, git_commit_tree_id(commit));
		git_commit_free(commit);
	}

	return;
}

static void __send_sideband(void *buf, size_t size, unsigned int max)
{
	const char *p = buf;
	while (size)
	{
		size_t t = size;

		if (t > max-1)
		{
			t = (max-1);
			size -= t;
		}
		else
			size = 0;

		fprintf(stdout, "%04x%c", (unsigned int) t+5, 1);
		fwrite((void *) p, sizeof(char), t, stdout);

		p += t;
	}
}

static int __send_pack(void *buf, size_t size, void *payload)
{
	UNUSED(payload);
	if (2 == transfer_flags.side_band)
		__send_sideband(buf, size, LARGE_PACKET_SIZE);
	else if (1 == transfer_flags.side_band)
		__send_sideband(buf, size, DEFAULT_PACKET_SIZE);
	else
		fwrite(buf, sizeof(char), size, stdout);

	return 0;
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

				if (0 == transfer_flags.multi_ack)
				{
					if (!found_common)
					{
						found_common = 1;
						gitio_write("ACK %.40s\n", line+5);
					}
				}

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

static int repo__build_send_pack(git_repository *repo)
{
	git_packbuilder *pb;

	if (git_packbuilder_new(&pb, repo))
	{
		fatal("unexpected internal error");
		return GITORIUM_ERROR;
	}

	// using multiple threads allows for faster processing, but the order of the
	// objects cannot be guaranteed.
	// XXX: should we using multiple threads?
	/* git_packbuilder_set_threads(pb, 0); */

	mydata info = {repo, pb};
	commit_node_t *cur = wanted_ref;

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

void repo_list_refs(git_repository **repo)
{
	git_strarray ref_list;

	git_reference_list(&ref_list, *repo, GIT_REF_LISTALL);

	if (0 == ref_list.count)
		gitio_write("0000000000000000000000000000000000000000 cap_upabilities^{}%c%s\n", 0, cap_up);
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
			gitio_write("%s HEAD%c%s\n", out, 0, cap_up);
			cap_up = NULL;
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

			if (cap_up)
			{
				gitio_write("%s %s%c%s\n", out, refname, 0, cap_up);
				cap_up = NULL;
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

					if (strstr(cap, "side-band-64k"))
						transfer_flags.side_band = 2;
					else if (strstr(cap, "side-band"))
						transfer_flags.side_band = 1;

					if (strstr(cap, "no-progress"))
						transfer_flags.no_progress = 1;
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

		if (transfer_flags.side_band)
			gitio_fflush(stdout);

		fflush(stdout);

		goto cleanup;
	}

	return;

	cleanup:
	return;
}