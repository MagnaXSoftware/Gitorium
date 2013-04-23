#include "ssh.h"

static int ssh__reset(void)
{
	FILE *file;
	char *path;

	config_lookup_string(&aCfg, "keyfile", (const char **)&path);

	if ((file = fopen(path, "w")) == NULL)
	{
		error("Could not reset authorized keys.");
		return GITORIUM_ERROR;
	}

	fclose(file);
	return 0;
}

static int ssh__add(const char *root, const git_tree_entry *entry, void *payload)
{
	printf("root:%s entry:%s\n", root, git_tree_entry_name(entry));
	return 0;

	if (strcmp("keys/", root))
		return 0; // For some reason libgit2 walks from the root tree instead of our subtree

	FILE *auth;
	git_blob *blob;
	char *path, *name = (char *) git_tree_entry_name(entry);
	name = strtok(name, ".");

	config_lookup_string(&aCfg, "keyfile", (const char **)&path);

	printf("Adding user %s\n", name);

	if (git_blob_lookup(&blob, payload, git_tree_entry_id((const git_tree_entry*) entry)))
	{
		error("Could not load the key.");
		return 0;
	}

	if ((auth = fopen(path, "a")) == NULL)
	{
		error("Could not open authorized keys file.");
		return 0;
	}

	fprintf(auth, "command=\""CMAKE_INSTALL_PREFIX"/bin/gitorium-shell %s\",no-port-forwarding,no-X11-forwarding,no-agent-forwarding %s\n", name, (char *) git_blob_rawcontent(blob));
	fclose(auth);

	git_blob_free(blob);

	return 0;
}

int ssh_setup(void)
{
	git_repository *bRepo;
	git_commit *hCommit;
	git_tree *hTree, *mTree;
	git_oid oid;

	char *bFullpath, *rPath;

	config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

	bFullpath = malloc(sizeof(ADMIN_REPO) + sizeof(char)*(strlen(rPath)+1));
	if (NULL == bFullpath)
		return GITORIUM_MEM_ALLOC;
	strcat(strcpy(bFullpath, rPath), ADMIN_REPO);

	if (git_repository_open(&bRepo, bFullpath))
	{
		error("Could not open the admin repository.");
		free(bFullpath);
		return GITORIUM_ERROR;
	}

	free(bFullpath);

	if (git_reference_name_to_id(&oid, bRepo, "refs/heads/master"))
	{
		error("Could not resolve the master.");
		git_repository_free(bRepo);
		return GITORIUM_ERROR;
	}

	if (git_commit_lookup(&hCommit, bRepo, &oid))
	{
		error("Could not load the commit.");
		git_repository_free(bRepo);
		return GITORIUM_ERROR;
	}

	if (git_commit_tree(&hTree, hCommit))
	{
		error("Could not load the main tree.");
		git_commit_free(hCommit);
		git_repository_free(bRepo);
		return GITORIUM_ERROR;
	}

	git_commit_free(hCommit);

	if (git_tree_lookup(&mTree, bRepo, git_tree_entry_id(git_tree_entry_byname(hTree, "keys"))))
	{
		error("Could not find the \"keys\" subtree in the main tree.");
		git_tree_free(hTree);
		git_repository_free(bRepo);
		return GITORIUM_ERROR;
	}

	git_tree_free(hTree);

	if (ssh__reset())
	{
		git_tree_free(mTree);
		git_repository_free(bRepo);
		return GITORIUM_ERROR;
	}

	git_tree_walk(mTree, GIT_TREEWALK_POST, ssh__add, bRepo);

	git_tree_free(mTree);
	git_repository_free(bRepo);

	return 0;
}
