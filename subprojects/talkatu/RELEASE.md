# Releases

This document describes how we release Talkatu.

For each release, someone will be chosen to manage the release.  This person
will be referred to as the release manager going forward in this document.

# Prerequisites

The release manager will need access to a few things to perform their duties.

 * Push access to the official repository
 * Transifex
 * Package Cloud

The release manager will need to update translations as part of the release
process.  This is done via `tx pull --minimum-perc 1 --all` in the root of the
project.  This will update all `.po`'s in the `po/` directory to what's
currently on Transifex and have at least 1 percent translated.  These should
be committed at this point.

# Preparing the Release Commit

Make sure you are on the `default` branch!!!

Make sure that the date is correct in ChangeLog and that ChangeLog has been
updated to reflect what is new in this release.

Update the version number in `meson.build` being sure to remove any extra
version like `-dev` which is not part of the release.

# Making the Release Commit

Make sure you are on the `default` branch!!!

Verify that everything in `Preparing the Release Commit` is actually completed.

Verify that `hg status` looks sane.

Create a commit with a message of "Releasing version ${VERSION}"

# Releasing

At this point, you just need to `hg push ${UPSTREAM}` and CI/CD will take care of the rest.

# Post Release

Once you've pushed the release commit, you need to updated a few more things
for the next round of development.

First determine what the next version will be.  Most likely it'll be a micro
release.

Next add a new line to `ChangeLog` with the following

```
${NEW_VERSION}: (????-??-??)
  * Nothing yet, be the first!
```

Next, update `meson.build` for the new version and append a `-dev` on the end
of it.

Finally commit those changes on `default` with a commit message along the lines
of "Bumping version for the next round of development."

Finally, push to the main repository.
