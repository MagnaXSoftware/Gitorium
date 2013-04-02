Workflow and internals for Gitorium
===================================

End User Usage
--------------

If the repository does not exist yet, the developer must ask the sysadmin to 
create a new repository (see further).

Once the repository is created, the developers can push and pull like they 
normally would:

 1. Add the remote repository/clone the remote repository. At the moment, the 
 only Gitorium-supported protocol is ssh.
    The URL for remote repositories looks like such `ssh://user@host:repo.git`, 
where `user` is the dedicated git user (typically `git`), `host` is the ip or 
hostname of the server and `repo` is the relative path to the repository.
 2. Push && pull!

### Shell

Contrary to most git hosting system, Gitorium offers a minimal interactive shell 
to its users. The shell is restricted to special internal commands and checking 
access rights to a repository.

Setup
-----

The first step is to setup the system by calling `gitorium setup`. That creates 
the administrative git repository, adds the default configuration values and 
imports the administrator's public key.

ATM, the `git` executable is required for the setup.

Administration
--------------

Most administration functions are ran through the `gitorium-admin` repository. 
That way, administrative actions are logged, and there is an easy way of going 
to a previous known good configuration.

Note: the gitorium.conf file is parsed with libconfig. If you know libconfig 
syntax and want to setup fancy systems to separate the definitions of groups and 
repositories, you are free to do so. However, note that there is no support for 
includes.

### Users

Users managed through the `keys` directory in the `gitorium-admin` repository. 
Each key file represents a unique user, thus users cannot have multiple keys.

Adding and removing users is handled by creating and removing their public ssh 
key from the `keys` directory. The list of users is rebuild each time the 
repositories' HEAD is updated.

### Groups

Groups are defined in the `gitorium.conf` file at the root of the 
`gitorium-admin` repository. Groups can only contain users. Groups 
must start with the `*` character.

Gitorium initialises a special _admins_ group. Members of this group have 
access, by default to the `gitorium-admin` repository and to some advanced shell 
functions.

Gitorium defines two special groups. The _all_ group matches all authenticated 
users. The _anon_ group will match all non-authenticated users, if/when 
anonymous access is implemented.

The groups are defined as a key value pair. The key is the name of the group, 
and the value is an array of users.  
In libconfig terms, _groups_ is defined as a group of settings, where each 
setting's value is an array.
    
    groups:
    {
        *group1 = ["user1", "user2"]
        *group2 = ["user1", "user3"]
    }

### Repositories

Repositories are defined in the `gitorium.conf` file. The repositories' name 
must be unique and can only contains characters allowed by the filesystem & ssh. 
However, for more reliable results, names should be limited to alphanumerics, 
underscore (`_`), dash (`-`) and forward slash (`/`). The forward slash can be 
used to nest repositories in subfolders, e.g for subprojects.

On each HEAD update, Gitorium reads the configuration file and creates any 
repository that doesn't exist on the system. Gitorium doesn't delete or 
otherwise modify repositories that are not in the configuration file.  
This can cause problems when a repository is removed (but not deleted) and a new 
one with the same name is added later, but we feel it's better than letting 
Gitorium delete repositories.

The repositories are defined as group of settings. Each repository must define a 
`name` setting. Repositories with no name setting will be ignored. Each 
repository should define a `perms` setting. Repositories with no `perms` 
settings will be considered fully private (no access). The `perms` setting is 
defined as a group of key-value pair, where each key is the name of a group or a 
user the corresponding value is the permission given.
In libconfig terms, _repositories_ is defined as a list of groups, where each 
group has at least a _name_ setting and preferably a _perms_ setting. 
The _perms_ setting is a group where each setting's key is the name of a user or 
group and the value is the permission given.

    repositories:
    (
        {
            name = "repo1";
            perms = {
                user1   = "RW";
                *group1 = "R";
            };
        },
        {
            name = "group2/repo2";
            perms = {
                *all = "R";
                *group2 = "RW";
            };
        }
    )


#### Permissions

Gitorium does authorization (can you do this action) not authentication (are you 
this user). SSH does authentication.

There are, at the moment, three permissions: 

 *  No access, _""_
 *  Read access, _"R"_
 *  Write access, _"W"_

No access prevents the user from interacting with the repository.  
Read access allows the user to pull from the repository (but not to push).  
Write access allows the user to push to the repository (but not to pull).

Gitorium performs authorization before the transfer of data. Gitorium will try 
to match a permission for the specific user, then for any group the user belongs 
to, then for the special _all_ group, then will deny access.

Within a level, the order of the rules matters, as the first rule that matches 
will be the one applied, i.e. if user U is a member of both group A and group B, 
and group A has no access to repository R, but group B has full access, then 
user U will be denied if group A's rule is higher in ther perms setting then 
group B's.

#### Hooks

**NOTE**: The following section is not yet final. The implementation is very 
incomplete and the features have not yet been fully defined.

Eventually, Gitorium will sets up various hooks on the remote repositories and 
will handle the authorisation from there. It will also provide support for 
custom actions to be run before/after the regular hooks.

It will be possible to add custom hooks through the repository configuration 
file. These hooks will be defined per repository and will be executed in the 
order they are added.

##### update

This hook will run once per ref included in the update. It can affect the 
outcome of the push.

##### post-update

This hook will run once the update has been applied. It does not affect the 
outcome of the push. It can be used to notify an email or a bug tracker.

### Shell

**NOTE**: The following section is not yet final. The implementation is very 
incomplete and the features have not yet been fully defined.

All users, when accessing the interactive shell, have access to a few commands, 
including a command to list the repositories, and to check their permission 
against a specific repository.

Members of the _admins_ group have access to additional commands permitting the 
creation & modification of repositories, groups, and users.
