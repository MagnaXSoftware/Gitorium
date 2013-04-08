#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

/* generic codes stuff */
enum {
	GITORIUM_OK             = 0,
	GITORIUM_ERROR          = -1,
	GITORIUM_MEM_ALLOC      = -2,
	GITORIUM_REPO_EXISTS    = -3,
	GITORIUM_REPO_NOEXISTS  = -4,
	GITORIUM_NOPERM         = -5,
	GITORIUM_EXTERN         = -6,
};

#endif //ERROR_H_INCLUDED