#include "repo.h"

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

// This will get extended as I get more capabilities
static char *cap = "agent=gitorium/"GITORIUM_VERSION;

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

typedef struct commit_node commit_node_t;
struct commit_node
{
	git_commit *commit;
	commit_node_t *next;
};

void repo_upload_pack(git_repository **repo, int stateless) 
{
	commit_node_t *wanted_ref = NULL;
	commit_node_t *common_ref = NULL;

	int multi_ack = 0;

	if (stateless)
	{
		//
	}
	else
	{
		repo_list_refs(repo);
		fflush(stdout);

		for (;;)
		{
			git_oid oid;
			git_commit *commit;
			char *line = gitio_fread_line(stdin);

			if (!line)
				break;

			if (!strprecmp(line, "want "))
			{
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

				commit_node_t new_ref = {commit, wanted_ref};
				wanted_ref = &new_ref;

				continue;
			}
		}

		int found_common = 0;
		
		for (;;)
		{
			git_oid oid;
			git_commit *commit;
			char *line = gitio_fread_line(stdin);

			if (!line && !found_common)
			{
				gitio_write("NAK");
				fflush(stdout);
				break;
			}

			if (!strprecmp(line, "have "))
			{
				if ((0 == multi_ack) && found_common)
					continue;

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

				commit_node_t new_ref = {commit, common_ref};
				common_ref = &new_ref;
				found_common = 1;

				gitio_write("ACK %.40s", line+5);
				fflush(stdout);

				continue;
			}

			if (!strprecmp(line, "done"))
			{
				if (!found_common)
				{
					gitio_write("NAK");
					fflush(stdout);
					break;
				}
			}
		}

		goto cleanup;
	}

	return;

cleanup:
	while (common_ref)
	{
		git_commit_free(common_ref->commit);
		common_ref = common_ref->next;
	}
	while (wanted_ref)
	{
		git_commit_free(wanted_ref->commit);
		wanted_ref = wanted_ref->next;
	}
	return;
}