GITORIUM
========

by MagnaX Software

Installation
------------

See [INSTALL.md](INSTALL.md) for details on installation. 

It's mostly install the dependencies, compile the source & install.

Setup
-----

If Gitorium was installed using a pre-compiled package (deb/rpm), the `git` user 
and the `/var/repositories` directory should have been created for you. If they 
weren't, or you built from source, create a regular user named `git` and a 
directory `/var/repository`. Then, set the directories' owner to `git`. 

The `git` user should not be allowed to login with a password on SSH.

Generate a SSH keypair for your gitorium administration. Lookup how to do so 
with `ssh-keygen`.  
We recommend having a dedicated administration user and a regular every-day 
user. That way, if your regular keypair is compromised, the administrator's will 
still be valid.

Rename your public key as the username of the administrator user with the pub 
extension, i.e. for the user `administrator`, name the public key 
`administrator.pub`.

Change the logged-in user to `git` (you may have to run the following as root)

    su git

Run the setup command

    gitorium setup '/path/to/admin/public/key'

Administration
--------------

Gitorium's administration is mostly done through a special git repository. Using 
the admin's key pair, clone the `gitorium-admin` repository. That repository 
holds a configuration file, `gitorium.conf`, and a directory, `keys`.

The `gitorium.conf` file holds the definition of each repository and each group. 
The required format is documented in the configuration file itself.

The `keys` subdirectory contains the public keys of each user of the system. The 
name of the keys is the name of their user, and thus a single user cannot have 
multiple keys.

The last part of the configuration is accomplished through another configuration 
file, most likely located in `/etc/gitorium/config.cfg`. That file contains 
configuration options that affect Gitorium itself. For example, the location of 
the repositories or the location of the ssh authorized_keys can be set and 
modified there.

Usage
-----

Gitorium aims to make itself transparent to the end user. For that reason, 
Gitorium-hosted git repositories can be cloned over ssh the same way regular 
repositories can (they are regular repositories after all). Smart HTTP is in the 
works.

However, Gitorium provides a few additions to plain old git (POG). The first 
being an interactive shell. The shell provides access to a few Gitorium & git 
utilities. Creating groups & repositories will eventually be possible using the 
gitorium shell.

The second addition is support for on-the-fly personal repositories. Users are 
able to create, on first push, private repositories. These repositories are 
unlisted, and their access is restricted to the user with a matching username 
(in other words, don't rename your users!).