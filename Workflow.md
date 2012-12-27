Workflow and internals for Gitorium
===================================

End User Usage
--------------

If the repository does not exist yet, the developer must ask the sysadmin (that's you!) to create a new repository (see further).

Once the repository is created, the developers can push and pull like they normally would.

 1. Add the remote repository/clone the remote repository. At the moment, the only supported protocol is ssh. The git, http and file protocols can be manually activated for the individual repositories, but they bypass all the access control and some other functionnality implemented by Gitorium.  
The URL for remote repositories looks like such `ssh://user@host:repo.git`, where `user` is the dedicated git user (typically `git`), `host` is the ip or hostname of the server and `repo` is the relative path to the repository.
 2. Push && pull!

### Shell

Contrary to most git hosting system, Gitorium offers a minimal interactive shell to its users. The shell is limited to running commands such as `list-repos` or checking access rights to the repositories.

Setup
-----

The first step is to setup the system by calling `gitorium setup`. That creates 
a new administrative git repository in the current dir, adds the default 
configuration values, imports the administration's public key.

After that, a helper program is called that creates a corresponding repository 
at the storage location (set through a configuration file). The local git 
repository is then pushed to the remote location (using the `file://` protocol). At 
that point, the hooks are setup in the remote admin repository.

Now, the ssh helper is called, it clears the current `authorized_keys` file and adds specially crafted entries to prevent regular shell access using the git user. Initially, only the admin's key is added, as it's the only one present.

Finally, the local repository is destroyed. From that point, management of the system only happens through the administration repository.

Administration
--------------

Most administration functions are ran through the `gitorium-admin` repository. That way, administrative actions are logged, and there is a full history of all that happend.

Note: the gitorium.conf file is parsed with libconfig. If you know libconfig syntax and want to setup fancy systems to separate the definitions of groups and repositories, you are free to do so.

### Users

Users managed through the `keys` directory in the `gitorium-admin` repository. Each key represents a user, and therefore duplicate users are not supported. In other words, each key is mapped by gitorium to a unique user, and vice-versa.

Adding and removing users is handled by creating and removing their public ssh key from the `keys` directory. Editing a file will update that user's key.

The system is setup so that the list of users is re-built each time the remote repository is updated.

### Groups

Groups are defined in the `gitorium.conf` file at the root of the `gitorium-admin` repository. The quick difference between users and groups is that groups must start with an asterisk (`*`). Group names must be unique.

Gitorium defines, on the fly, the `*all` group, which matches all users with an ssh key. If/when we wrap gitweb, we might also define an `*anonymous` group.

The groups are defined as a key value pair, one per line. The key is the name of the group, while the value is a bracket enclosed comma separated list of users and/or groups.  
In libconfig terms, groups is defined as a group of settings, where each setting's value is an array.
    
    groups:
    {
        *group1 = ["user1", "user2", "*group1"]
        *group2 = ["user1", "user3"]
    }

### Repositories

Repositories are also defined in the `gitorium.conf` file at the root of the `gitorium-admin` repository. Groups names must be unique and can only contains characters allowed by the filesystem & ssh. However, for more reliable results, name should be limited to alphanumerics, underscore (`_`), dash (`-`) and forward slash (`/`). The forward slash can be used to nest repositories in subfolders, i.e to make personal repositories (when wild repositories will be implemented, this will be more useful).

After the admin repository is updated, Gitorium reads the configuration file and creates any repository that doesn't exist on the system. Gitorium doesn't delete or otherwise modify repositories that are not in the configuration file. This can cause problems when a repository is removed from the configuration file and a new one with the same name is added later.

Repositories are defined inside a `{}` pair. Each repository must have at least a name key. If there is no name key, that repository entry is considered non-valid. Repositories do not need to contain a perms key, but if they don't, no one has access to that repository.  
In libconfig temrs, repositories is defined as a list of groups, where each group has at least a `name` setting and preferably a `perms` setting. The `perms` setting is a group where each setting's key is the name of a user or group and the value is the permission.

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

There are, at the moment, three permissions: 

 *  No access, ""
 *  Read, "R"
 *  Write, "W"

It is possible to give write permission without giving read permission.

Gitorium performs authorization before anything is transfered.

Permissions are defined on 3 levels: the user level, the group level and the all level. If no permission is found on one level, Gitorium tries to find one on a higher level, until it has reached the highest, and then denies access by default.

At the user level, it tries to find a rule for the specific user. The first rule that matches is the one used.

At the group level, it iterates through all the groups (except `*all`) to find those that include the user. It then combines the rules of the user's group to get the final permission.

At the all level, it searches for the special `*all` group and uses that permission. The first rule that matches is the one used.

Adding an exclamation mark (`!`) before a permission ("!", "!R", "!W" instead of "", "R", "W") will make that rule the most important at the current permission level. It will stop further permission processing as soon as it is encountered.

#### Hooks

Eventually, Gitorium will sets up various hooks on the remote repositories and will handle the authorisation from there. It will also provide support for custom actions to be run before/after the regular hooks.

It will be possible to add custom hooks through the repository configuration file. These hooks will be defined per repository and will be executed in the order they are added.

##### update

This hook will run once per ref included in the update. At the moment, there are no gitorium defined action for this hook.

##### post-update

This hook will run once the update has been applied. It does not affect the outcome of the push. It can be used to notify an email or a bug tracker.

### Shell

We want to include, in the shell, functions to facilitate adding repositories, groups and users. These functions would only be available to users in the `admin` group.
