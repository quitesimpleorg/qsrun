qsRunner
========
qsRunner is a launcher. It contains user defined entries for applications and also searches
system-applications. Using libcalculate, it can also be used as a calculator. It
can also search for files (and their contents) by querying a sqlite database, although it itself does
not index files. 

If you run a desktop environment like KDE it is questionable whether you will
find this useful, since they usually bring applications that are more or less
comparable to qsRunner, although much more
powerful  (like KRunner). It can be useful for users running a window manager like
fluxbox etc.

Dependencies
------------
It has been only tested for Qt 5.7.

For the calculation engine, libqalculate is needed.
For file search, easyindex is necessary. 

Currently no conditional compile flags are supported...

Getting started
----------------
Currently it may not be considered a classical GUI application because the  
configuration must be done outside of it.  

mkdir $HOME/.config/qsRunner
In this folder user-defined entries should be put (See "Entry format").

Config format
------------
Path: $HOME/.config/qsRunner/qsrunner.config

```
[Search]
dbpath="/path/to/database.sqlite"

[General]
systemApplicationsPath="/usr/share/applications/"
```

systemApplicationsPath will default to "/usr/share/applications/",
therefore specifying it explicitly is not necessary. 

Entry format
------------
It rudimentary supports .desktop files, but for user entries, the own format
should be preferred.

It's a simple format: [key] [value].

Example: quasselclient.qsrun:

```
command quasselclient
name Quassel
icon /usr/share/icons/hicolor/128x128/apps/quassel.png
row 1
col 0
key I
```

"key" means a shortcut key, you can launch those by pressing Ctrl + "key", so in
the example above: CTRL + I.

Simply pressing Ctrl will show you the associated shortcuts on each individual
button.


General usage
=============
Starting to type will search user defined entries first, followed by system
entries. Then the PATH variable will be searched, if there is a single match you can also
press TAB for auto completion. 

In general it will launch anything once you press enter, however it won't open a
terminal.

Calculator
=========
Start by typing "=", followed by your expression, e. g: "=(2+3)^2"

File searches
=============
It only queries a sqlite database. It does not index files, this is beyond the
scope a launcher. For the file searches functionality, easyindex is necessary.
See: git.quitesimple.org/easyindex.


