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
			symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-post-update", puPath);
			free(puPath);

			char *uPath = malloc(sizeof(char) * (strlen(nFullpath) + strlen("/hooks/update") + 1));
			if (NULL == uPath)
				goto cleanup_mem;

			strcat(strcpy(uPath, nFullpath), "/hooks/update");
			symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-update", uPath);
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

	if ('/' == orig[strlen(orig)-1])
		orig[strlen(orig)-1] = 0;

	if (!strcmp(".git", &orig[strlen(orig)-4]))
		orig[strlen(orig)-4] = 0;

	return orig;
}
