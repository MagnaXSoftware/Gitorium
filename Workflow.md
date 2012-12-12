Workflow and internals for Gitorium
===================================

Setup
-----

The first step is to setup the system by calling `gitorium setup`. That creates 
a new administrative git repository in the current dir, adds the default 
configuration values, imports the administration's public key.

After that, a helper program is called that creates a corresponding repository 
at the storage location (set through a configuration file). The local git 
repository is then pushed to the remote location (using file:// protocol). At 
that point, the hooks are setup in the remote admin repository.

Now, the ssh helper is called, it clears the current `authorized_keys` file and adds specially crafted entries to prevent regular shell access using the git user. Initially, only the admin's key is added, as it's the only one present.

Finally, the local repository is destroyed. From that point, management of the system only happens through the administration repository.

Usage
-----

At this point, wild repositories (repositories created on the fly) are not supported, so all the repositories must be defined in the repository configuration before being used.

Once a repository has been defined, it is automatically created by the system, and is ready for use.

Permissions
-----------

There are, at the moment, two permission levels: 
 *  Read, "R"
 *  Write, "W"

Permissions on repositories are checked at various level.  
When issuing a push or pull, gitorium-shell first determines if the repository exists, and if the user can read the repository. If he can't or the repository is nonexistent, the command fails.  
When issuing a push, write permissions are checked directly in the repository by the pre-receive hook. That way, finer permissions can be implemented and run in the hook.

Hooks
-----

Gitorium sets up various hooks on the remote repositories to authorise users to perform commit. **Note: Gitorium does not do authentication. It leaves that task to ssh.**

All the hooks can be customized in the repository configuration file.

### pre-receive

This hook is run once by the remote repository once a user issued a "push" command.

The first thing is it loads the repository configuration to check permission. If the user doesn't have "W" access to the repository, it fails the update.

### update

This hook is run once per ref included in the update. At the moment, there are no gitorium defined action for this hook.

### post-update

This hook is run once the update has been applied. It does not affect the outcome of the push.

This hook is used on the admin repo to check and update the repository list. *Note: Repositories removed from the configuration are simply moved into the `.old` directory. Nothing is deleted.* It is also used in the admin repo to check the user public keys, and update appropriately the `authorized_users` file.
