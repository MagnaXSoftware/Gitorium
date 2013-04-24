#ifndef UPLOAD_PACK_H_INCLUDED
#define UPLOAD_PACK_H_INCLUDED

#include "common.h"

#include <git2.h>
#include <unistd.h>

#include "gitio.h"

void    repo_list_refs     (git_repository **repo);
void    repo_upload_pack   (git_repository **repo, int stateless);

#endif // UPLOAD_PACK_H_INCLUDED
