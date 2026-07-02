Put your own copies of these two files here (not committed by me - see main README):

  EuroScopePlugIn.h
  EuroScope.lib

Source: whoever in VATCAN engineering circles already has a working
EuroScope plugin dev setup (see main README's "Known issue #1").

Once both files are in this folder, the GitHub Actions workflow at
.github/workflows/build-plugin.yml will pick them up automatically -
no other config needed.

This repo should stay PRIVATE. EuroScope.lib almost certainly can't
legally sit in a public repo (unclear redistribution terms), and even
private isn't zero-risk - anyone you add as a collaborator can see it.
